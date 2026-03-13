/**
 * @file config.h
 * @brief 编译配置
 * @version 0.4
 * @date 2026-03-13
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

// ==================== 版本信息 ====================
#define FIRMWARE_VERSION_MAJOR 0
#define FIRMWARE_VERSION_MINOR 4
#define FIRMWARE_VERSION_PATCH 0
#define FIRMWARE_VERSION "0.4.0"

// ==================== 功能开关 ====================
#define ENABLE_WIFI true           // WiFi 功能
#define ENABLE_MQTT true           // MQTT 功能
#define ENABLE_OTA true            // OTA 更新
#define ENABLE_DEEP_SLEEP true     // 低功耗模式
#define ENABLE_AUTO_WATERING true  // 自动浇水

// ==================== WiFi 配置 ====================
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define WIFI_MAX_RETRIES 10        // WiFi 连接最大重试次数
#define WIFI_RETRY_DELAY 500       // WiFi 重试延迟 (ms)
#define WIFI_TIMEOUT 10000         // WiFi 连接超时 (ms)

// ==================== ThingsCloud 配置 ====================
#define MQTT_SERVER "bemfa.com"
#define MQTT_PORT 1883
#define MQTT_USER "YOUR_PRIVATE_KEY"      // 从 ThingsCloud 控制台获取
#define MQTT_DEVICE_TOPIC "waterer_001"   // 设备主题
#define MQTT_KEEPALIVE 60                 // MQTT 心跳间隔 (秒)

// ==================== 传感器配置 ====================
// 土壤湿度
#define SOIL_MOISTURE_PIN GPIO_NUM_0
#define SOIL_MOISTURE_ADC_MIN 1200   // 水中 ADC 值
#define SOIL_MOISTURE_ADC_MAX 3200   // 干燥空气 ADC 值
#define SOIL_MOISTURE_FILTER_SAMPLES 10  // 滤波采样数

// 水位检测
#define WATER_LEVEL_PIN GPIO_NUM_2
#define WATER_LEVEL_DEBOUNCE_MS 50   // 防抖动时间

// NTC 温度
#define NTC_PIN GPIO_NUM_3
#define NTC_NOMINAL_RES 10000.0f     // 10kΩ @ 25°C
#define NTC_B_VALUE 3950.0f          // B 值
#define NTC_PULLUP_RES 10000.0f      // 上拉电阻
#define NTC_FILTER_SAMPLES 16        // 滤波采样数

// 电池电压
#define BATTERY_ADC_PIN GPIO_NUM_3   // 复用 NTC 引脚
#define BATTERY_DIVIDER_RATIO 1.5f   // 分压比 (R1+R2)/R2
#define BATTERY_ADC_REF 3.3f         // ADC 参考电压
#define BATTERY_ADC_MAX 4095         // 12-bit ADC

// ==================== 执行器配置 ====================
// 水泵控制
#define PUMP_PIN GPIO_NUM_1
#define PUMP_MAX_DURATION_MS 300000  // 最大浇水时长 5 分钟
#define PUMP_DEFAULT_DURATION_MS 30000  // 默认浇水时长 30 秒
#define PUMP_COOLDOWN_MS 600000      // 浇水间隔 10 分钟

// ==================== 保护阈值 ====================
// 温度保护
#define TEMP_WARNING_THRESHOLD 50.0f     // 50°C 警告
#define TEMP_CUTOFF_THRESHOLD 60.0f      // 60°C 切断
#define TEMP_RECOVER_THRESHOLD 45.0f     // 45°C 恢复

// 电池保护
#define BATTERY_FULL_VOLTAGE 4.2f        // 满电电压 (单节)
#define BATTERY_LOW_VOLTAGE 3.5f         // 低电警告
#define BATTERY_CUTOFF_VOLTAGE 3.0f      // 切断电压
#define BATTERY_PERCENT_MIN 0
#define BATTERY_PERCENT_MAX 100

// ==================== 自动浇水配置 ====================
#define AUTO_WATERING_INTERVAL_MS 43200000  // 12 小时
#define SOIL_MOISTURE_TARGET 70             // 目标湿度 70%
#define SOIL_MOISTURE_HYSTERESIS 10         // 迟滞 10%

// ==================== 低功耗配置 ====================
#define DEEP_SLEEP_DURATION_SECONDS 3600    // 休眠 1 小时
#define WAKEUP_REASON_GPIO RTC_GPIO_WAKEUP
#define WAKEUP_REASON_TIMER RTC_ULP_WAKEUP

// ==================== 存储配置 ====================
#define EEPROM_SIZE 512
#define EEPROM_WIFI_ADDR 0
#define EEPROM_SETTINGS_ADDR 256
#define EEPROM_CALIBRATION_ADDR 384

// ==================== 日志配置 ====================
#define LOG_TAG "SmartWaterer"
#define LOG_LEVEL_DEBUG 3
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARN 1
#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL LOG_LEVEL_INFO

// ==================== 调试开关 ====================
#define DEBUG_ENABLE true
#define DEBUG_PRINT_ADC false      // 打印 ADC 原始值
#define DEBUG_PRINT_MEMORY false   // 打印内存使用情况

#endif // __CONFIG_H__
