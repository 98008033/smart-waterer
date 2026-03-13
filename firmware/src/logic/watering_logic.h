/**
 * @file watering_logic.h
 * @brief 自动浇水逻辑模块
 * @version 0.4
 * @date 2026-03-13
 */

#ifndef __WATERING_LOGIC_H__
#define __WATERING_LOGIC_H__

#include <Arduino.h>

class WateringLogic {
public:
    WateringLogic();
    ~WateringLogic();
    
    // 初始化
    void begin();
    
    // 模式设置
    enum Mode {
        MODE_MANUAL,      // 手动模式
        MODE_AUTO_TIME,   // 定时模式
        MODE_AUTO_SOIL    // 土壤湿度触发
    };
    
    void setMode(Mode mode);
    Mode getMode();
    
    // 定时设置
    void setInterval(unsigned long ms);
    unsigned long getInterval();
    
    // 土壤湿度设置
    void setTargetMoisture(int percent);
    int getTargetMoisture();
    
    // 手动浇水
    void manualWater(uint16_t duration_ms);
    
    // 自动检查
    void checkAndWater();
    
    // 状态
    bool isWatering();
    unsigned long getLastWateringTime();
    uint16_t getWateringCount();
    
private:
    Mode _mode;
    unsigned long _interval;
    int _targetMoisture;
    unsigned long _lastWateringTime;
    uint16_t _wateringCount;
    bool _watering;
    
    bool shouldWaterByTime();
    bool shouldWaterBySoil();
    void executeWatering(uint16_t duration_ms);
};

#endif // __WATERING_LOGIC_H__
