# 腾讯云 IoT 平台接入指南

**项目：** 智能浇花机系统
**日期：** 2026-03-12
**方案：** 腾讯云 IoT 公共实例 + 微信小程序

---

## 1. 平台信息

| 项目 | 说明 |
|------|------|
| **平台** | 腾讯云 IoT 公共实例 |
| **官网** | https://console.cloud.tencent.com/iotexplorer |
| **免费额度** | 100 万消息/月，设备数无限制 |
| **协议** | MQTT 3.1.1 |
| **OTA** | 支持（COS 存储 + 物模型） |

---

## 2. 注册与准备

### 2.1 注册腾讯云账号

1. 访问：https://cloud.tencent.com
2. 点击"免费注册"
3. 选择"个人"账号类型
4. 完成实名认证（需要身份证）

### 2.2 开通 IoT 服务

1. 登录控制台：https://console.cloud.tencent.com
2. 搜索"IoT Explorer"
3. 点击进入"物联网开发平台"
4. 选择"公共实例"（免费）

---

## 3. 创建产品

### 3.1 创建步骤

1. 进入"公共实例" → "产品" → "新建产品"
2. 填写产品信息：
   - **产品名称：** Smart Waterer
   - **产品名称：** 智能浇花机
   - **通信协议：** MQTT
   - **联网方式：** WiFi
   - **数据协议：** 原生 MQTT
   - **品类：** 其他

3. 点击"确定"

### 3.2 产品三元组

创建成功后获取：
- **ProductID** （产品 ID）
- **DeviceName** （设备名称）
- **DeviceSecret** （设备密钥）

**⚠️ 重要：** 保存三元组，ESP32 连接需要！

---

## 4. 定义物模型

### 4.1 功能定义

| 功能 | 标识符 | 类型 | 读写 | 说明 |
|------|--------|------|------|------|
| 土壤湿度 | soil_moisture | 属性 | 只读 | 0-100% |
| 电池电量 | battery_level | 属性 | 只读 | 0-100% |
| 水箱水位 | water_level | 属性 | 只读 | 0/1 |
| 浇水开关 | water_switch | 属性 | 读写 | 0/1 |
| 浇水时长 | water_duration | 属性 | 读写 | 秒 |
| 浇水记录 | water_record | 事件 | 上报 | 时间 + 水量 |

### 4.2 物模型 JSON

```json
{
  "version": "1.0",
  "properties": [
    {
      "id": "soil_moisture",
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
      "id": "battery_level",
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
      "id": "water_level",
      "name": "水箱水位",
      "dataType": {
        "type": "bool",
        "0": "缺水",
        "1": "正常"
      },
      "accessMode": "r"
    },
    {
      "id": "water_switch",
      "name": "浇水开关",
      "dataType": {
        "type": "bool",
        "0": "关闭",
        "1": "开启"
      },
      "accessMode": "rw"
    },
    {
      "id": "water_duration",
      "name": "浇水时长",
      "dataType": {
        "type": "int",
        "min": 1,
        "max": 300,
        "unit": "秒"
      },
      "accessMode": "rw"
    }
  ],
  "events": [
    {
      "id": "water_record",
      "name": "浇水记录",
      "dataType": {
        "type": "struct",
        "specs": [
          {"id": "timestamp", "name": "时间", "dataType": {"type": "int"}},
          {"id": "amount", "name": "水量", "dataType": {"type": "int", "unit": "ml"}}
        ]
      }
    }
  ]
}
```

---

## 5. 创建设备

### 5.1 创建步骤

1. 进入产品 → "设备" → "新建设备"
2. 填写设备名称：`waterer-001`
3. 点击"确定"

### 5.2 获取设备信息

创建成功后获取：
- **ProductID**
- **DeviceName**
- **DeviceSecret**

**示例：**
```
ProductID: ABC123456
DeviceName: waterer-001
DeviceSecret: xyz789abcdef
```

---

## 6. ESP32 接入

### 6.1 连接信息

**MQTT 服务器：**
```
主机：ABC123456.iotcloud.tencentdevices.com
端口：8883 (TLS) 或 1883 (非加密)
ClientID: ABC123456waterer-001
Username: ABC123456waterer-001;12010126;ABC123456;1710316800
Password: 2d9b5e8c7a3f1d6e9b4c8a2f5d7e1c3b6a9d4f8e;hmacsha1
```

**密码生成公式：**
```
Password = hmacsha1(DeviceSecret, timestamp)
Username = ProductID + DeviceName + ProductID + timestamp
```

### 6.2 Arduino 代码示例

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi 配置
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// 腾讯云 IoT 配置
const char* PRODUCT_ID = "ABC123456";
const char* DEVICE_NAME = "waterer-001";
const char* DEVICE_SECRET = "xyz789abcdef";
const char* MQTT_SERVER = "ABC123456.iotcloud.tencentdevices.com";
const int MQTT_PORT = 8883;

// MQTT 客户端
WiFiClientSecure wifiClient;
PubSubClient client(MQTT_SERVER, MQTT_PORT, wifiClient);

// 主题
String subTopic;
String pubTopic;

void setup() {
    Serial.begin(115200);
    
    // 连接 WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    
    // 配置 MQTT 主题
    subTopic = String(PRODUCT_ID) + "/" + DEVICE_NAME + "/s+/+";
    pubTopic = String(PRODUCT_ID) + "/" + DEVICE_NAME + "/data/update";
    
    // 连接 MQTT
    connectMQTT();
}

void loop() {
    if (!client.connected()) {
        connectMQTT();
    }
    client.loop();
    
    // 上报数据（每 30 秒）
    static unsigned long lastReport = 0;
    if (millis() - lastReport > 30000) {
        reportData();
        lastReport = millis();
    }
}

void connectMQTT() {
    String clientId = String(PRODUCT_ID) + DEVICE_NAME;
    String username = String(PRODUCT_ID) + DEVICE_NAME + ";" + 
                      String(PRODUCT_ID) + ";" + String(time(nullptr));
    String password = generatePassword(DEVICE_SECRET, time(nullptr));
    
    while (!client.connected()) {
        Serial.print("Connecting to MQTT...");
        if (client.connect(clientId.c_str(), username.c_str(), password.c_str())) {
            Serial.println("connected");
            client.subscribe(subTopic.c_str());
        } else {
            Serial.print("failed, rc=");
            Serial.println(client.state());
            delay(5000);
        }
    }
}

void reportData() {
    // 读取传感器数据
    int soilMoisture = analogRead(34);
    int batteryLevel = analogRead(35);
    bool waterLevel = digitalRead(36);
    
    // 构建 JSON
    StaticJsonDocument<256> doc;
    doc["soil_moisture"] = map(soilMoisture, 0, 4095, 0, 100);
    doc["battery_level"] = map(batteryLevel, 0, 4095, 0, 100);
    doc["water_level"] = waterLevel ? 1 : 0;
    
    String payload;
    serializeJson(doc, payload);
    
    // 发布到腾讯云
    client.publish(pubTopic.c_str(), payload.c_str());
    Serial.println("Data reported: " + payload);
}

void callback(char* topic, byte* payload, unsigned int length) {
    // 处理下行指令
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    StaticJsonDocument<256> doc;
    deserializeJson(doc, message);
    
    if (doc.containsKey("water_switch")) {
        bool switchState = doc["water_switch"];
        // 控制水泵
        digitalWrite(5, switchState ? HIGH : LOW);
        Serial.println("Water switch: " + String(switchState));
    }
}

String generatePassword(String secret, long timestamp) {
    // HMAC-SHA1 实现（需要使用加密库）
    // 这里简化处理，实际需要使用 mbedTLS 或类似库
    return "your_hmac_sha1_result";
}
```

---

## 7. OTA 固件更新

### 7.1 配置步骤

1. 进入产品 → "固件" → "新建固件"
2. 填写固件信息：
   - **固件名称：** Smart Waterer v1.0
   - **固件版本：** 1.0.0
   - **固件类型：** ESP32
   - **上传方式：** 手动上传

3. 上传固件文件（.bin 格式）
4. 设置推送策略：
   - 手动推送
   - 灰度发布（推荐）

### 7.2 ESP32 OTA 代码

```cpp
#include <HTTPUpdate.h>

void checkOTA() {
    // 从腾讯云获取固件版本
    // 如果有新版本，下载并更新
    t_httpUpdate_return ret = httpUpdate.update(
        "https://your-cos-bucket.cos.ap-guangzhou.myqcloud.com/firmware/v1.0.1.bin"
    );
    
    switch(ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("OTA failed: %s\n", httpUpdate.getLastErrorString().c_str());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("No updates");
            break;
        case HTTP_UPDATE_OK:
            Serial.println("Update OK, restarting...");
            ESP.restart();
            break;
    }
}
```

---

## 8. 微信小程序接入

### 8.1 使用云开发模板

1. 访问：https://cloud.tencent.com/document/product/1081/45687
2. 下载"物联网开发平台小程序模板"
3. 导入微信开发者工具
4. 配置 AppID 和云开发环境

### 8.2 配置腾讯云

在小程序配置文件中填写：
```javascript
// config.js
module.exports = {
  appId: 'YOUR_WECHAT_APPID',
  cloud: {
    env: 'YOUR_CLOUD_ENV'
  },
  iot: {
    productId: 'ABC123456',
    deviceName: 'waterer-001'
  }
}
```

---

## 9. 成本估算

| 项目 | 免费额度 | 超出后价格 | 预估月成本 |
|------|---------|-----------|-----------|
| 消息数 | 100 万条/月 | ¥0.8/万条 | ¥0 |
| 规则引擎 | 1000 次/天 | ¥0.01/次 | ¥0 |
| 数据存储 | 1GB | ¥0.115/GB/天 | ¥0 |
| COS 存储 (OTA) | 50GB | ¥0.115/GB/天 | ¥0 |
| **总计** | - | - | **¥0/月** |

**注：** 个人项目完全在免费额度内！

---

## 10. 下一步行动

### 10.1 立即执行

- [ ] 注册腾讯云账号
- [ ] 完成实名认证
- [ ] 创建 IoT 公共实例
- [ ] 创建产品（Smart Waterer）
- [ ] 定义物模型
- [ ] 创建设备（waterer-001）
- [ ] 保存三元组

### 10.2 开发准备

- [ ] 安装 Arduino IDE
- [ ] 安装 ESP32 板支持
- [ ] 安装 PubSubClient 库
- [ ] 安装 ArduinoJson 库
- [ ] 准备 ESP32 开发板

### 10.3 开发计划

| 周次 | 任务 | 产出 |
|------|------|------|
| Week 1 | ESP32 固件开发 | MQTT 接入 + 传感器读取 |
| Week 2 | 小程序开发 | 使用模板快速接入 |
| Week 3 | 联调测试 | 完整功能测试 |

---

## 11. 参考资料

- [腾讯云 IoT Explorer 文档](https://cloud.tencent.com/document/product/1081)
- [MQTT 设备接入指南](https://cloud.tencent.com/document/product/1081/34947)
- [物模型定义规范](https://cloud.tencent.com/document/product/1081/35311)
- [OTA 固件升级](https://cloud.tencent.com/document/product/1081/35314)
- [微信小程序 IoT 开发](https://developers.weixin.qq.com/miniprogram/dev/framework/)

---

*最后更新：2026-03-12*
