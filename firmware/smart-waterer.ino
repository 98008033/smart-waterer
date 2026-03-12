/**
 * @file smart-waterer.ino
 * @brief 智能浇花机固件
 * @version 0.1
 * @date 2026-03-12
 * 
 * 功能:
 * - WiFi 连接
 * - MQTT 通信 (腾讯云 IoT)
 * - 传感器读取 (土壤/水位/电池)
 * - 水泵控制
 * - OTA 固件更新
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Update.h>

// ==================== 配置参数 ====================

// WiFi 配置
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// 腾讯云 IoT 配置
#define ESP_PRODUCT_ID     "ABC123456"
#define ESP_DEVICE_NAME    "waterer-001"
#define ESP_DEVICE_SECRET  "xyz789abcdef"
#define ESP_MQTT_HOST      String(ESP_PRODUCT_ID) + ".iotcloud.tencentdevices.com"
#define ESP_MQTT_PORT      8883

// 引脚定义
#define SOIL_MOISTURE_PIN  0   // GPIO0 - 土壤湿度 ADC
#define PUMP_PIN           1   // GPIO1 - 水泵控制
#define WATER_LEVEL_PIN    2   // GPIO2 - 水位检测
#define BATTERY_VOLT_PIN   3   // GPIO3 - 电池电压 ADC
#define LED_PIN            4   // GPIO4 - LED 指示
#define BUZZER_PIN         5   // GPIO5 - 蜂鸣器

// 系统参数
#define REPORT_INTERVAL    30000    // 上报间隔 30 秒
#define SOIL_THRESHOLD     30       // 土壤湿度阈值 30%
#define WATER_DURATION     2000     // 浇水时长 2 秒
#define LOW_BATTERY        20       // 低电量阈值 20%

// ==================== 全局变量 ====================

WiFiClient espClient;
PubSubClient client(espClient);

// 系统状态
struct SystemStatus {
    int soilMoisture = 0;
    int batteryLevel = 0;
    bool waterLevel = false;
    bool isWatering = false;
    unsigned long lastReport = 0;
    bool wifiConnected = false;
    bool mqttConnected = false;
} status;

// ==================== 函数声明 ====================

// WiFi
void connectWiFi();
bool isWiFiConnected();

// MQTT
void mqttCallback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT();
void reportSensorData();
void handleDownlink(const String& payload);

// 传感器
void initSensors();
int readSoilMoisture();
bool readWaterLevel();
int readBatteryLevel();

// 执行器
void initActuators();
void setPumpState(bool state);
void waterPlant();
void beepAlert();
void setLED(bool state);

// OTA
void setupOTA();
void handleOTA();
bool updateFirmware(const String& url);

// 工具
String generatePassword(const String& secret, long timestamp);
void enterDeepSleep();
void blinkLED(int times);

// ==================== 设置程序 ====================

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n=== 智能浇花机启动 ===");
    
    // 初始化硬件
    initSensors();
    initActuators();
    
    // 启动提示
    blinkLED(3);
    beepAlert();
    
    // 连接 WiFi
    Serial.print("连接 WiFi...");
    connectWiFi();
    
    // 配置 MQTT
    client.setServer(ESP_MQTT_HOST.c_str(), ESP_MQTT_PORT);
    client.setCallback(mqttCallback);
    
    // 配置 OTA
    setupOTA();
    
    Serial.println("系统初始化完成！");
}

// ==================== 主循环 ====================

void loop() {
    // 保持 WiFi 连接
    if (!isWiFiConnected()) {
        Serial.println("WiFi 断开，重连中...");
        connectWiFi();
    }
    
    // 保持 MQTT 连接
    if (!client.connected()) {
        reconnectMQTT();
    }
    client.loop();
    
    // 处理 OTA
    handleOTA();
    
    // 定期上报数据
    if (millis() - status.lastReport > REPORT_INTERVAL) {
        reportSensorData();
        status.lastReport = millis();
    }
    
    // 自动浇水逻辑
    if (status.soilMoisture < SOIL_THRESHOLD && status.waterLevel && status.batteryLevel > LOW_BATTERY) {
        Serial.println("土壤干燥，开始浇水...");
        waterPlant();
    }
    
    // 低功耗模式
    delay(1000);
}

// ==================== WiFi 实现 ====================

void connectWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int timeout = 30;
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        delay(500);
        Serial.print(".");
        timeout--;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi 连接成功！");
        Serial.print("IP 地址：");
        Serial.println(WiFi.localIP());
        status.wifiConnected = true;
        blinkLED(1);
    } else {
        Serial.println("\nWiFi 连接失败！");
        status.wifiConnected = false;
        ESP.restart();
    }
}

bool isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

// ==================== MQTT 实现 ====================

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // 处理下行指令
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.println("收到下行指令：" + message);
    handleDownlink(message);
}

void reconnectMQTT() {
    while (!client.connected()) {
        Serial.print("连接 MQTT...");
        
        String clientId = String(ESP_PRODUCT_ID) + ESP_DEVICE_NAME;
        long timestamp = time(nullptr);
        String username = String(ESP_PRODUCT_ID) + ESP_DEVICE_NAME + ";" + 
                         String(ESP_PRODUCT_ID) + ";" + String(timestamp);
        String password = generatePassword(ESP_DEVICE_SECRET, timestamp);
        
        if (client.connect(clientId.c_str(), username.c_str(), password.c_str())) {
            Serial.println("成功！");
            status.mqttConnected = true;
            
            // 订阅下行主题
            String subTopic = String(ESP_PRODUCT_ID) + "/" + 
                             ESP_DEVICE_NAME + "/s+/+";
            client.subscribe(subTopic.c_str());
            Serial.println("订阅主题：" + subTopic);
            
            // 上报设备状态
            reportSensorData();
        } else {
            Serial.print("失败，rc=");
            Serial.println(client.state());
            status.mqttConnected = false;
            delay(5000);
        }
    }
}

void reportSensorData() {
    // 读取传感器数据
    status.soilMoisture = readSoilMoisture();
    status.waterLevel = readWaterLevel();
    status.batteryLevel = readBatteryLevel();
    
    // 构建 JSON
    StaticJsonDocument<256> doc;
    doc["soil_moisture"] = status.soilMoisture;
    doc["battery_level"] = status.batteryLevel;
    doc["water_level"] = status.waterLevel ? 1 : 0;
    
    String payload;
    serializeJson(doc, payload);
    
    // 发布到腾讯云
    String pubTopic = String(ESP_PRODUCT_ID) + "/" + 
                     ESP_DEVICE_NAME + "/data/update";
    
    if (client.publish(pubTopic.c_str(), payload.c_str())) {
        Serial.println("数据上报：" + payload);
    } else {
        Serial.println("数据上报失败");
    }
}

void handleDownlink(const String& payload) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
        Serial.println("JSON 解析失败");
        return;
    }
    
    // 处理浇水开关
    if (doc.containsKey("water_switch")) {
        bool switchState = doc["water_switch"];
        setPumpState(switchState);
        status.isWatering = switchState;
        Serial.println("浇水开关：" + String(switchState ? "开" : "关"));
    }
    
    // 处理浇水时长
    if (doc.containsKey("water_duration")) {
        int duration = doc["water_duration"];
        // 更新全局变量（需要修改定义）
        Serial.println("浇水时长：" + String(duration) + "ms");
    }
}

String generatePassword(const String& secret, long timestamp) {
    // HMAC-SHA1 实现（简化版，实际需要使用 mbedTLS）
    // 这里使用腾讯云提供的密码生成方式
    return "hmac_sha1_result";  // 需要实现 HMAC-SHA1
}

// ==================== 传感器实现 ====================

void initSensors() {
    pinMode(SOIL_MOISTURE_PIN, INPUT);
    pinMode(WATER_LEVEL_PIN, INPUT_PULLUP);
    pinMode(BATTERY_VOLT_PIN, INPUT);
    
    Serial.println("传感器初始化完成");
}

int readSoilMoisture() {
    int adcValue = analogRead(SOIL_MOISTURE_PIN);
    
    // ADC 值映射到湿度百分比
    // 干燥空气：3200 → 0%
    // 水中：1200 → 100%
    int moisture = map(adcValue, 3200, 1200, 0, 100);
    moisture = constrain(moisture, 0, 100);
    
    Serial.print("土壤湿度：");
    Serial.print(moisture);
    Serial.println("%");
    
    return moisture;
}

bool readWaterLevel() {
    bool hasWater = digitalRead(WATER_LEVEL_PIN) == HIGH;
    
    Serial.print("水箱水位：");
    Serial.println(hasWater ? "正常" : "缺水");
    
    return hasWater;
}

int readBatteryLevel() {
    int adcValue = analogRead(BATTERY_VOLT_PIN);
    
    // 计算电池电压
    // V_batt = V_adc / 0.667
    float voltage = (adcValue / 4095.0) * 3.3 * 1.5;
    
    // 映射到电量百分比
    // 6.0V → 0%, 8.4V → 100%
    int percent = map(voltage * 100, 600, 840, 0, 100);
    percent = constrain(percent, 0, 100);
    
    Serial.print("电池电量：");
    Serial.print(percent);
    Serial.println("%");
    
    return percent;
}

// ==================== 执行器实现 ====================

void initActuators() {
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    
    // 初始状态
    setPumpState(false);
    setLED(false);
    
    Serial.println("执行器初始化完成");
}

void setPumpState(bool state) {
    digitalWrite(PUMP_PIN, state ? HIGH : LOW);
    Serial.print("水泵状态：");
    Serial.println(state ? "开" : "关");
}

void waterPlant() {
    Serial.println("开始浇水...");
    setPumpState(true);
    delay(WATER_DURATION);
    setPumpState(false);
    Serial.println("浇水完成");
}

void beepAlert() {
    for (int i = 0; i < 3; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        delay(200);
    }
}

void setLED(bool state) {
    digitalWrite(LED_PIN, state ? HIGH : LOW);
}

void blinkLED(int times) {
    for (int i = 0; i < times; i++) {
        setLED(true);
        delay(100);
        setLED(false);
        delay(100);
    }
}

// ==================== OTA 实现 ====================

void setupOTA() {
    Serial.println("OTA 配置完成");
}

void handleOTA() {
    // 检查是否有新版本
    // 从腾讯云 IoT 获取固件版本信息
    // 如果有新版本，下载并更新
}

bool updateFirmware(const String& firmwareUrl) {
    HTTPClient http;
    http.begin(firmwareUrl);
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.println("下载失败");
        return false;
    }
    
    int contentLength = http.getSize();
    if (contentLength <= 0) {
        return false;
    }
    
    if (Update.begin(contentLength)) {
        WiFiClient* client = http.getStreamPtr();
        size_t written = Update.writeStream(*client);
        
        if (written == contentLength) {
            Serial.println("下载完成");
        }
        
        if (Update.end()) {
            if (Update.isFinished()) {
                Serial.println("更新成功，重启中...");
                ESP.restart();
                return true;
            }
        }
    }
    
    return false;
}

// ==================== 低功耗实现 ====================

void enterDeepSleep() {
    // 配置唤醒源（定时器）
    esp_sleep_enable_timer_wakeup(30 * 60 * 1000000ULL);  // 30 分钟
    
    // 进入深度睡眠
    Serial.println("进入深度睡眠...");
    esp_deep_sleep_start();
}
