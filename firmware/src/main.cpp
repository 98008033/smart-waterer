/**
 * @file main.cpp
 * @brief 智能浇花机 v0.4 - 主程序入口
 * @version 0.4.0
 * @date 2026-03-13
 * 
 * 功能模块：
 * - WiFi 连接管理
 * - ThingsCloud MQTT 通信
 * - 传感器数据采集
 * - 自动浇水控制
 * - 电池安全监控
 * - OTA 固件更新
 */

#include <Arduino.h>
#include <WiFi.h>

// 配置文件
#include "config/config.h"
#include "config/pins.h"

// 核心模块
#include "core/wifi_manager.h"
#include "core/mqtt_client.h"
#include "core/ota_update.h"

// 传感器模块
#include "sensors/soil_moisture.h"
#include "sensors/water_level.h"
#include "sensors/battery_monitor.h"
#include "sensors/ntc_temperature.h"

// 执行器模块
#include "actuators/pump_controller.h"

// 逻辑模块
#include "logic/watering_logic.h"
#include "logic/safety_manager.h"

// 工具模块
#include "utils/logger.h"

// ==================== 全局对象 ====================
WiFiManager wifiManager;
MQTTClient mqttClient;
OTAManager otaManager;

SoilMoistureSensor soilSensor;
WaterLevelSensor waterSensor;
BatteryMonitor batteryMonitor;
NTCTemperatureSensor tempSensor;

PumpController pumpController;
WateringLogic wateringLogic;
SafetyManager safetyManager;

// ==================== 系统状态 ====================
struct SystemState {
    bool wifiConnected = false;
    bool mqttConnected = false;
    bool safetyLock = false;
    unsigned long lastWateringTime = 0;
    unsigned long lastReportTime = 0;
    uint16_t wateringCount = 0;
} systemState;

// ==================== 函数声明 ====================
void setupHardware();
void setupModules();
void checkSafety();
void reportData();
void processCommands();
void enterDeepSleep();

// ==================== 初始化 ====================
void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(1000);
    
    LOG_I(LOG_TAG, "================================");
    LOG_I(LOG_TAG, "智能浇花机 v%s", FIRMWARE_VERSION);
    LOG_I(LOG_TAG, "启动时间：%s", __DATE__);
    LOG_I(LOG_TAG, "================================");
    
    // 初始化硬件
    setupHardware();
    
    // 初始化模块
    setupModules();
    
    // 检查安全状态
    checkSafety();
    
    // 连接 WiFi
    #if ENABLE_WIFI
    LOG_I(LOG_TAG, "正在连接 WiFi...");
    wifiManager.begin(WIFI_SSID, WIFI_PASSWORD);
    
    if (wifiManager.isConnected()) {
        systemState.wifiConnected = true;
        LOG_I(LOG_TAG, "WiFi 连接成功");
        
        // 连接 MQTT
        #if ENABLE_MQTT
        LOG_I(LOG_TAG, "正在连接 MQTT...");
        mqttClient.begin(MQTT_SERVER, MQTT_PORT);
        
        if (mqttClient.isConnected()) {
            systemState.mqttConnected = true;
            LOG_I(LOG_TAG, "MQTT 连接成功");
        } else {
            LOG_E(LOG_TAG, "MQTT 连接失败");
        }
        #endif
    } else {
        LOG_E(LOG_TAG, "WiFi 连接失败");
    }
    
    // 检查 OTA 更新
    #if ENABLE_OTA
    otaManager.checkUpdate();
    #endif
    
    LOG_I(LOG_TAG, "系统初始化完成");
    LOG_I(LOG_TAG, "================================");
}

// ==================== 主循环 ====================
void loop() {
    // 维护 WiFi 连接
    #if ENABLE_WIFI
    if (!wifiManager.isConnected()) {
        wifiManager.reconnect();
        systemState.wifiConnected = wifiManager.isConnected();
    }
    #endif
    
    // 维护 MQTT 连接
    #if ENABLE_MQTT
    if (systemState.wifiConnected && systemState.mqttConnected) {
        mqttClient.loop();
    }
    #endif
    
    // 检查安全状态
    checkSafety();
    
    // 如果安全锁定，跳过所有操作
    if (systemState.safetyLock) {
        LOG_W(LOG_TAG, "系统安全锁定，暂停所有操作");
        delay(1000);
        return;
    }
    
    // 处理 MQTT 命令
    #if ENABLE_MQTT
    if (systemState.mqttConnected) {
        processCommands();
    }
    #endif
    
    // 自动浇水逻辑
    #if ENABLE_AUTO_WATERING
    wateringLogic.checkAndWater();
    #endif
    
    // 定期上报数据
    unsigned long currentTime = millis();
    if (currentTime - systemState.lastReportTime >= 60000) {  // 60 秒上报一次
        reportData();
        systemState.lastReportTime = currentTime;
    }
    
    // 进入低功耗模式
    #if ENABLE_DEEP_SLEEP
    if (!systemState.wifiConnected && !systemState.mqttConnected) {
        LOG_I(LOG_TAG, "进入 Deep Sleep 模式...");
        enterDeepSleep();
    }
    #endif
    
    delay(100);
}

// ==================== 硬件初始化 ====================
void setupHardware() {
    LOG_I(LOG_TAG, "初始化硬件...");
    
    // 初始化引脚
    pins_init();
    
    // 初始化传感器
    soilSensor.begin();
    waterSensor.begin();
    batteryMonitor.begin();
    tempSensor.begin();
    
    // 初始化执行器
    pumpController.begin();
    
    // 初始化安全 manager
    safetyManager.begin();
    
    LOG_I(LOG_TAG, "硬件初始化完成");
}

// ==================== 模块初始化 ====================
void setupModules() {
    LOG_I(LOG_TAG, "初始化模块...");
    
    // 初始化 WiFi
    #if ENABLE_WIFI
    wifiManager.onConnected([]() {
        LOG_I(LOG_TAG, "WiFi 已连接");
        systemState.wifiConnected = true;
    });
    
    wifiManager.onDisconnected([]() {
        LOG_W(LOG_TAG, "WiFi 已断开");
        systemState.wifiConnected = false;
    });
    #endif
    
    // 初始化 MQTT
    #if ENABLE_MQTT
    mqttClient.onConnected([]() {
        LOG_I(LOG_TAG, "MQTT 已连接");
        systemState.mqttConnected = true;
    });
    
    mqttClient.onDisconnected([]() {
        LOG_W(LOG_TAG, "MQTT 已断开");
        systemState.mqttConnected = false;
    });
    #endif
    
    // 初始化浇水逻辑
    wateringLogic.begin();
    
    LOG_I(LOG_TAG, "模块初始化完成");
}

// ==================== 安全检查 ====================
void checkSafety() {
    // 读取温度
    float temperature = tempSensor.read();
    
    // 读取电池电压
    float voltage = batteryMonitor.readVoltage();
    int batteryPercent = batteryMonitor.readPercent();
    
    // 检查温度保护
    if (temperature >= TEMP_CUTOFF_THRESHOLD) {
        LOG_E(LOG_TAG, "温度过高！%.1f°C >= %.1f°C", temperature, TEMP_CUTOFF_THRESHOLD);
        safetyManager.triggerSafetyLock(SafetyLockReason::HIGH_TEMP);
        systemState.safetyLock = true;
    } else if (temperature >= TEMP_WARNING_THRESHOLD) {
        LOG_W(LOG_TAG, "温度偏高！%.1f°C >= %.1f°C", temperature, TEMP_WARNING_THRESHOLD);
        // 限制充电，但不锁定系统
    }
    
    // 检查电池保护
    if (voltage <= BATTERY_CUTOFF_VOLTAGE) {
        LOG_E(LOG_TAG, "电池电压过低！%.2fV <= %.2fV", voltage, BATTERY_CUTOFF_VOLTAGE);
        safetyManager.triggerSafetyLock(SafetyLockReason::LOW_BATTERY);
        systemState.safetyLock = true;
    } else if (voltage <= BATTERY_LOW_VOLTAGE) {
        LOG_W(LOG_TAG, "电池电量低！%.2fV <= %.2fV", voltage, BATTERY_LOW_VOLTAGE);
        // 发送低电警告
    }
}

// ==================== 数据上报 ====================
void reportData() {
    #if ENABLE_MQTT
    if (!systemState.mqttConnected) {
        return;
    }
    
    // 读取传感器数据
    int soilMoisture = soilSensor.readPercent();
    bool waterLevel = waterSensor.read();
    float temperature = tempSensor.read();
    float voltage = batteryMonitor.readVoltage();
    int batteryPercent = batteryMonitor.readPercent();
    
    // 构建 JSON 数据
    String json = "{";
    json += "\"soil_moisture\":" + String(soilMoisture) + ",";
    json += "\"water_level\":" + String(waterLevel ? 1 : 0) + ",";
    json += "\"temperature\":" + String(temperature, 1) + ",";
    json += "\"battery_voltage\":" + String(voltage, 2) + ",";
    json += "\"battery_percent\":" + String(batteryPercent);
    json += "}";
    
    // 上报到 ThingsCloud
    mqttClient.publish(MQTT_DEVICE_TOPIC, json.c_str());
    
    LOG_I(LOG_TAG, "数据上报：%s", json.c_str());
    #endif
}

// ==================== 处理命令 ====================
void processCommands() {
    // MQTT 命令处理在 mqtt_client 中实现回调
    // 这里可以添加其他命令处理逻辑
}

// ==================== 进入低功耗模式 ====================
void enterDeepSleep() {
    #if ENABLE_DEEP_SLEEP
    LOG_I(LOG_TAG, "准备进入 Deep Sleep...");
    
    // 关闭外设
    pumpController.stop();
    
    // 配置唤醒源
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DURATION_SECONDS * 1000000ULL);
    
    // 进入 Deep Sleep
    LOG_I(LOG_TAG, "进入 Deep Sleep，%d 秒后唤醒", DEEP_SLEEP_DURATION_SECONDS);
    esp_deep_sleep_start();
    
    // 不会执行到这里
    #endif
}
