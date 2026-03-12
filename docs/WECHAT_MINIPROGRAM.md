# 微信小程序开发指南 📱

**基于腾讯云 IoT + 云开发模板**

---

## 1. 准备工作

### 1.1 注册微信小程序

```
1. 访问：https://mp.weixin.qq.com
2. 点击"立即注册"
3. 选择"小程序"
4. 填写信息：
   - 邮箱
   - 密码
   - 验证码
5. 激活邮箱
6. 信息登记（个人主体）
7. 完成注册
```

**⏰ 耗时：** 10 分钟

### 1.2 开通云开发

```
1. 登录小程序后台
2. 点击"云开发"
3. 开通云开发
4. 创建环境：
   - 环境名称：smart-waterer
   - 计费模式：免费版
5. 完成创建
```

**⏰ 耗时：** 5 分钟

### 1.3 下载微信开发者工具

**Windows:**
```
1. 访问：https://developers.weixin.qq.com/miniprogram/dev/devtools/download.html
2. 下载 Windows 稳定版
3. 运行安装程序
4. 安装完成
```

**Mac:**
```
1. 访问：https://developers.weixin.qq.com/miniprogram/dev/devtools/download.html
2. 下载 Mac 稳定版
3. 拖拽到应用程序文件夹
4. 打开微信开发者工具
```

**⏰ 耗时：** 10 分钟

---

## 2. 使用 IoT 模板

### 2.1 下载模板

**方式一：GitHub 下载**
```
1. 访问：https://github.com/wechat-miniprogram/miniprogram-demo
2. 搜索"IoT"
3. 下载 IoT 模板
4. 解压到本地
```

**方式二：云开发控制台下载**
```
1. 登录云开发控制台
2. 点击"模板市场"
3. 搜索"IoT"
4. 点击"使用模板"
```

**⏰ 耗时：** 5 分钟

### 2.2 导入项目

```
1. 打开微信开发者工具
2. 点击"+"号
3. 选择"导入项目"
4. 选择模板目录
5. 填写 AppID（小程序的 AppID）
6. 点击"导入"
```

**⏰ 耗时：** 3 分钟

### 2.3 配置云开发

```
1. 打开 project.config.json
2. 修改 cloudfunctionRoot 和 miniprogramRoot
3. 打开 utils/config.js
4. 修改环境 ID：
   module.exports = {
     envId: 'smart-waterer-xxx',
     region: 'ap-guangzhou'
   }
```

**⏰ 耗时：** 5 分钟

---

## 3. 配置腾讯云 IoT

### 3.1 修改配置文件

**文件：** `utils/config.js`

```javascript
module.exports = {
  // 云开发环境
  envId: 'smart-waterer-xxx',
  region: 'ap-guangzhou',
  
  // 腾讯云 IoT 配置
  iot: {
    productId: 'ABC123456',  // 替换为你的 ProductID
    deviceName: 'waterer-001'  // 替换为你的设备名
  },
  
  // MQTT 配置
  mqtt: {
    host: 'ABC123456.iotcloud.tencentdevices.com',
    port: 8883,
    clientId: 'ABC123456waterer-001'
  }
}
```

**⏰ 耗时：** 5 分钟

### 3.2 获取设备信息

**从腾讯云 IoT 控制台获取：**
```
1. 访问：https://console.cloud.tencent.com/iotexplorer
2. 进入产品 → 设备列表
3. 点击设备名称
4. 复制：
   - ProductID
   - DeviceName
   - DeviceSecret
```

---

## 4. 界面定制

### 4.1 修改首页

**文件：** `pages/index/index.wxml`

```xml
<view class="container">
  <!-- 标题 -->
  <view class="header">
    <text class="title">智能浇花机</text>
  </view>
  
  <!-- 数据显示 -->
  <view class="data-section">
    <view class="data-item">
      <text class="label">土壤湿度</text>
      <text class="value">{{soilMoisture}}%</text>
    </view>
    <view class="data-item">
      <text class="label">电池电量</text>
      <text class="value">{{batteryLevel}}%</text>
    </view>
    <view class="data-item">
      <text class="label">水箱水位</text>
      <text class="value">{{waterLevel ? '正常' : '缺水'}}</text>
    </view>
  </view>
  
  <!-- 控制按钮 -->
  <view class="control-section">
    <button class="btn" bindtap="toggleWater">
      {{isWatering ? '停止浇水' : '开始浇水'}}
    </button>
  </view>
</view>
```

**⏰ 耗时：** 10 分钟

### 4.2 修改样式

**文件：** `pages/index/index.wxss`

```css
.container {
  padding: 20rpx;
}

.header {
  text-align: center;
  margin-bottom: 40rpx;
}

.title {
  font-size: 40rpx;
  font-weight: bold;
}

.data-section {
  display: flex;
  justify-content: space-around;
  margin-bottom: 40rpx;
}

.data-item {
  text-align: center;
}

.label {
  font-size: 28rpx;
  color: #666;
}

.value {
  font-size: 48rpx;
  color: #07c160;
  font-weight: bold;
}

.control-section {
  text-align: center;
}

.btn {
  background-color: #07c160;
  color: white;
  font-size: 32rpx;
  padding: 20rpx 60rpx;
}
```

**⏰ 耗时：** 10 分钟

### 4.3 修改逻辑

**文件：** `pages/index/index.js`

```javascript
const app = getApp()
const config = require('../../utils/config')

Page({
  data: {
    soilMoisture: 0,
    batteryLevel: 0,
    waterLevel: false,
    isWatering: false
  },
  
  onLoad() {
    this.connectMQTT()
  },
  
  // 连接 MQTT
  connectMQTT() {
    const mqtt = require('../../utils/mqtt')
    mqtt.connect({
      host: config.mqtt.host,
      port: config.mqtt.port,
      clientId: config.mqtt.clientId,
      onSuccess: () => {
        console.log('MQTT connected')
        this.subscribe()
      }
    })
  },
  
  // 订阅主题
  subscribe() {
    const mqtt = require('../../utils/mqtt')
    const topic = `${config.iot.productId}/${config.iot.deviceName}/s+/+`
    mqtt.subscribe(topic, (message) => {
      this.handleMessage(message)
    })
  },
  
  // 处理消息
  handleMessage(message) {
    const data = JSON.parse(message)
    this.setData({
      soilMoisture: data.soil_moisture || 0,
      batteryLevel: data.battery_level || 0,
      waterLevel: data.water_level || false
    })
  },
  
  // 切换浇水
  toggleWater() {
    const mqtt = require('../../utils/mqtt')
    const topic = `${config.iot.productId}/${config.iot.deviceName}/data/update`
    const message = {
      water_switch: !this.data.isWatering
    }
    mqtt.publish(topic, JSON.stringify(message))
    this.setData({
      isWatering: !this.data.isWatering
    })
  }
})
```

**⏰ 耗时：** 20 分钟

---

## 5. 测试与调试

### 5.1 模拟器测试

```
1. 微信开发者工具 → 模拟器
2. 查看界面显示
3. 点击按钮测试
4. 查看控制台输出
```

**⏰ 耗时：** 10 分钟

### 5.2 真机测试

```
1. 点击"预览"
2. 扫码打开小程序
3. 测试各项功能
4. 检查数据显示
```

**⏰ 耗时：** 10 分钟

### 5.3 常见问题

| 问题 | 原因 | 解决方案 |
|------|------|---------|
| 无法连接 MQTT | 配置错误 | 检查 ProductID 和设备信息 |
| 数据显示 0 | 设备未上报 | 检查设备是否在线 |
| 按钮无响应 | 代码错误 | 查看控制台报错 |
| 样式错乱 | WXSS 错误 | 检查样式文件 |

---

## 6. 提交审核

### 6.1 准备材料

```
- 小程序截图（5 张）
- 功能介绍（100 字内）
- 测试账号（如需登录）
- 隐私政策链接
```

### 6.2 提交流程

```
1. 微信开发者工具 → 上传
2. 填写版本号
3. 填写版本说明
4. 登录小程序后台
5. 版本管理 → 提交审核
6. 填写审核信息
7. 提交
```

**审核时间：** 1-3 天

---

## 7. 发布上线

### 7.1 发布流程

```
1. 审核通过后
2. 小程序后台 → 版本管理
3. 点击"提交发布"
4. 确认发布
5. 等待发布完成
```

**⏰ 耗时：** 5 分钟

### 7.2 推广分享

```
- 生成小程序码
- 分享给朋友
- 发布到朋友圈
- 添加到我的小程序
```

---

## 8. 进阶功能

### 8.1 添加图表

```javascript
// 使用 ECharts for Weixin
const ec = require('../../components/ec-canvas/echarts')

function initChart(canvas, width, height) {
  const chart = ec.init(canvas, null, {
    width: width,
    height: height
  })
  
  const option = {
    xAxis: {
      type: 'category',
      data: ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun']
    },
    yAxis: {
      type: 'value'
    },
    series: [{
      data: [820, 932, 901, 934, 1290, 1330, 1320],
      type: 'line'
    }]
  }
  
  chart.setOption(option)
  return chart
}
```

### 8.2 添加通知

```javascript
// 订阅消息
wx.requestSubscribeMessage({
  tmplIds: ['TEMPLATE_ID'],
  success(res) {
    if (res.errMsg === 'requestSubscribeMessage:ok') {
      console.log('订阅成功')
    }
  }
})
```

### 8.3 添加历史记录

```javascript
// 云函数查询历史记录
const cloud = require('wx-server-sdk')
cloud.init()

exports.main = async (event, context) => {
  const db = cloud.database()
  const result = await db.collection('water_records')
    .where({
      deviceId: event.deviceId
    })
    .orderBy('timestamp', 'desc')
    .limit(10)
    .get()
  
  return result.data
}
```

---

## 9. 完整项目结构

```
smart-waterer-miniprogram/
├── app.js                    # 小程序入口
├── app.json                  # 小程序配置
├── app.wxss                  # 全局样式
├── project.config.json       # 项目配置
├── pages/
│   └── index/
│       ├── index.wxml       # 首页界面
│       ├── index.wxss       # 首页样式
│       ├── index.js         # 首页逻辑
│       └── index.json       # 首页配置
├── utils/
│   ├── config.js            # 配置文件
│   ├── mqtt.js              # MQTT 客户端
│   └── util.js              # 工具函数
├── components/              # 组件目录
│   └── ec-canvas/          # ECharts 组件
└── cloudfunctions/         # 云函数目录
    └── getHistory/         # 获取历史记录
```

---

**祝你开发顺利！** 🚀

*最后更新：2026-03-12*
