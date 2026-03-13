# IoT 平台选型报告

**项目：** 智能浇花机  
**日期：** 2026-03-13  
**状态：** 已选定 **ThingsCloud** ⭐

---

## 📊 平台对比总结

| 平台 | 类型 | 成本 (1 台/年) | 难度 | 推荐度 |
|------|------|---------------|------|--------|
| **ThingsCloud** | 开源 IoT | ¥0-288 | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| 腾讯云 IoT | 云服务 | ¥8.2 | ⭐⭐⭐ | ⭐⭐⭐ |
| EMQX Cloud | MQTT 云 | ¥0 (100 台) | ⭐⭐⭐ | ⭐⭐⭐⭐ |
| 自建 EMQX | 自托管 | ¥288/年 | ⭐⭐⭐⭐ | ⭐⭐⭐ |

---

## 🏆 最终选择：ThingsCloud

### 选择理由

1. **开源平台** - 代码透明，可审计
2. **可自托管** - 数据完全掌控
3. **功能完整** - 设备管理 + 可视化 + 规则
4. **灵活部署** - 云端/自建可选
5. **成本可控** - 免费版够用，自建更便宜

### 不选腾讯云的理由

1. **收费** - ¥0.8/台/月，10 台就是¥8/月
2. **闭源** - 无法审计代码
3. **配置复杂** - 物模型、产品、设备繁琐
4. **绑定厂商** - 数据迁移困难

---

## 💰 详细成本对比

### ThingsCloud

**免费版：**
```
设备数：≤10 台
消息数：≤10 万/月
功能：完整
────────
总计：¥0/年
```

**专业版：**
```
设备数：≤100 台
消息数：≤100 万/月
费用：¥99/月
────────
总计：¥1188/年
```

**自托管：**
```
服务器：¥24/月 × 12 = ¥288/年
域名：¥60/年
────────
总计：¥348/年（支持 1000+ 设备）
```

### 腾讯云 IoT

```
设备费：¥0.5/台/月 × 12 = ¥6/年
消息费：约¥1.2/年
OTA 费：约¥1/年
────────
总计：¥8.2/年/台
```

**10 台设备：**
- ThingsCloud 免费版：¥0
- ThingsCloud 自托管：¥348
- 腾讯云：¥82

---

## 📋 平台特性对比

| 特性 | ThingsCloud | 腾讯云 | EMQX |
|------|-------------|--------|------|
| MQTT | ✅ | ✅ | ✅ |
| HTTP API | ✅ | ✅ | ⚠️ 插件 |
| WebSocket | ✅ | ✅ | ✅ |
| CoAP | ⚠️ 插件 | ✅ | ✅ |
| 物模型 | ✅ | ✅ | ❌ |
| 规则引擎 | ✅ | ✅ | ✅ |
| 数据可视化 | ✅ | ⚠️ 需配置 | ❌ |
| OTA 升级 | ✅ | ✅ | ⚠️ 需开发 |
| 告警通知 | ✅ | ✅ | ⚠️ 插件 |
| 自托管 | ✅ | ❌ | ✅ |
| 开源 | ✅ | ❌ | ✅ |
| 多租户 | ✅ | ✅ | ⚠️ 企业版 |

---

## 🎯 接入复杂度

### ThingsCloud

```
1. 注册账号 (5 分钟)
2. 创建应用 (2 分钟)
3. 创建产品 (5 分钟)
4. 定义物模型 (10 分钟)
5. 创建设备 (2 分钟)
6. 获取连接信息 (1 分钟)
────────────────
总计：约 25 分钟
```

### 腾讯云 IoT

```
1. 注册 + 实名认证 (30 分钟)
2. 开通服务 (5 分钟)
3. 创建产品 (10 分钟)
4. 定义物模型 (15 分钟)
5. 创建设备 (5 分钟)
6. 获取三元组 (2 分钟)
7. 计算 MQTT 密码 (10 分钟)
────────────────
总计：约 77 分钟
```

---

## 🔧 代码对比

### ThingsCloud

```cpp
#define MQTT_SERVER "mqtt.thingscloud.xyz"
#define MQTT_USER "Access Key"
#define MQTT_PASS "Secret Key"
#define APP_ID "your_app_id"
#define DEVICE_ID "waterer-001"

// 主题
String pubTopic = "/app/" + APP_ID + "/device/" + DEVICE_ID + "/post";
String subTopic = "/app/" + APP_ID + "/device/" + DEVICE_ID + "/set";
```

### 腾讯云 IoT

```cpp
#define MQTT_SERVER "ABC123456.iotcloud.tencentdevices.com"
#define MQTT_USER "ABC123456waterer-001;12010126;ABC123456;1710316800"
#define MQTT_PASS "hmacsha1(DeviceSecret, timestamp)"

// 主题
String pubTopic = String(PRODUCT_ID) + "/" + DEVICE_NAME + "/data/update";
String subTopic = String(PRODUCT_ID) + "/" + DEVICE_NAME + "/s+/+";
```

**结论：** ThingsCloud 更简洁，无需复杂密码计算。

---

## 📊 数据格式对比

### ThingsCloud

**上报：**
```json
{
  "soilMoisture": 65,
  "batteryLevel": 85,
  "waterLevel": true
}
```

**控制：**
```json
{
  "waterSwitch": true,
  "waterDuration": 30
}
```

### 腾讯云 IoT

**上报：**
```json
{
  "id": "123",
  "version": "1.0",
  "params": {
    "soil_moisture": 65,
    "battery_level": 85
  }
}
```

**控制：**
```json
{
  "id": "456",
  "version": "1.0",
  "params": {
    "water_switch": 1
  }
}
```

**结论：** ThingsCloud 格式更简洁。

---

## 🚀 扩展性

### ThingsCloud

**垂直扩展：**
- 免费版 → 专业版 → 企业版
- 平滑升级，无需迁移

**水平扩展：**
- 自托管支持集群部署
- 支持负载均衡
- 数据库可替换（MySQL/PostgreSQL/InfluxDB）

### 腾讯云 IoT

**垂直扩展：**
- 公共实例 → 企业版实例
- 需要迁移

**水平扩展：**
- 腾讯云自动扩展
- 用户无需关心

---

## ⚠️ 注意事项

### ThingsCloud

1. **社区规模** - 相对较小，文档可能不完善
2. **企业支持** - 不如大厂完善
3. **自托管要求** - 需要一定运维能力

### 腾讯云 IoT

1. **成本** - 设备多时成本较高
2. **实名认证** - 需要身份证
3. **厂商绑定** - 迁移成本高

---

## 🎯 推荐场景

### 选择 ThingsCloud

- ✅ 个人开发者/小团队
- ✅ 需要数据自主可控
- ✅ 希望开源可审计
- ✅ 考虑未来自托管

### 选择腾讯云 IoT

- ✅ 企业用户
- ✅ 需要大厂背书
- ✅ 预算充足
- ✅ 不想运维

### 选择 EMQX Cloud

- ✅ 只需要 MQTT Broker
- ✅ 不需要物模型
- ✅ 追求极致性能

---

## 📞 官方资源

### ThingsCloud

- 官网：https://thingscloud.xyz
- GitHub: https://github.com/thingscloud
- 文档：查看官网

### 腾讯云 IoT

- 官网：https://cloud.tencent.com/product/iotexplorer
- 文档：https://cloud.tencent.com/document/product/1081

### EMQX

- 官网：https://www.emqx.com
- GitHub: https://github.com/emqx/emqx
- Cloud: https://www.emqx.com/zh/cloud

---

## ✅ 结论

**智能浇花机项目选择 ThingsCloud，理由：**

1. **成本低** - 免费版足够个人使用
2. **开源** - 代码透明，可审计
3. **灵活** - 可云端可自建
4. **功能完整** - 设备管理 + 可视化 + 规则
5. **数据可控** - 不被厂商绑定

---

**最后更新：** 2026-03-13  
**决策人：** 项目团队
