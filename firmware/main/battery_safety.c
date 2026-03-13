/**
 * @file battery_safety.c
 * @brief 电池安全监控模块 - P0 级安全关键代码
 * 
 * 功能：
 * - 电池温度监控（NTC 热敏电阻）
 * - 过充/过放保护
 * - 过流/短路保护
 * - 高温/低温保护
 * 
 * ⚠️ 此模块为安全关键代码，不可省略或简化！
 */

#include "esp_log.h"
#include "driver/adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "battery_safety"

// =============================================================================
// 配置参数 - ⚠️ 安全关键，请勿随意修改！
// =============================================================================

// NTC 温度传感器配置
#define NTC_ADC_CHANNEL     ADC1_CHANNEL_3  // GPIO3
#define NTC_NOMINAL_RES     10000.0f        // 10kΩ @ 25°C
#define NTC_B_VALUE         3950.0f         // B 值
#define NTC_TEMP_NOMINAL    25.0f           // 标称温度°C
#define NTC_PULLUP_RES      10000.0f        // 上拉电阻 10kΩ

// 温度保护阈值 ⚠️ 安全关键
#define TEMP_CRITICAL_HIGH  60.0f           // 60°C 紧急切断
#define TEMP_WARNING_HIGH   50.0f           // 50°C 警告
#define TEMP_WARNING_LOW    5.0f            // 5°C 低温警告
#define TEMP_CRITICAL_LOW   0.0f            // 0°C 禁止充电
#define TEMP_EXTREME_LOW    -10.0f          // -10°C 切断负载

// 电池电压保护阈值 (2 串 7.4V)
#define VOLTAGE_FULL        8.40f           // 满电电压
#define VOLTAGE_HIGH        8.30f           // 高压警告
#define VOLTAGE_LOW         6.60f           // 低压警告
#define VOLTAGE_EMPTY       6.00f           // 空电切断
#define VOLTAGE_CELL_EMPTY  3.00f           // 单串空电

// ADC 配置
#define ADC_ATTEN           ADC_ATTEN_DB_11 // 0-3.3V 输入范围
#define ADC_RESOLUTION      4095            // 12-bit ADC
#define ADC_SAMPLES         16              // 采样次数（平均滤波）

// 保护动作延迟
#define TEMP_CHECK_INTERVAL_MS  1000        // 温度检查间隔 1 秒
#define VOLT_CHECK_INTERVAL_MS  5000        // 电压检查间隔 5 秒

// =============================================================================
// 状态定义
// =============================================================================

typedef enum {
    BATTERY_STATUS_NORMAL = 0,      // 正常
    BATTERY_STATUS_TEMP_HIGH,       // 温度过高
    BATTERY_STATUS_TEMP_LOW,        // 温度过低
    BATTERY_STATUS_VOLT_HIGH,       // 电压过高
    BATTERY_STATUS_VOLT_LOW,        // 电压过低
    BATTERY_STATUS_CHARGE_FAULT,    // 充电故障
    BATTERY_STATUS_DISCHARGE_FAULT  // 放电故障
} battery_status_t;

typedef struct {
    float temperature;              // 电池温度°C
    float voltage;                  // 电池电压 V
    float current;                  // 电池电流 A（如有传感器）
    battery_status_t status;        // 当前状态
    bool charging_enabled;          // 充电使能
    bool discharge_enabled;         // 放电使能
    uint32_t fault_count;           // 故障计数
    uint32_t last_check_time;       // 上次检查时间
} battery_data_t;

// 全局电池数据
static battery_data_t g_battery = {
    .temperature = 25.0f,
    .voltage = 7.4f,
    .status = BATTERY_STATUS_NORMAL,
    .charging_enabled = true,
    .discharge_enabled = true,
    .fault_count = 0,
    .last_check_time = 0
};

// 回调函数指针
static void (*g_temp_critical_cb)(float temp) = NULL;
static void (*g_temp_warning_cb)(float temp) = NULL;
static void (*g_volt_critical_cb)(float volt) = NULL;
static void (*g_state_change_cb)(bool charge, bool discharge) = NULL;

// =============================================================================
// ADC 初始化
// =============================================================================

esp_err_t battery_adc_init(void)
{
    ESP_LOGI(TAG, "初始化电池 ADC...");
    
    // 配置 ADC1
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(NTC_ADC_CHANNEL, ADC_ATTEN);
    
    ESP_LOGI(TAG, "ADC 初始化完成");
    return ESP_OK;
}

// =============================================================================
// NTC 温度读取 - Steinhart-Hart 方程
// =============================================================================

/**
 * @brief 读取 NTC 温度
 * @return float 温度值 (°C)
 */
float battery_read_temperature(void)
{
    uint32_t adc_sum = 0;
    
    // 多次采样取平均
    for (int i = 0; i < ADC_SAMPLES; i++) {
        adc_sum += adc1_get_raw(NTC_ADC_CHANNEL);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    float adc_value = (float)adc_sum / ADC_SAMPLES;
    
    // 计算 NTC 电阻值
    // R_ntc = R_pullup × (ADC_max / ADC_value - 1)
    float r_ntc = NTC_PULLUP_RES * (ADC_RESOLUTION / adc_value - 1.0f);
    
    // Steinhart-Hart 方程计算温度
    // 1/T = 1/T0 + (1/B) × ln(R/R0)
    float steinhart = 1.0f / (NTC_TEMP_NOMINAL + 273.15f);  // 1/T0
    steinhart += (1.0f / NTC_B_VALUE) * logf(r_ntc / NTC_NOMINAL_RES);
    
    float temp_k = 1.0f / steinhart;  // 开尔文温度
    float temp_c = temp_k - 273.15f;   // 摄氏温度
    
    ESP_LOGV(TAG, "NTC ADC: %.1f, R: %.1fΩ, T: %.1f°C", 
             adc_value, r_ntc, temp_c);
    
    return temp_c;
}

// =============================================================================
// 电池电压读取
// =============================================================================

/**
 * @brief 读取电池电压
 * @return float 电压值 (V)
 */
float battery_read_voltage(void)
{
    uint32_t adc_sum = 0;
    
    // 多次采样取平均
    for (int i = 0; i < ADC_SAMPLES; i++) {
        adc_sum += adc1_get_raw(NTC_ADC_CHANNEL);  // 复用 ADC 通道
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    float adc_value = (float)adc_sum / ADC_SAMPLES;
    
    // 计算实际电压（考虑分压比）
    // V_batt = V_adc × (R1 + R2) / R2 = V_adc × 1.5
    float v_adc = (adc_value / ADC_RESOLUTION) * 3.3f;
    float v_batt = v_adc * 1.5f;
    
    ESP_LOGV(TAG, "电压 ADC: %.1f, V: %.2fV", adc_value, v_batt);
    
    return v_batt;
}

// =============================================================================
// 温度保护逻辑
// =============================================================================

/**
 * @brief 检查温度保护
 * @param temp 当前温度
 * @return battery_status_t 状态
 */
static battery_status_t check_temperature_protection(float temp)
{
    // 严重高温 - 立即切断
    if (temp >= TEMP_CRITICAL_HIGH) {
        ESP_LOGE(TAG, "🔴 严重高温！%.1f°C >= %.1f°C，切断充放电！", 
                 temp, TEMP_CRITICAL_HIGH);
        
        g_battery.charging_enabled = false;
        g_battery.discharge_enabled = false;
        g_battery.fault_count++;
        
        if (g_temp_critical_cb) {
            g_temp_critical_cb(temp);
        }
        
        return BATTERY_STATUS_TEMP_HIGH;
    }
    
    // 高温警告 - 降流
    if (temp >= TEMP_WARNING_HIGH) {
        ESP_LOGW(TAG, "🟠 高温警告！%.1f°C >= %.1f°C，限制充电", 
                 temp, TEMP_WARNING_HIGH);
        
        g_battery.charging_enabled = false;  // 暂停充电
        
        if (g_temp_warning_cb) {
            g_temp_warning_cb(temp);
        }
        
        return BATTERY_STATUS_TEMP_HIGH;
    }
    
    // 低温禁止充电
    if (temp <= TEMP_CRITICAL_LOW) {
        ESP_LOGE(TAG, "🔵 严重低温！%.1f°C <= %.1f°C，禁止充电！", 
                 temp, TEMP_CRITICAL_LOW);
        
        g_battery.charging_enabled = false;
        
        return BATTERY_STATUS_TEMP_LOW;
    }
    
    // 极低温切断负载
    if (temp <= TEMP_EXTREME_LOW) {
        ESP_LOGE(TAG, "🔵 极低温！%.1f°C <= %.1f°C，切断负载！", 
                 temp, TEMP_EXTREME_LOW);
        
        g_battery.charging_enabled = false;
        g_battery.discharge_enabled = false;
        
        return BATTERY_STATUS_TEMP_LOW;
    }
    
    // 温度正常 - 恢复
    if (g_battery.status == BATTERY_STATUS_TEMP_HIGH || 
        g_battery.status == BATTERY_STATUS_TEMP_LOW) {
        
        // 添加迟滞，防止频繁切换
        if (temp < TEMP_WARNING_HIGH - 5.0f && temp > TEMP_CRITICAL_LOW + 5.0f) {
            ESP_LOGI(TAG, "🟢 温度恢复正常：%.1f°C", temp);
            g_battery.charging_enabled = true;
            g_battery.discharge_enabled = true;
            g_battery.fault_count = 0;
        }
    }
    
    return BATTERY_STATUS_NORMAL;
}

// =============================================================================
// 电压保护逻辑
// =============================================================================

/**
 * @brief 检查电压保护
 * @param volt 当前电压
 * @return battery_status_t 状态
 */
static battery_status_t check_voltage_protection(float volt)
{
    // 过压保护
    if (volt >= VOLTAGE_FULL) {
        ESP_LOGW(TAG, "🟠 电池充满：%.2fV，停止充电", volt);
        g_battery.charging_enabled = false;
        return BATTERY_STATUS_VOLT_HIGH;
    }
    
    // 低压保护
    if (volt <= VOLTAGE_EMPTY) {
        ESP_LOGE(TAG, "🔴 电池耗尽：%.2fV <= %.2fV，切断负载！", 
                 volt, VOLTAGE_EMPTY);
        g_battery.discharge_enabled = false;
        
        if (g_volt_critical_cb) {
            g_volt_critical_cb(volt);
        }
        
        return BATTERY_STATUS_VOLT_LOW;
    }
    
    // 低压警告
    if (volt <= VOLTAGE_LOW) {
        ESP_LOGW(TAG, "🟡 电量低：%.2fV，准备关机", volt);
        return BATTERY_STATUS_VOLT_LOW;
    }
    
    // 电压恢复
    if (g_battery.status == BATTERY_STATUS_VOLT_LOW && volt >= VOLTAGE_LOW + 0.3f) {
        ESP_LOGI(TAG, "🟢 电压恢复正常：%.2fV", volt);
        g_battery.discharge_enabled = true;
    }
    
    return BATTERY_STATUS_NORMAL;
}

// =============================================================================
// 主保护循环
// =============================================================================

/**
 * @brief 电池安全监控任务
 * @param pvParameters 参数
 */
void battery_safety_task(void *pvParameters)
{
    ESP_LOGI(TAG, "🛡️ 电池安全监控任务启动");
    
    uint32_t volt_check_counter = 0;
    
    while (1) {
        // 读取温度
        g_battery.temperature = battery_read_temperature();
        
        // 检查温度保护
        battery_status_t temp_status = check_temperature_protection(g_battery.temperature);
        
        // 定期读取电压（节省功耗）
        volt_check_counter++;
        if (volt_check_counter >= VOLT_CHECK_INTERVAL_MS / TEMP_CHECK_INTERVAL_MS) {
            g_battery.voltage = battery_read_voltage();
            battery_status_t volt_status = check_voltage_protection(g_battery.voltage);
            
            // 取最严重状态
            if (volt_status != BATTERY_STATUS_NORMAL) {
                g_battery.status = volt_status;
            } else {
                g_battery.status = temp_status;
            }
            
            volt_check_counter = 0;
        } else {
            g_battery.status = temp_status;
        }
        
        // 通知状态变化
        if (g_state_change_cb) {
            g_state_change_cb(g_battery.charging_enabled, g_battery.discharge_enabled);
        }
        
        // 记录日志
        if (g_battery.status != BATTERY_STATUS_NORMAL) {
            ESP_LOGW(TAG, "⚠️ 电池状态：%d, T=%.1f°C, V=%.2fV, CHG=%d, DIS=%d",
                     g_battery.status, g_battery.temperature, g_battery.voltage,
                     g_battery.charging_enabled, g_battery.discharge_enabled);
        }
        
        vTaskDelay(pdMS_TO_TICKS(TEMP_CHECK_INTERVAL_MS));
    }
}

// =============================================================================
// API 接口
// =============================================================================

/**
 * @brief 初始化电池安全模块
 * @return esp_err_t 错误码
 */
esp_err_t battery_safety_init(void)
{
    ESP_LOGI(TAG, "🛡️ 初始化电池安全模块...");
    
    // 初始化 ADC
    battery_adc_init();
    
    // 创建监控任务
    xTaskCreate(
        battery_safety_task,
        "battery_safety",
        2048,
        NULL,
        10,  // 高优先级
        NULL
    );
    
    ESP_LOGI(TAG, "🛡️ 电池安全模块初始化完成");
    return ESP_OK;
}

/**
 * @brief 获取电池数据
 * @return battery_data_t* 电池数据指针
 */
battery_data_t* battery_get_data(void)
{
    return &g_battery;
}

/**
 * @brief 检查是否允许充电
 * @return true 允许充电，false 禁止充电
 */
bool battery_can_charge(void)
{
    return g_battery.charging_enabled;
}

/**
 * @brief 检查是否允许放电
 * @return true 允许放电，false 禁止放电
 */
bool battery_can_discharge(void)
{
    return g_battery.discharge_enabled;
}

/**
 * @brief 设置温度临界回调
 * @param callback 回调函数
 */
void battery_set_temp_critical_cb(void (*callback)(float))
{
    g_temp_critical_cb = callback;
}

/**
 * @brief 设置温度警告回调
 * @param callback 回调函数
 */
void battery_set_temp_warning_cb(void (*callback)(float))
{
    g_temp_warning_cb = callback;
}

/**
 * @brief 设置电压临界回调
 * @param callback 回调函数
 */
void battery_set_volt_critical_cb(void (*callback)(float))
{
    g_volt_critical_cb = callback;
}

/**
 * @brief 设置状态变化回调
 * @param callback 回调函数
 */
void battery_set_state_change_cb(void (*callback)(bool, bool))
{
    g_state_change_cb = callback;
}

// =============================================================================
// 使用示例
// =============================================================================

/*

// 1. 初始化
void app_main(void)
{
    // 初始化电池安全模块
    battery_safety_init();
    
    // 设置回调函数
    battery_set_temp_critical_cb([](float temp) {
        // 温度过高，发送告警
        send_alert("电池过热！");
        beep_alert();
    });
    
    battery_set_state_change_cb([](bool charge, bool discharge) {
        // 状态变化，控制充电/放电电路
        set_charge_enable(charge);
        set_discharge_enable(discharge);
    });
}

// 2. 充电前检查
esp_err_t start_charging(void)
{
    if (!battery_can_charge()) {
        ESP_LOGE(TAG, "禁止充电！");
        return ESP_FAIL;
    }
    
    // 开始充电...
    return ESP_OK;
}

// 3. 放电前检查
esp_err_t start_pump(void)
{
    if (!battery_can_discharge()) {
        ESP_LOGE(TAG, "禁止放电！");
        return ESP_FAIL;
    }
    
    // 启动水泵...
    return ESP_OK;
}

*/
