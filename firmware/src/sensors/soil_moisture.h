/**
 * @file soil_moisture.h
 * @brief 土壤湿度传感器模块
 * @version 0.4
 * @date 2026-03-13
 */

#ifndef __SOIL_MOISTURE_H__
#define __SOIL_MOISTURE_H__

#include <Arduino.h>

class SoilMoistureSensor {
public:
    SoilMoistureSensor();
    ~SoilMoistureSensor();
    
    // 初始化
    void begin();
    
    // 读取数据
    int readADC();
    int readPercent();
    float readFiltered();
    
    // 校准
    void calibrate(int dryValue, int wetValue);
    void loadCalibration();
    void saveCalibration();
    
    // 状态
    bool isAvailable();
    
private:
    int _adcMin;
    int _adcMax;
    int _lastReading;
    unsigned long _lastReadTime;
    
    int filterReading();
};

#endif // __SOIL_MOISTURE_H__
