# 智能浇花机 - 固件开发任务分解

**版本：** v0.4  
**日期：** 2026-03-13  
**状态：** 🟡 Pending

---

## 📋 任务总览

| 任务 | 状态 | 优先级 | 工时 | 说明 |
|------|------|--------|------|------|
| 开发环境搭建 | ⏳ Pending | P0 | 2h | PlatformIO + ESP32 支持 |
| 项目结构创建 | ⏳ Pending | P0 | 1h | 模块化目录结构 |
| WiFi 连接模块 | ⏳ Pending | P0 | 3h | WiFi 配网 + 重连 |
| ThingsCloud MQTT | ⏳ Pending | P0 | 4h | MQTT 连接 + 数据上报 |
| 传感器驱动 | ⏳ Pending | P0 | 4h | 土壤/水位/NTC/电池 |
| 水泵控制模块 | ⏳ Pending | P0 | 2h | PWM 控制 + 保护 |
| 自动浇水逻辑 | ⏳ Pending | P0 | 4h | 定时 + 湿度触发 |
| 电池安全监控 | ⏳ Pending | P0 | 3h | 温度/电压保护 |
| OTA 固件更新 | ⏳ Pending | P1 | 4h | HTTP OTA |
| 低功耗优化 | ⏳ Pending | P1 | 4h | Deep Sleep |
| 单元测试 | ⏳ Pending | P1 | 4h | 各模块测试 |
| 集成测试 | ⏳ Pending | P0 | 4h | 系统联调 |

**总计：** 38 小时

---

## 📁 代码结构

```
firmware/
├── src/
│   ├── main.cpp                  # 主程序入口
│   ├── config/
│   │   ├── config.h              # 编译配置
│   │   ├── pins.h                # 引脚定义
│   │   └── secrets.h.example     # 密钥模板
│   ├── core/
│   │   ├── wifi_manager.cpp      # WiFi 管理
│   │   ├── wifi_manager.h
│   │   ├── mqtt_client.cpp       # MQTT 客户端
│   │   ├── mqtt_client.h
│   │   └── ota_update.cpp        # OTA 更新
│   │   └── ota_update.h
│   ├── sensors/
│   │   ├── sensor_base.h         # 传感器基类
│   │   ├── soil_moisture.cpp     # 土壤湿度
│   │   ├── soil_moisture.h
│   │   ├── water_level.cpp       # 水位检测
│   │   ├── water_level.h
│   │   ├── battery_monitor.cpp   # 电池监控
│   │   ├── battery_monitor.h
│   │   ├── ntc_temperature.cpp   # NTC 温度
│   │   └── ntc_temperature.h
│   ├── actuators/
│   │   ├── pump_controller.cpp   # 水泵控制
│   │   ├── pump_controller.h
│   │   └── pump_controller.cpp.map
│   ├── logic/
│   │   ├── watering_logic.cpp    # 浇水逻辑
│   │   ├── watering_logic.h
│   │   ├── safety_manager.cpp    # 安全管理
│   │   └── safety_manager.h
│   └── utils/
│       ├── logger.h              # 日志工具
│       ├── eeprom_helper.cpp     # EEPROM 存储
│       └── eeprom_helper.h
├── include/
│   └── version.h                 # 版本信息
├── lib/                          # 第三方库
├── test/                         # 单元测试
│   ├── test_sensors.cpp
│   └── test_actuators.cpp
├── platformio.ini                # PlatformIO 配置
└── README.md                     # 固件说明
```

---

## 🔧 模块详细说明

### 1. 核心模块（core/）

#### WiFi 管理（wifi_manager）
**功能：**
- WiFi 连接（STA 模式）
- 自动重连
- 信号强度检测
- 配网模式（AP/SmartConfig）

**接口：**
```cpp
class WiFiManager {
public:
    void begin(const char* ssid, const char* password);
    bool isConnected();
    void reconnect();
    int getSignalStrength();
};
```

#### MQTT 客户端（mqtt_client）
**功能：**
- ThingsCloud MQTT 连接
- 数据上报（土壤/水位/电池）
- 命令接收（浇水控制）
- 断线重连

**接口：**
```cpp
class MQTTClient {
public:
    void begin(const char* host, uint16_t port);
    void publish(const char* topic, const char* payload);
    void subscribe(const char* topic);
    void loop();
};
```

#### OTA 更新（ota_update）
**功能：**
- HTTP OTA 下载
- 固件校验
- 断点续传
- 回滚机制

---

### 2. 传感器模块（sensors/）

#### 传感器基类（sensor_base）
```cpp
class Sensor {
public:
    virtual float read() = 0;
    virtual bool isAvailable() = 0;
    virtual void calibrate() = 0;
};
```

#### 土壤湿度（soil_moisture）
**功能：**
- ADC 读取
- 映射到 0-100%
- 滤波处理
- 校准功能

**引脚：** GPIO0

#### 水位检测（water_level）
**功能：**
- 数字输入读取
- 防抖动处理
- 状态变化检测

**引脚：** GPIO2

#### 电池监控（battery_monitor）
**功能：**
- 电池电压 ADC 读取
- 电量百分比计算
- 低电量报警
- 电压滤波

**引脚：** GPIO3（复用 NTC）

#### NTC 温度（ntc_temperature）
**功能：**
- NTC 电阻读取
- Steinhart-Hart 方程转换
- 温度保护（60°C 切断）
- 高温预警（50°C）

**引脚：** GPIO3（复用电池电压）

---

### 3. 执行器模块（actuators/）

#### 水泵控制（pump_controller）
**功能：**
- PWM 控制
- 流量计算
- 超时保护
- 温度保护联动

**引脚：** GPIO1

**接口：**
```cpp
class PumpController {
public:
    void begin();
    void start(uint16_t duration_ms);
    void stop();
    bool isRunning();
    void setSafetyLock(bool locked);
};
```

---

### 4. 逻辑模块（logic/）

#### 浇水逻辑（watering_logic）
**功能：**
- 定时浇水
- 土壤湿度触发
- App 远程控制
- 浇水历史记录

**状态机：**
```
IDLE → CHECK_SOIL → WATERING → COOLDOWN → IDLE
```

#### 安全管理（safety_manager）
**功能：**
- 温度监控（60°C 切断）
- 电压监控（低电保护）
- 过流保护联动
- 故障日志

**保护阈值：**
| 参数 | 警告值 | 切断值 |
|------|--------|--------|
| 温度 | 50°C | 60°C |
| 电池电压 | 3.5V | 3.0V |

---

### 5. 工具模块（utils/）

#### 日志工具（logger）
```cpp
#define LOG_I(tag, msg) Serial.printf("[%s] [I] %s\n", tag, msg)
#define LOG_W(tag, msg) Serial.printf("[%s] [W] %s\n", tag, msg)
#define LOG_E(tag, msg) Serial.printf("[%s] [E] %s\n", tag, msg)
```

#### EEPROM 存储（eeprom_helper）
**存储内容：**
- WiFi 配置
- 浇水设置
- 校准数据
- 运行统计

---

## 📝 开发顺序

### Phase 1: 基础框架（4h）
1. ✅ 创建项目结构
2. ✅ 配置 PlatformIO
3. ⏳ 实现 WiFi 管理
4. ⏳ 实现日志工具

### Phase 2: 传感器驱动（8h）
1. ⏳ 实现传感器基类
2. ⏳ 实现土壤湿度
3. ⏳ 实现水位检测
4. ⏳ 实现电池监控
5. ⏳ 实现 NTC 温度

### Phase 3: 执行器控制（4h）
1. ⏳ 实现水泵控制
2. ⏳ 实现安全管理
3. ⏳ 单元测试

### Phase 4: 业务逻辑（8h）
1. ⏳ 实现浇水逻辑
2. ⏳ 实现 MQTT 客户端
3. ⏳ 集成测试

### Phase 5: 高级功能（8h）
1. ⏳ 实现 OTA 更新
2. ⏳ 实现低功耗
3. ⏳ 系统优化

### Phase 6: 测试优化（6h）
1. ⏳ 集成测试
2. ⏳ 性能优化
3. ⏳ 文档完善

---

## 🎯 验收标准

### 功能验收
- [ ] WiFi 连接稳定
- [ ] MQTT 数据上报正常
- [ ] 传感器读数准确
- [ ] 水泵控制可靠
- [ ] 自动浇水逻辑正确
- [ ] 温度保护触发正常
- [ ] OTA 升级成功

### 性能验收
- [ ] 待机电流 < 100μA
- [ ] 响应延迟 < 3 秒
- [ ] 续航 ≥ 280 天
- [ ] 内存占用 < 80%

### 代码质量
- [ ] 模块化设计
- [ ] 代码注释完整
- [ ] 单元测试通过
- [ ] 无内存泄漏

---

## 📚 参考资料

- [ESP32 Arduino 核心](https://docs.espressif.com/projects/arduino-esp32/)
- [ThingsCloud 文档](https://thingscloud.xyz)
- [PlatformIO 文档](https://docs.platformio.org/)
- [MQTT 协议规范](https://mqtt.org/)

---

*最后更新：2026-03-13*  
*状态：Pending*
