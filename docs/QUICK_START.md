# 🚀 智能浇花机 - 快速开始指南

**最后更新：** 2026-03-13  
**IoT 平台：** ThingsCloud（免费）⭐

---

## 📋 准备工作

### 硬件清单

| 名称 | 数量 | 说明 |
|------|------|------|
| ESP32-C3 开发板 | 1 | 主控制器 |
| 土壤湿度传感器 | 1 | 电容式防水 |
| 水位传感器 | 1 | 非接触式 |
| 5V 水泵 | 1 | 微型潜水泵 |
| MOSFET 模块 | 1 | 水泵驱动 |
| 面包板 + 杜邦线 | 1 批 | 原型开发 |
| USB 数据线 | 1 | 供电 + 烧录 |

### 软件准备

- [ ] Arduino IDE：https://www.arduino.cc/en/software
- [ ] ESP32 板支持：安装教程见下文
- [ ] 库：PubSubClient, ArduinoJson

### 云平台

- [ ] ThingsCloud 账号：微信扫码登录 https://console.thingscloud.xyz
- [ ] 获取私钥：控制台 → 密钥管理
- [ ] 创建主题：例如 `waterer_001`

**详细云平台配置：** 见 `THINGSCLOUD_SETUP.md`

---

## 🔧 步骤 1：安装 Arduino IDE

### Windows/Mac

1. 访问：https://www.arduino.cc/en/software
2. 下载并安装
3. 打开 Arduino IDE

### Linux

```bash
sudo snap install arduino
```

**⏰ 耗时：** 10 分钟

---

## 🔧 步骤 2：安装 ESP32 支持

1. 打开 Arduino IDE
2. 点击 **文件** → **首选项**
3. 在"附加开发板管理器网址"输入：
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
4. 点击 **工具** → **开发板** → **开发板管理器**
5. 搜索 `ESP32`
6. 安装 **ESP32 by Espressif Systems**
7. 安装完成

**⏰ 耗时：** 5 分钟

---

## 🔧 步骤 3：安装库

1. 点击 **工具** → **管理库**
2. 搜索并安装：
   - `PubSubClient` (MQTT 客户端)
   - `ArduinoJson` (JSON 解析)
3. 安装完成

**⏰ 耗时：** 3 分钟

---

## 🔧 步骤 4：配置代码

### 4.1 下载固件代码

从 `firmware/main/thingscloud_mqtt.ino` 复制代码

### 4.2 修改配置

打开代码，修改以下部分：

```cpp
// WiFi 配置
const char* WIFI_SSID = "你的 WiFi 名称";
const char* WIFI_PASS = "你的 WiFi 密码";

// ThingsCloud 配置
const char* MQTT_SERVER = "bemfa.com";
const int MQTT_PORT = 1883;
const char* MQTT_USER = "你的巴法云私钥";  // 控制台 → 密钥管理
const char* DEVICE_TOPIC = "waterer_001";  // 你的主题名
```

**⏰ 耗时：** 2 分钟

---

## 🔧 步骤 5：连接硬件

### 最小系统连接

```
ESP32-C3          传感器/模块
─────────────────────────────
GPIO3 (ADC)   ──→ 土壤湿度 AO
GPIO2         ──→ 水位检测 OUT
GPIO1         ──→ MOSFET 基极
GPIO4         ──→ LED+
GPIO5         ──→ 蜂鸣器+
3.3V          ──→ 传感器 VCC
5V            ──→ 水泵+
GND           ──→ 所有 GND
```

### 详细接线图

见 `HARDWARE_DESIGN.md` 文档

**⏰ 耗时：** 15 分钟

---

## 🔧 步骤 6：烧录固件

1. ESP32 连接电脑（USB 线）
2. Arduino IDE 选择开发板：
   - **工具** → **开发板** → **ESP32 Arduino** → **XiaoS3** 或 **ESP32C3 Dev Module**
3. 选择端口：
   - **工具** → **端口** → 选择 COM 口
4. 点击 **上传** 按钮（→）
5. 等待编译和上传完成

**⏰ 耗时：** 2 分钟

---

## 🔧 步骤 7：测试验证

### 7.1 打开串口监视器

1. 点击 **工具** → **串口监视器**
2. 波特率选择 `115200`
3. 查看输出

**正常输出：**
```
连接 WiFi: MyWiFi
.....
WiFi 已连接
IP 地址：192.168.1.100
连接 MQTT...已连接
订阅主题：waterer_001
===== 系统启动完成 =====
上报数据：soil_moisture:65,battery_level:85,water_level:1,temp:25.5
```

### 7.2 检查云平台

1. 打开 ThingsCloud 控制台
2. 进入 **我的主题** → `waterer_001`
3. 查看 **消息记录**
4. 应该看到设备上报的数据

### 7.3 发送控制指令

1. 在控制台 → 主题 → **发送消息**
2. 输入：`water_switch:1`
3. 点击发送
4. 水泵应启动

**⏰ 耗时：** 5 分钟

---

## 🎉 完成！

现在你的智能浇花机已经：
- ✅ 连接到 ThingsCloud
- ✅ 上报传感器数据
- ✅ 接收控制指令
- ✅ 可以远程浇水

---

## 📱 下一步

### 手机控制

**方案 1：ThingsCloud 控制台**
- 直接用云平台发送指令
- 网址：https://console.thingscloud.xyz

**方案 2：微信小程序**
- 开发小程序（见 `WECHAT_MINIPROGRAM.md`）
- 或使用现成模板

**方案 3：Web 控制台**
- 自建 Web 界面
- 使用 Node-RED 或 Vue

### 自动化

**定时浇水：**
```cpp
// 在 loop 中添加
if (hour() == 8 && minute() == 0) {
    pump_on();
    delay(30000);  // 浇 30 秒
    pump_off();
}
```

**根据土壤湿度自动浇水：**
```cpp
int moisture = readSoilMoisture();
if (moisture < 30) {
    pump_on();
    delay(30000);
    pump_off();
}
```

---

## 🛠️ 故障排查

### 问题 1：WiFi 连接失败
- 检查 WiFi 名称和密码
- 确认 2.4G WiFi（ESP32 不支持 5G）
- 重启 ESP32

### 问题 2：MQTT 连接失败
- 检查私钥是否正确
- 检查主题名是否正确
- 确认网络通畅

### 问题 3：传感器读数异常
- 检查接线
- 校准传感器（见 `HARDWARE_DESIGN.md`）
- 更换传感器测试

---

## 📚 相关文档

| 文档 | 说明 |
|------|------|
| `THINGSCLOUD_SETUP.md` | ThingsCloud 详细接入 |
| `HARDWARE_DESIGN.md` | 硬件设计和接线 |
| `FIRMWARE_DEVELOPMENT.md` | 固件开发指南 |
| `SAFETY_RISK_ASSESSMENT.md` | 安全风险评估 |
| `BOM.md` | 元件采购清单 |

---

## 💡 提示

1. **首次开发**建议先用面包板搭建原型
2. **测试完成**后再焊接到 PCB
3. **安全整改**必须完成（见安全评估报告）
4. **防水处理**很重要（浇花机环境潮湿）

---

**祝你开发顺利！** 🎉

*最后更新：2026-03-13*
