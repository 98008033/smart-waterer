# 🌱 智能浇花机

**基于 ESP32 + ThingsCloud 的 IoT 智能浇花系统**

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![ESP32](https://img.shields.io/badge/chip-ESP32--C3-green.svg)
![IoT](https://img.shields.io/badge/platform-ThingsCloud-yellow.svg)

---

## 🎯 项目简介

智能浇花机是一个低成本的 IoT 智能灌溉系统，可以：

- 📊 **实时监测** 土壤湿度、电池电量、水箱水位
- 📱 **远程控制** 手机 APP/微信小程序控制浇水
- ⏰ **自动浇水** 定时或根据湿度自动灌溉
- 🔋 **电池供电** 18650 锂电池，续航 30 天+
- ☁️ **云平台** ThingsCloud 免费 IoT 平台

**成本：** ¥200（自制）  
**开发周期：** 2 周  
**难度：** ⭐⭐⭐ 中等

---

## 📸 效果预览

```
┌─────────────────────────────────────┐
│   手机 APP      │   设备实物        │
│                 │                   │
│  💧 土壤湿度 65%│  ┌───────────┐   │
│  🔋 电池 85%    │  │ ESP32-C3  │   │
│  📦 水位 正常  │  │ 传感器     │   │
│                 │  │ 水泵      │   │
│  [一键浇水]    │  └───────────┘   │
└─────────────────────────────────────┘
```

---

## 🛠️ 核心组件

| 组件 | 技术选型 | 说明 |
|------|---------|------|
| **主控** | ESP32-C3 | WiFi+BLE，低功耗 |
| **传感器** | 电容式土壤湿度 | 防水耐腐蚀 |
| **云平台** | ThingsCloud | 免费 IoT 平台 ⭐ |
| **执行器** | 5V 微型水泵 | 流量 5L/min |
| **电源** | 18650×2 | 7.4V 5000mAh |

---

## 📋 功能清单

### 基础功能
- [x] WiFi 连接
- [x] 土壤湿度检测
- [x] 电池电量检测
- [x] 水箱水位检测
- [x] 水泵控制
- [x] 云平台数据上报
- [x] 手机远程控制

### 高级功能
- [ ] 自动定时浇水
- [ ] 根据湿度自动浇水
- [ ] 低电量告警
- [ ] 缺水告警
- [ ] OTA 固件升级
- [ ] 微信小程序
- [ ] 历史数据统计

---

## 🔧 快速开始

### 1. 准备硬件

```
ESP32-C3 开发板 ×1
土壤湿度传感器 ×1
水位传感器 ×1
5V 水泵 ×1
MOSFET 模块 ×1
面包板 + 杜邦线
```

详细清单：见 [BOM.md](docs/BOM.md)

### 2. 安装软件

```bash
# Arduino IDE
https://www.arduino.cc/en/software

# 库
PubSubClient
ArduinoJson
```

### 3. 配置云平台

1. 微信扫码登录 https://console.thingscloud.xyz
2. 获取私钥（密钥管理）
3. 创建主题 `waterer_001`

详细配置：见 [THINGSCLOUD_SETUP.md](docs/THINGSCLOUD_SETUP.md)

### 4. 烧录固件

```cpp
// 修改配置
const char* WIFI_SSID = "你的 WiFi";
const char* WIFI_PASS = "你的密码";
const char* MQTT_USER = "你的私钥";
const char* DEVICE_TOPIC = "waterer_001";

// 烧录
Arduino IDE → 上传
```

详细步骤：见 [QUICK_START.md](docs/QUICK_START.md)

---

## 📚 文档导航

### 入门
- [快速开始](docs/QUICK_START.md) ⭐ **必读**
- [项目计划](docs/PLAN.md)
- [常见问题](docs/FAQ.md)

### 硬件
- [硬件设计](docs/HARDWARE_DESIGN.md)
- [元件清单](docs/BOM.md)
- [安全风险评估](docs/SAFETY_RISK_ASSESSMENT.md) ⚠️ **重要**

### 软件
- [ThingsCloud 接入](docs/THINGSCLOUD_SETUP.md) ⭐ **必读**
- [固件开发指南](docs/FIRMWARE_DEVELOPMENT.md)
- [电池安全代码](firmware/main/battery_safety.c)

### IoT 平台
- [IoT 平台选型](docs/IOT_PLATFORM_RESEARCH.md)
- [微信小程序](docs/WECHAT_MINIPROGRAM.md)

### 其他
- [测试计划](docs/TEST_PLAN.md)
- [审查报告](docs/REVIEW_REPORT.md)

---

## 💰 成本分析

| 项目 | 成本 | 说明 |
|------|------|------|
| **硬件** | ¥200 | 含安全整改 |
| **云平台** | ¥0 | ThingsCloud 免费 |
| **PCB** | ¥15 | 嘉立创打样 |
| **外壳** | ¥25 | 防水盒 |
| **总计** | **¥240** | 量产可降至¥150 |

**对比商用：** 市面同类产品¥500-1000

---

## 🛡️ 安全提示

⚠️ **本项目涉及锂电池和水电混合，请务必注意：**

1. **电池安全** - 必须添加温度监控和保护板
2. **防水处理** - PCB 喷涂三防漆，外壳 IP65
3. **过流保护** - 添加自恢复保险丝
4. **使用环境** - 避免阳光直射和高温

详细安全要求：见 [SAFETY_RISK_ASSESSMENT.md](docs/SAFETY_RISK_ASSESSMENT.md)

---

## 🤝 贡献指南

欢迎提交 Issue 和 Pull Request！

### 开发分支
- `main` - 稳定版本
- `dev` - 开发版本
- `feature/*` - 新功能分支

### 提交规范
```
feat: 添加新功能
fix: 修复 bug
docs: 更新文档
style: 代码格式
refactor: 重构
test: 测试
chore: 构建/工具
```

---

## 📄 开源协议

MIT License - 详见 [LICENSE](LICENSE)

---

## 🙏 致谢

- [ESP32](https://www.espressif.com/) - 强大的主控芯片
- [ThingsCloud](https://thingscloud.xyz/) - 免费 IoT 平台
- [Arduino](https://www.arduino.cc/) - 友好的开发环境
- [嘉立创](https://www.jlcpcb.com/) - PCB 打样

---

## 📮 联系方式

- 项目 Issue: https://github.com/yourname/smart-waterer/issues
- 讨论区：GitHub Discussions

---

**🌟 如果这个项目对你有帮助，请给个 Star！**

*最后更新：2026-03-13*
