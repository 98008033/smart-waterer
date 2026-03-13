/**
 * @file mqtt_client.h
 * @brief ThingsCloud MQTT 客户端
 * @version 0.4
 * @date 2026-03-13
 */

#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

#include <Arduino.h>
#include <PubSubClient.h>
#include <functional>

class MQTTClient {
public:
    MQTTClient();
    ~MQTTClient();
    
    // 初始化
    void begin(const char* host, uint16_t port);
    
    // 连接状态
    bool isConnected();
    void loop();
    
    // 发布/订阅
    bool publish(const char* topic, const char* payload);
    bool subscribe(const char* topic);
    
    // 回调函数
    void onConnected(std::function<void()> callback);
    void onDisconnected(std::function<void()> callback);
    void onMessage(std::function<void(const char*, const char*)> callback);
    
private:
    WiFiClient _wifiClient;
    PubSubClient _client;
    bool _connected;
    
    const char* _mqttUser;
    const char* _deviceTopic;
    
    std::function<void()> _onConnected;
    std::function<void()> _onDisconnected;
    std::function<void(const char*, const char*)> _onMessage;
    
    void connectMQTT();
    void callback(char* topic, byte* payload, unsigned int length);
};

#endif // __MQTT_CLIENT_H__
