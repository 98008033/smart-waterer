/**
 * @file wifi_manager.h
 * @brief WiFi 连接管理模块
 * @version 0.4
 * @date 2026-03-13
 */

#ifndef __WIFI_MANAGER_H__
#define __WIFI_MANAGER_H__

#include <Arduino.h>
#include <WiFi.h>
#include <functional>

class WiFiManager {
public:
    WiFiManager();
    ~WiFiManager();
    
    // 初始化
    void begin(const char* ssid, const char* password);
    
    // 连接状态
    bool isConnected();
    int getSignalStrength();
    String getIPAddress();
    
    // 重连
    void reconnect();
    
    // 回调函数
    void onConnected(std::function<void()> callback);
    void onDisconnected(std::function<void()> callback);
    
    // 配网模式
    void startAPMode(const char* apName = "SmartWaterer-AP");
    void stopAPMode();
    
private:
    const char* _ssid;
    const char* _password;
    bool _connected;
    int _retryCount;
    
    std::function<void()> _onConnected;
    std::function<void()> _onDisconnected;
    
    void connectWiFi();
    void handleEvents();
};

#endif // __WIFI_MANAGER_H__
