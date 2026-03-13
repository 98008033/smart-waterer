/**
 * @file pins.h
 * @brief 引脚定义
 * @version 0.4
 * @date 2026-03-13
 */

#ifndef __PINS_H__
#define __PINS_H__

#include "driver/gpio.h"

// ==================== ESP32-C3 引脚定义 ====================

// ADC 引脚
#define PIN_ADC1_CH0  GPIO_NUM_0   // GPIO0 - ADC1_CH0
#define PIN_ADC1_CH1  GPIO_NUM_1   // GPIO1 - ADC1_CH1 (用于 PWM)
#define PIN_ADC1_CH2  GPIO_NUM_2   // GPIO2 - ADC1_CH2
#define PIN_ADC1_CH3  GPIO_NUM_3   // GPIO3 - ADC1_CH3

// ==================== 传感器引脚 ====================

// 土壤湿度传感器
#define PIN_SOIL_MOISTURE    PIN_ADC1_CH0
#define PIN_SOIL_MOISTURE_ADC_CHANNEL  ADC1_CHANNEL_0

// 水位传感器
#define PIN_WATER_LEVEL      PIN_ADC1_CH2

// NTC 温度传感器
#define PIN_NTC_TEMP         PIN_ADC1_CH3
#define PIN_NTC_TEMP_ADC_CHANNEL  ADC1_CHANNEL_3

// 电池电压检测（复用 NTC 引脚）
#define PIN_BATTERY_VOLTAGE  PIN_ADC1_CH3
#define PIN_BATTERY_VOLTAGE_ADC_CHANNEL  ADC1_CHANNEL_3

// ==================== 执行器引脚 ====================

// 水泵控制
#define PIN_PUMP_CONTROL     PIN_ADC1_CH1
#define PIN_PUMP_PWM_CHANNEL LEDC_CHANNEL_0

// ==================== 其他引脚 ====================

// 预留引脚
#define PIN_RESERVED_1       GPIO_NUM_4
#define PIN_RESERVED_2       GPIO_NUM_5
#define PIN_RESERVED_3       GPIO_NUM_6
#define PIN_RESERVED_4       GPIO_NUM_7

// ==================== 引脚功能表 ====================
/**
 * | GPIO | 功能           | 类型    | 说明              |
 * |------|----------------|---------|-------------------|
 * | 0    | 土壤湿度       | ADC     | 电容式传感器      |
 * | 1    | 水泵控制       | PWM     | MOSFET 驱动       |
 * | 2    | 水位检测       | DIGITAL | 浮球/探针开关     |
 * | 3    | NTC/电池电压   | ADC     | 复用引脚          |
 * | 4    | 预留           | -       | v0.4 已删除 LED   |
 * | 5    | 预留           | -       | v0.4 已删除蜂鸣器 |
 * | 6-7  | 预留           | -       | 未使用            |
 */

// ==================== 引脚初始化 ====================
static inline void pins_init(void) {
    // ADC 引脚配置
    // 在传感器模块中初始化
    
    // PWM 引脚配置
    // 在水泵控制模块中初始化
    
    // 数字输入引脚配置
    gpio_config_t io_conf = {0};
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_WATER_LEVEL);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
}

#endif // __PINS_H__
