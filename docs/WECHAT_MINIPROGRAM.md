# 📱 微信小程序开发指南

**最后更新：** 2026-03-13  
**IoT 平台：** ThingsCloud

---

## 🎯 方案选择

### 方案 1：云开发 + ThingsCloud API ⭐ 推荐

**优点：**
- ✅ 开发简单
- ✅ 无需后端服务器
- ✅ 免费额度够用
- ✅ 直接调用 ThingsCloud API

**缺点：**
- ⚠️ 依赖微信云开发

**成本：** ¥0（免费额度内）

### 方案 2：自建后端 + MQTT

**优点：**
- ✅ 完全控制
- ✅ 功能强大

**缺点：**
- ⚠️ 需要服务器
- ⚠️ 开发复杂

**成本：** ¥24/月（服务器）

---

## 🚀 方案 1：云开发快速接入

### 步骤 1：创建小程序

1. 注册微信小程序账号
   - https://mp.weixin.qq.com
2. 下载微信开发者工具
   - https://developers.weixin.qq.com/miniprogram/dev/devtools/download.html
3. 创建小程序项目
   - AppID：使用测试号或正式号
   - 模板：选择"云开发模板"

### 步骤 2：启用云开发

1. 打开微信开发者工具
2. 点击 **云开发** 按钮
3. 创建云开发环境
   - 环境名称：`waterer-dev`
   - 版本：免费版
4. 记录环境 ID

### 步骤 3：配置小程序

**project.config.json：**
```json
{
  "cloudfunctionRoot": "cloudfunctions/",
  "miniprogramRoot": "miniprogram/",
  "cloudfunctionTemplateRoot": "cloudfunctionTemplate/",
  "setting": {
    "es6": true,
    "minified": true
  },
  "cloudbaseRoot": "cloudbaserc.json"
}
```

### 步骤 4：创建云函数

**目录结构：**
```
cloudfunctions/
└── controlDevice/
    ├── index.js
    └── package.json
```

**index.js：**
```javascript
const mqtt = require('mqtt');

exports.main = async (event, context) => {
    const { action, value } = event;
    
    // 连接 ThingsCloud MQTT
    const client = mqtt.connect('mqtt://bemfa.com:1883');
    
    return new Promise((resolve, reject) => {
        client.on('connect', () => {
            console.log('MQTT 已连接');
            
            // 发布控制指令
            const topic = 'waterer_001';
            const message = `${action}:${value}`;
            
            client.publish(topic, message, (err) => {
                client.end();
                if (err) {
                    reject(err);
                } else {
                    resolve({
                        success: true,
                        message: `已发送：${message}`
                    });
                }
            });
        });
        
        client.on('error', (err) => {
            reject(err);
        });
    });
};
```

**package.json：**
```json
{
  "name": "controlDevice",
  "version": "1.0.0",
  "description": "控制设备云函数",
  "main": "index.js",
  "dependencies": {
    "mqtt": "^5.0.0"
  }
}
```

### 步骤 5：小程序前端代码

**pages/index/index.js：**
```javascript
Page({
  data: {
    soilMoisture: 0,
    batteryLevel: 0,
    waterLevel: false,
    pumpState: false
  },

  onLoad() {
    // 加载设备状态
    this.loadDeviceStatus();
    // 定时刷新（30 秒）
    setInterval(() => this.loadDeviceStatus(), 30000);
  },

  // 加载设备状态
  async loadDeviceStatus() {
    try {
      // 从 ThingsCloud 获取最新数据
      const res = await wx.cloud.callFunction({
        name: 'getDeviceStatus'
      });
      
      if (res.result.success) {
        this.setData({
          soilMoisture: res.result.data.soil_moisture,
          batteryLevel: res.result.data.battery_level,
          waterLevel: res.result.data.water_level
        });
      }
    } catch (err) {
      console.error('加载状态失败:', err);
    }
  },

  // 控制水泵
  async controlPump(action) {
    wx.showLoading({ title: '执行中...' });
    
    try {
      const res = await wx.cloud.callFunction({
        name: 'controlDevice',
        data: {
          action: 'water_switch',
          value: action ? 1 : 0
        }
      });
      
      if (res.result.success) {
        wx.showToast({
          icon: 'success',
          title: action ? '已开启浇水' : '已停止浇水'
        });
        
        this.setData({ pumpState: action });
      }
    } catch (err) {
      wx.showToast({
        icon: 'none',
        title: '操作失败'
      });
    } finally {
      wx.hideLoading();
    }
  },

  // 一键浇水
  async oneClickWater() {
    await this.controlPump(true);
    
    // 30 秒后自动关闭
    setTimeout(() => {
      this.controlPump(false);
    }, 30000);
  }
});
```

**pages/index/index.wxml：**
```xml
<view class="container">
  <view class="header">
    <text class="title">🌱 智能浇花机</text>
  </view>

  <!-- 状态卡片 -->
  <view class="status-card">
    <view class="status-item">
      <text class="label">土壤湿度</text>
      <text class="value">{{soilMoisture}}%</text>
      <progress percent="{{soilMoisture}}" color="#4CAF50" />
    </view>

    <view class="status-item">
      <text class="label">电池电量</text>
      <text class="value">{{batteryLevel}}%</text>
      <progress percent="{{batteryLevel}}" color="#2196F3" />
    </view>

    <view class="status-item">
      <text class="label">水箱水位</text>
      <text class="value">{{waterLevel ? '正常' : '缺水'}}</text>
    </view>
  </view>

  <!-- 控制按钮 -->
  <view class="control-section">
    <button class="btn-water" bindtap="oneClickWater">
      💧 一键浇水
    </button>

    <button class="btn-switch" bindtap="controlPump" data-action="{{!pumpState}}">
      {{pumpState ? '停止浇水' : '开启浇水'}}
    </button>
  </view>
</view>
```

**pages/index/index.wxss：**
```css
.container {
  padding: 20rpx;
}

.header {
  text-align: center;
  padding: 40rpx 0;
}

.title {
  font-size: 36rpx;
  font-weight: bold;
}

.status-card {
  background: #fff;
  border-radius: 20rpx;
  padding: 30rpx;
  margin-bottom: 30rpx;
  box-shadow: 0 4rpx 12rpx rgba(0,0,0,0.1);
}

.status-item {
  margin-bottom: 30rpx;
}

.label {
  font-size: 28rpx;
  color: #666;
}

.value {
  font-size: 48rpx;
  font-weight: bold;
  color: #333;
  float: right;
}

.control-section {
  display: flex;
  flex-direction: column;
  gap: 20rpx;
}

.btn-water {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  border: none;
  border-radius: 20rpx;
  padding: 30rpx;
  font-size: 32rpx;
}

.btn-switch {
  background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
  color: white;
  border: none;
  border-radius: 20rpx;
  padding: 30rpx;
  font-size: 32rpx;
}
```

---

## 📊 方案 2：自建后端（可选）

### 架构

```
小程序 ←→ Node.js 后端 ←→ ThingsCloud MQTT
```

### Node.js 后端代码

**server.js：**
```javascript
const express = require('express');
const mqtt = require('mqtt');
const cors = require('cors');

const app = express();
app.use(cors());
app.use(express.json());

// 连接 MQTT
const client = mqtt.connect('mqtt://bemfa.com:1883');

let deviceStatus = {};

client.on('connect', () => {
    console.log('MQTT 已连接');
    client.subscribe('waterer_001');
});

client.on('message', (topic, message) => {
    // 解析设备上报数据
    const data = parseMessage(message.toString());
    deviceStatus = data;
    console.log('设备状态:', data);
});

// API: 获取设备状态
app.get('/api/status', (req, res) => {
    res.json({
        success: true,
        data: deviceStatus
    });
});

// API: 控制设备
app.post('/api/control', (req, res) => {
    const { action, value } = req.body;
    const message = `${action}:${value}`;
    
    client.publish('waterer_001', message);
    
    res.json({
        success: true,
        message: `已发送：${message}`
    });
});

app.listen(3000, () => {
    console.log('服务器运行在 http://localhost:3000');
});

function parseMessage(msg) {
    // 解析简化格式：soil_moisture:65,battery_level:85
    const data = {};
    const pairs = msg.split(',');
    pairs.forEach(pair => {
        const [key, value] = pair.split(':');
        data[key] = isNaN(value) ? value : Number(value);
    });
    return data;
}
```

---

## 🔗 ThingsCloud HTTP API

如果不想用 MQTT，可以用 HTTP API：

### 推送数据

```http
POST https://api.thingscloud.xyz/push
Content-Type: application/json

{
    "topic": "waterer_001",
    "data": "soil_moisture:65,battery_level:85"
}
```

### 获取最新数据

```http
GET https://api.thingscloud.xyz/pull?topic=waterer_001
```

**响应：**
```json
{
    "status": 200,
    "msg": "ok",
    "data": {
        "soil_moisture": "65",
        "battery_level": "85"
    }
}
```

---

## 🎨 UI 设计建议

### 配色方案

```css
主色：#4CAF50（绿色 - 植物）
辅色：#2196F3（蓝色 - 水）
警告：#FF5722（橙色 - 缺水/低电）
```

### 图标推荐

- 💧 浇水
- 🌱 植物
- 🔋 电池
- 📦 水箱
- ⏰ 定时

---

## 📝 下一步

1. **完善云函数** - 添加错误处理
2. **优化 UI** - 美化界面
3. **添加功能** - 定时设置、历史记录
4. **测试发布** - 提交审核

---

## 📚 参考资料

- [微信小程序官方文档](https://developers.weixin.qq.com/miniprogram/dev/framework/)
- [云开发文档](https://developers.weixin.qq.com/miniprogram/dev/wxcloud/basis/getting-started.html)
- [ThingsCloud API](https://docs.bemfa.com)

---

*最后更新：2026-03-13*
