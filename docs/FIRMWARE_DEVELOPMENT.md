# 固件开发指南 💻

**ESP32-C3 固件开发完整教程**

---

## 1. 开发环境搭建

### 1.1 安装 Arduino IDE

**Windows:**
```
1. 访问：https://www.arduino.cc/en/software
2. 下载 Windows Installer
3. 运行安装程序
4. 安装完成
```

**Mac:**
```
1. 访问：https://www.arduino.cc/en/software
2. 下载 macOS App
3. 拖拽到应用程序文件夹
4. 打开 Arduino
```

**Linux:**
```bash
# Ubuntu/Debian
sudo apt-get install arduino

# 或下载 AppImage
wget https://downloads.arduino.cc/arduino-ide/arduino-ide_latest_Linux_64bit.AppImage
chmod +x arduino-ide_latest_Linux_64bit.AppImage
./arduino-ide_latest_Linux_64bit.AppImage
```

**⏰ 耗时：** 10 分钟

### 1.2 安装 ESP32 板支持

```
1. 打开 Arduino IDE
2. 点击"文件" → "首选项"
3. 在"附加开发板管理器网址"输入：
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
4. 点击"工具" → "开发板" → "开发板管理器"
5. 搜索"ESP32"
6. 安装"ESP32 by Espressif Systems"
7. 安装完成
```

**⏰ 耗时：** 5 分钟

### 1.3 安装必需库

**通过库管理器安装：**
```
1. 点击"工具" → "管理库"
2. 搜索并安装以下库：

- PubSubClient (MQTT 客户端)
- ArduinoJson (JSON 解析)
- WiFi (ESP32 自带)
- HTTPClient (HTTP 请求)
- Update (OTA 更新)
```

**⏰ 耗时：** 5 分钟

### 1.4 验证安装

```
1. 点击"工具" → "开发板" → "ESP32 Arduino"
2. 选择"XiaoS3"或"ESP32C3 Dev Module"
3. 点击"文件" → "示例" → "01.Basics" → "Blink"
4. 点击"上传"按钮
5. 编译成功说明安装正确
```

**⏰ 耗时：** 5 分钟

---

## 2. 项目结构

### 2.1 目录结构

```
smart-waterer-firmware/
├── smart-waterer-firmware.ino  # 主程序
├── config.h                     # 配置参数
├── wifi.cpp                     # WiFi 连接
├── wifi.h
├── mqtt.cpp                     # MQTT 客户端
├── mqtt.h
├── sensors.cpp                  # 传感器读取
├── sensors.h
├── pump.cpp                     # 水泵控制
├── pump.h
├── ota.cpp                      # OTA 更新
├── ota.h
└── utils.cpp                    # 工具函数
└── utils.h
```

### 2.2 主程序框架

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "wifi.h"
#include "mqtt.h"
#include "sensors.h"
#include "pump.h"
#include "ota.h"

// 全局变量
WiFiClient espClient;
PubSubClient client(ESP_MQTT_HOST, ESP_MQTT_PORT, espClient);

void setup() {
    Serial.begin(115200);
    
    // 初始化硬件
    initSensors();
    initPump();
    
    // 连接 WiFi
    connectWiFi(WIFI_SSID, WIFI_PASSWORD);
    
    // 配置 MQTT
    client.setServer(ESP_MQTT_HOST, ESP_MQTT_PORT);
    client.setCallback(mqttCallback);
    
    // 配置 OTA
    setupOTA();
}

void loop() {
    // 处理 MQTT 连接
    if (!client.connected()) {
        reconnectMQTT();
    }
    client.loop();
    
    // 处理 OTA
    handleOTA();
    
    // 定期上报数据
    static unsigned long lastReport = 0;
    if (millis() - lastReport > REPORT_INTERVAL) {
        reportSensorData();
        lastReport = millis();
    }
    
    // 进入深度睡眠（省电）
    enterDeepSleep();
}
```

---

## 3. 核心功能实现

### 3.1 WiFi 连接

**文件：** `wifi.cpp`

```cpp
#include "wifi.h"
#include <WiFi.h>

void connectWiFi(const char* ssid, const char* password) {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    
    int timeout = 30;
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        delay(500);
        Serial.print(".");
        timeout--;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection failed!");
        ESP.restart();
    }
}

bool isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}
```

### 3.2 MQTT 接入

**文件：** `mqtt.cpp`

```cpp
#include "mqtt.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

extern WiFiClient espClient;
extern PubSubClient client;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // 处理下行指令
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload, length);
    
    if (doc.containsKey("water_switch")) {
        bool switchState = doc["water_switch"];
        setPumpState(switchState);
    }
    
    if (doc.containsKey("water_duration")) {
        int duration = doc["water_duration"];
        setWaterDuration(duration);
    }
}

void reconnectMQTT() {
    while (!client.connected()) {
        Serial.print("Connecting to MQTT...");
        
        String clientId = String(ESP_PRODUCT_ID) + ESP_DEVICE_NAME;
        String username = String(ESP_PRODUCT_ID) + ESP_DEVICE_NAME + ";" + 
                         String(ESP_PRODUCT_ID) + ";" + String(time(nullptr));
        String password = generatePassword(ESP_DEVICE_SECRET, time(nullptr));
        
        if (client.connect(clientId.c_str(), username.c_str(), password.c_str())) {
            Serial.println("connected");
            
            // 订阅下行主题
            String subTopic = String(ESP_PRODUCT_ID) + "/" + 
                             ESP_DEVICE_NAME + "/s+/+";
            client.subscribe(subTopic.c_str());
        } else {
            Serial.print("failed, rc=");
            Serial.println(client.state());
            delay(5000);
        }
    }
}

void reportSensorData() {
    StaticJsonDocument<256> doc;
    
    doc["soil_moisture"] = readSoilMoisture();
    doc["battery_level"] = readBatteryLevel();
    doc["water_level"] = readWaterLevel();
    
    String payload;
    serializeJson(doc, payload);
    
    String pubTopic = String(ESP_PRODUCT_ID) + "/" + 
                     ESP_DEVICE_NAME + "/data/update";
    
    if (client.publish(pubTopic.c_str(), payload.c_str())) {
        Serial.println("Data reported: " + payload);
    }
}
```

### 3.3 传感器读取

**文件：** `sensors.cpp`

```cpp
#include "sensors.h"

// 引脚定义
#define SOIL_MOISTURE_PIN  0  // GPIO0
#define WATER_LEVEL_PIN    2  // GPIO2
#define BATTERY_VOLT_PIN   3  // GPIO3

void initSensors() {
    pinMode(SOIL_MOISTURE_PIN, INPUT);
    pinMode(WATER_LEVEL_PIN, INPUT_PULLUP);
    pinMode(BATTERY_VOLT_PIN, INPUT);
}

int readSoilMoisture() {
    int adcValue = analogRead(SOIL_MOISTURE_PIN);
    // ADC 值映射到湿度百分比
    // 干燥空气：3200 → 0%
    // 水中：1200 → 100%
    int moisture = map(adcValue, 3200, 1200, 0, 100);
    return constrain(moisture, 0, 100);
}

bool readWaterLevel() {
    return digitalRead(WATER_LEVEL_PIN) == HIGH;
}

int readBatteryLevel() {
    int adcValue = analogRead(BATTERY_VOLT_PIN);
    // 计算电池电压
    // V_batt = V_adc / 0.667
    float voltage = (adcValue / 4095.0) * 3.3 * 1.5;
    // 映射到电量百分比
    // 6.0V → 0%, 8.4V → 100%
    int percent = map(voltage * 100, 600, 840, 0, 100);
    return constrain(percent, 0, 100);
}
```

### 3.4 水泵控制

**文件：** `pump.cpp`

```cpp
#include "pump.h"

#define PUMP_PIN  1  // GPIO1

int waterDuration = 2000;  // 默认浇水 2 秒

void initPump() {
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, LOW);
}

void setPumpState(bool state) {
    digitalWrite(PUMP_PIN, state ? HIGH : LOW);
}

void setWaterDuration(int duration) {
    waterDuration = constrain(duration, 1000, 300000);  // 1-300 秒
}

void waterPlant() {
    Serial.println("Start watering...");
    setPumpState(true);
    delay(waterDuration);
    setPumpState(false);
    Serial.println("Watering finished.");
}

void autoWatering(int soilMoisture) {
    if (soilMoisture < SOIL_THRESHOLD) {
        waterPlant();
    }
}
```

### 3.5 OTA 更新

**文件：** `ota.cpp`

```cpp
#include "ota.h"
#include <HTTPClient.h>
#include <Update.h>

void setupOTA() {
    // OTA 配置
    Update.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("OTA Progress: %u%%\r", (progress / total) * 100);
    });
}

void handleOTA() {
    // 检查是否有新版本
    // 从腾讯云 IoT 获取固件版本信息
    // 如果有新版本，下载并更新
}

bool updateFirmware(const char* firmwareUrl) {
    HTTPClient http;
    http.begin(firmwareUrl);
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.println("Download failed");
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
            Serial.println("Download complete");
        }
        
        if (Update.end()) {
            if (Update.isFinished()) {
                Serial.println("Update successful. Restarting...");
                ESP.restart();
                return true;
            }
        }
    }
    
    return false;
}
```

---

## 4. 完整示例代码

### 4.1 主程序

**文件：** `smart-waterer-firmware.ino`

```cpp
/**
 * 智能浇花机固件
 * 
 * 功能:
 * - WiFi 连接
 * - MQTT 通信
 * - 传感器读取
 * - 水泵控制
 * - OTA 更新
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"

// 全局对象
WiFiClient espClient;
PubSubClient client(ESP_MQTT_HOST, ESP_MQTT_PORT, espClient);

void setup() {
    Serial.begin(115200);
    
    // 初始化硬件
    initSensors();
    initPump();
    
    // 连接 WiFi
    connectWiFi(WIFI_SSID, WIFI_PASSWORD);
    
    // 配置 MQTT
    client.setServer(ESP_MQTT_HOST, ESP_MQTT_PORT);
    client.setCallback(mqttCallback);
    
    Serial.println("System initialized!");
}

void loop() {
    // 保持 MQTT 连接
    if (!client.connected()) {
        reconnectMQTT();
    }
    client.loop();
    
    // 定期上报数据
    static unsigned long lastReport = 0;
    if (millis() - lastReport > REPORT_INTERVAL) {
        reportSensorData();
        lastReport = millis();
    }
    
    // 低功耗模式
    delay(1000);
}
```

### 4.2 配置文件

**文件：** `config.h`

```cpp
#ifndef CONFIG_H
#define CONFIG_H

// WiFi 配置
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// 腾讯云 IoT 配置
#define ESP_PRODUCT_ID     "ABC123456"
#define ESP_DEVICE_NAME    "waterer-001"
#define ESP_DEVICE_SECRET  "xyz789abcdef"
#define ESP_MQTT_HOST      "ABC123456.iotcloud.tencentdevices.com"
#define ESP_MQTT_PORT      8883

// 上报间隔（毫秒）
#define REPORT_INTERVAL  30000  // 30 秒

// 土壤湿度阈值
#define SOIL_THRESHOLD   30  // 低于 30% 自动浇水

#endif
```

---

## 5. 烧录指南

### 5.1 驱动安装

**Windows:**
```
1. 下载 CH340 驱动
   https://www.wch.cn/downloads/CH341SER_ZIP.html
2. 运行安装程序
3. 重启电脑
```

**Mac:**
```bash
# 使用 Homebrew 安装
brew install ch340-serial-driver
```

### 5.2 烧录步骤

```
1. 用 USB 线连接 ESP32-C3 和电脑
2. Arduino IDE 选择开发板：
   "工具" → "开发板" → "ESP32 Arduino" → "XiaoS3"
3. 选择端口：
   "工具" → "端口" → "/dev/ttyUSB0" 或 "COM3"
4. 点击"上传"按钮
5. 等待编译和上传
6. 看到"Done uploading."说明成功
```

### 5.3 串口调试

```
1. 点击"工具" → "串口监视器"
2. 波特率选择：115200
3. 查看输出信息
4. 发送指令测试
```

---

## 6. 故障排查

### 6.1 常见问题

| 问题 | 原因 | 解决方案 |
|------|------|---------|
| 编译失败 | 库未安装 | 安装缺少的库 |
| 上传失败 | 驱动问题 | 重新安装 CH340 驱动 |
| 无法连接 WiFi | 密码错误 | 检查 WiFi 配置 |
| MQTT 连接失败 | 三元组错误 | 检查腾讯云配置 |
| 传感器读数异常 | 接线错误 | 检查电路连接 |

### 6.2 调试技巧

**串口输出：**
```cpp
Serial.println("Debug message");
Serial.printf("Value: %d\n", value);
```

**LED 指示：**
```cpp
// 闪烁表示状态
digitalWrite(LED_PIN, HIGH);
delay(100);
digitalWrite(LED_PIN, LOW);
```

---

## 7. 下一步

完成固件开发后：

1. **测试基本功能**
   - WiFi 连接
   - MQTT 通信
   - 传感器读取
   - 水泵控制

2. **优化代码**
   - 低功耗优化
   - 错误处理
   - 代码重构

3. **准备发布**
   - 版本管理
   - 发布说明
   - OTA 包制作

---

**祝你开发顺利！** 🚀

*最后更新：2026-03-12*
