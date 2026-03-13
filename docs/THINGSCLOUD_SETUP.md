# ☁️ ThingsCloud 接入指南

**项目：** 智能浇花机  
**日期：** 2026-03-13  
**平台：** ThingsCloud  
**官网：** https://thingscloud.xyz

---

## 1. 平台信息

| 项目 | 说明 |
|------|------|
| **名称** | ThingsCloud |
| **官网** | https://thingscloud.xyz |
| **类型** | 开源 IoT 平台 |
| **协议** | MQTT 3.1/3.1.1 |
| **部署** | 云服务 / 自托管 |
| **GitHub** | https://github.com/thingscloud |

**ThingsCloud 是什么：**
- 开源物联网平台
- 支持 MQTT 协议
- 可云端托管或自建
- 提供设备管理、数据可视化、规则引擎

---

## 2. 注册与准备

### 2.1 注册账号

**步骤：**
1. 访问：https://thingscloud.xyz
2. 点击"注册"或"Sign Up"
3. 使用邮箱注册
4. 验证邮箱

### 2.2 创建应用

**步骤：**
1. 登录控制台
2. 进入"应用管理"
3. 点击"创建应用"
4. 填写应用名称：`Smart Waterer`
5. 记录应用 ID 和密钥

### 2.3 获取连接信息

**创建应用后获取：**
- **Application ID** - 应用 ID
- **Access Key** - 访问密钥
- **Secret Key** - 密钥
- **MQTT Broker** - MQTT 服务器地址

**⚠️ 重要：** 保存以上信息，ESP32 连接需要！

---

## 3. 设备接入

### 3.1 创建产品

**步骤：**
1. 进入"产品管理"
2. 点击"创建产品"
3. 填写：
   - 产品名称：`Smart Waterer`
   - 通信协议：`MQTT`
   - 数据格式：`JSON`
   - 联网方式：`WiFi`

### 3.2 定义物模型

**属性定义：**

| 属性 | 标识符 | 类型 | 读写 | 说明 |
|------|--------|------|------|------|
| 土壤湿度 | soilMoisture | int | 只读 | 0-100% |
| 电池电量 | batteryLevel | int | 只读 | 0-100% |
| 水位状态 | waterLevel | bool | 只读 | true/false |
| 浇水开关 | waterSwitch | bool | 读写 | true/false |
| 浇水时长 | waterDuration | int | 读写 | 1-300 秒 |

**物模型 JSON：**
```json
{
  "properties": [
    {
      "identifier": "soilMoisture",
      "name": "土壤湿度",
      "dataType": {
        "type": "int",
        "min": 0,
        "max": 100,
        "unit": "%"
      },
      "accessMode": "r"
    },
    {
      "identifier": "batteryLevel",
      "name": "电池电量",
      "dataType": {
        "type": "int",
        "min": 0,
        "max": 100,
        "unit": "%"
      },
      "accessMode": "r"
    },
    {
      "identifier": "waterLevel",
      "name": "水位状态",
      "dataType": {
        "type": "bool"
      },
      "accessMode": "r"
    },
    {
      "identifier": "waterSwitch",
      "name": "浇水开关",
      "dataType": {
        "type": "bool"
      },
      "accessMode": "rw"
    },
    {
      "identifier": "waterDuration",
      "name": "浇水时长",
      "dataType": {
        "type": "int",
        "min": 1,
        "max": 300,
        "unit": "seconds"
      },
      "accessMode": "rw"
    }
  ]
}
```

### 3.3 创建设备

**步骤：**
1. 进入产品 → "设备管理"
2. 点击"创建设备"
3. 设备名称：`waterer-001`
4. 获取设备凭证：
   - **DeviceName** - 设备名称
   - **DeviceToken** - 设备令牌

---

## 4. ESP32 接入

### 4.1 连接信息

**MQTT 配置：**
```
Broker: mqtt.thingscloud.xyz  (或你的自建地址)
Port: 1883 (非加密) / 8883 (TLS)
ClientID: waterer-001
Username: 你的 Access Key
Password: 你的 Secret Key 或 Device Token
```

**主题格式：**
```
上报主题：/app/{appId}/device/{deviceId}/post
订阅主题：/app/{appId}/device/{deviceId}/set
```

### 4.2 Arduino 代码示例

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ==================== 配置区 ====================

// WiFi 配置
const char* WIFI_SSID = "你的 WiFi 名称";
const char* WIFI_PASS = "你的 WiFi 密码";

// ThingsCloud 配置
const char* MQTT_SERVER = "mqtt.thingscloud.xyz";
const int MQTT_PORT = 1883;
const char* MQTT_USER = "你的 Access Key";
const char* MQTT_PASS = "你的 Secret Key";
const char* APP_ID = "你的应用 ID";
const char* DEVICE_ID = "waterer-001";

// 主题
String publishTopic;
String subscribeTopic;

// 引脚定义
#define PIN_SOIL_ADC    GPIO3
#define PIN_WATER_LEVEL GPIO2
#define PIN_PUMP        GPIO1
#define PIN_LED         GPIO4

// ==================== 全局变量 ====================

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastReportTime = 0;
const unsigned long REPORT_INTERVAL = 60000;  // 60 秒

bool pumpState = false;
int waterDuration = 30;

// ==================== 回调函数 ====================

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.print("收到消息：");
    Serial.println(message);
    
    // 解析 JSON
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (!error) {
        if (doc.containsKey("waterSwitch")) {
            pumpState = doc["waterSwitch"];
            digitalWrite(PIN_PUMP, pumpState ? HIGH : LOW);
        }
        if (doc.containsKey("waterDuration")) {
            waterDuration = doc["waterDuration"];
        }
    }
}

// ==================== 读取传感器 ====================

int readSoilMoisture() {
    int adc = analogRead(PIN_SOIL_ADC);
    int moisture = map(adc, 3200, 1200, 0, 100);
    return constrain(moisture, 0, 100);
}

int readBatteryLevel() {
    int adc = analogRead(PIN_BATTERY_ADC);
    float voltage = (adc / 4095.0) * 3.3 * 1.5;
    int percent = map(voltage * 100, 600, 840, 0, 100);
    return constrain(percent, 0, 100);
}

bool readWaterLevel() {
    return digitalRead(PIN_WATER_LEVEL) == HIGH;
}

// ==================== 上报数据 ====================

void reportData() {
    StaticJsonDocument<512> doc;
    
    doc["soilMoisture"] = readSoilMoisture();
    doc["batteryLevel"] = readBatteryLevel();
    doc["waterLevel"] = readWaterLevel();
    doc["pumpState"] = pumpState;
    
    String payload;
    serializeJson(doc, payload);
    
    Serial.print("上报数据：");
    Serial.println(payload);
    
    client.publish(publishTopic.c_str(), payload.c_str());
}

// ==================== 连接 WiFi ====================

void connectWiFi() {
    Serial.print("连接 WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi 已连接");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi 连接失败");
    }
}

// ==================== 连接 MQTT ====================

void connectMQTT() {
    String clientId = String("ESP32_") + DEVICE_ID;
    
    // 配置主题
    publishTopic = String("/app/") + APP_ID + "/device/" + DEVICE_ID + "/post";
    subscribeTopic = String("/app/") + APP_ID + "/device/" + DEVICE_ID + "/set";
    
    while (!client.connected()) {
        Serial.print("连接 MQTT...");
        
        if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
            Serial.println("已连接");
            client.subscribe(subscribeTopic.c_str());
            Serial.println("订阅：" + subscribeTopic);
        } else {
            Serial.print("失败，rc=");
            Serial.println(client.state());
            delay(5000);
        }
    }
}

// ==================== 设置函数 ====================

void setup() {
    Serial.begin(115200);
    
    // 初始化引脚
    pinMode(PIN_SOIL_ADC, ANALOG);
    pinMode(PIN_WATER_LEVEL, INPUT);
    pinMode(PIN_PUMP, OUTPUT);
    pinMode(PIN_LED, OUTPUT);
    
    digitalWrite(PIN_PUMP, LOW);
    digitalWrite(PIN_LED, LOW);
    
    // 连接 WiFi
    connectWiFi();
    
    // 配置 MQTT
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback);
    
    // 连接 MQTT
    connectMQTT();
    
    Serial.println("\n===== 系统启动完成 =====");
}

// ==================== 主循环 ====================

void loop() {
    if (!client.connected()) {
        connectMQTT();
    }
    client.loop();
    
    if (millis() - lastReportTime > REPORT_INTERVAL) {
        reportData();
        lastReportTime = millis();
    }
    
    delay(100);
}
```

---

## 5. 测试验证

### 5.1 串口监视器

**正常输出：**
```
连接 WiFi: MyWiFi
.....
WiFi 已连接
IP: 192.168.1.100
连接 MQTT...已连接
订阅：/app/xxx/device/waterer-001/set
===== 系统启动完成 =====
上报数据：{"soilMoisture":65,"batteryLevel":85,"waterLevel":true}
```

### 5.2 控制台验证

**步骤：**
1. 登录 ThingsCloud 控制台
2. 进入"设备管理"
3. 选择 `waterer-001`
4. 查看"设备数据"
5. 应显示上报的属性值

### 5.3 发送控制指令

**步骤：**
1. 控制台 → 设备 → waterer-001
2. 点击"设备控制"
3. 设置 `waterSwitch: true`
4. 发送
5. 水泵应启动

---

## 6. OTA 固件升级

### 6.1 方案

**ThingsCloud OTA 流程：**
1. 控制台上传固件包
2. 创建升级任务
3. 设备检查更新
4. 下载并刷写

### 6.2 ESP32 OTA 代码

```cpp
#include <HTTPUpdate.h>

void checkOTA() {
    // 从 ThingsCloud 获取最新版本号
    String version = getLatestVersion();
    
    if (version != CURRENT_VERSION) {
        String firmwareUrl = getFirmwareUrl(version);
        
        t_httpUpdate_return ret = httpUpdate.update(firmwareUrl);
        
        switch(ret) {
            case HTTP_UPDATE_FAILED:
                Serial.printf("OTA 失败：%s\n", httpUpdate.getLastErrorString().c_str());
                break;
            case HTTP_UPDATE_NO_UPDATES:
                Serial.println("无需更新");
                break;
            case HTTP_UPDATE_OK:
                Serial.println("更新成功，重启中...");
                ESP.restart();
                break;
        }
    }
}
```

---

## 7. 数据可视化

### 7.1 ThingsCloud 自带仪表盘

**功能：**
- 设备状态监控
- 历史数据图表
- 告警通知

### 7.2 自建 Dashboard

**方案 A：Grafana + InfluxDB**
```bash
# 安装 InfluxDB
docker run -d influxdb

# 安装 Grafana
docker run -d grafana/grafana

# 配置 ThingsCloud 数据导出
```

**方案 B：Node-RED**
```bash
npm install -g node-red
node-red
# 访问 http://localhost:1880
```

---

## 8. 自托管部署（可选）

### 8.1 Docker 部署

```bash
# 拉取镜像
docker pull thingscloud/server:latest

# 运行
docker run -d \
  -p 1883:1883 \
  -p 8080:8080 \
  -v ./data:/data \
  thingscloud/server:latest
```

### 8.2 配置

**docker-compose.yml：**
```yaml
version: '3'
services:
  thingscloud:
    image: thingscloud/server:latest
    ports:
      - "1883:1883"
      - "8080:8080"
    volumes:
      - ./data:/data
    environment:
      - TZ=Asia/Shanghai
    restart: always
```

---

## 9. 成本

### 云服务

| 版本 | 设备数 | 消息数 | 价格 |
|------|--------|--------|------|
| 免费版 | ≤10 | ≤10 万/月 | ¥0 |
| 专业版 | ≤100 | ≤100 万/月 | ¥99/月 |
| 企业版 | 不限 | 不限 | 定制 |

### 自托管

| 项目 | 成本 | 说明 |
|------|------|------|
| 服务器 | ¥24/月 | 腾讯云轻量 2 核 2G |
| 域名 | ¥60/年 | 可选 |
| **总计** | **¥24/月** | 支持 1000+ 设备 |

---

## 10. 常见问题

### Q1: 连接失败？
**A:** 检查 Access Key/Secret Key 是否正确，网络是否通畅。

### Q2: 消息收不到？
**A:** 确认主题格式正确，订阅和发布对应。

### Q3: 自托管如何配置域名？
**A:** 使用 Nginx 反向代理，配置 SSL 证书。

---

## 11. 参考资料

- [ThingsCloud 官网](https://thingscloud.xyz)
- [ThingsCloud GitHub](https://github.com/thingscloud)
- [MQTT 协议](https://mqtt.org)
- [ESP32 Arduino 教程](https://randomnerdtutorials.com)

---

*最后更新：2026-03-13*
