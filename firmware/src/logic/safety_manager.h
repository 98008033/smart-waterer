/**
 * @file safety_manager.h
 * @brief 安全管理模块
 * @version 0.4
 * @date 2026-03-13
 */

#ifndef __SAFETY_MANAGER_H__
#define __SAFETY_MANAGER_H__

#include <Arduino.h>

enum class SafetyLockReason {
    NONE,
    HIGH_TEMP,
    LOW_BATTERY,
    OVER_CURRENT,
    WATER_SHORTAGE,
    MANUAL
};

class SafetyManager {
public:
    SafetyManager();
    ~SafetyManager();
    
    // 初始化
    void begin();
    
    // 安全锁定
    void triggerSafetyLock(SafetyLockReason reason);
    void releaseSafetyLock();
    bool isLocked();
    SafetyLockReason getLockReason();
    
    // 温度监控
    void checkTemperature(float temp);
    void setTempWarning(float temp);
    void setTempCutoff(float temp);
    
    // 电压监控
    void checkVoltage(float voltage);
    void setLowVoltage(float voltage);
    void setCutoffVoltage(float voltage);
    
    // 状态
    String getLockReasonString();
    unsigned long getLockTime();
    
private:
    bool _locked;
    SafetyLockReason _lockReason;
    unsigned long _lockTime;
    
    float _tempWarning;
    float _tempCutoff;
    float _lowVoltage;
    float _cutoffVoltage;
    
    void logSafetyEvent(const char* event);
};

#endif // __SAFETY_MANAGER_H__
