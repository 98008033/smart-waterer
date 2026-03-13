/**
 * @file pump_controller.h
 * @brief 水泵控制模块
 * @version 0.4
 * @date 2026-03-13
 */

#ifndef __PUMP_CONTROLLER_H__
#define __PUMP_CONTROLLER_H__

#include <Arduino.h>

class PumpController {
public:
    PumpController();
    ~PumpController();
    
    // 初始化
    void begin();
    
    // 控制
    void start(uint16_t duration_ms);
    void stop();
    bool isRunning();
    
    // 设置
    void setDefaultDuration(uint16_t ms);
    void setMaxDuration(uint16_t ms);
    void setCooldownTime(uint16_t ms);
    
    // 安全
    void setSafetyLock(bool locked);
    bool isSafetyLocked();
    
    // 统计
    uint16_t getTotalRunningTime();
    uint16_t getWateringCount();
    
private:
    uint16_t _defaultDuration;
    uint16_t _maxDuration;
    uint16_t _cooldownTime;
    bool _running;
    bool _safetyLocked;
    unsigned long _startTime;
    unsigned long _lastWateringTime;
    uint16_t _totalRunningTime;
    uint16_t _wateringCount;
    
    void update();
};

#endif // __PUMP_CONTROLLER_H__
