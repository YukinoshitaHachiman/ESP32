#include "touch_sensor.h"
#include "driver/touch_pad.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include <stdio.h>

static const char *TAG = "TOUCH_SENSOR";

// 全局变量
static uint32_t touch_value = 0;
static bool is_touched = false;
static TaskHandle_t touch_task_handle = NULL;
static uint32_t touch_baseline = 0;    // 基准值
static uint32_t touch_threshold = 0;   // 动态阈值
static bool is_calibrated = false;     // 是否已校准
static touch_interrupt_callback_t interrupt_callback = NULL;  // 中断回调函数
static bool interrupt_enabled = false;  // 中断是否启用

/**
 * @brief 触摸检测任务
 * @param pvParameters 任务参数
 */
static void touch_detection_task(void *pvParameters)
{
    uint32_t touch_value_raw = 0;
    
    ESP_LOGI(TAG, "触摸检测任务启动");
    
    while (1) {
        // 读取原始触摸值
        if (touch_pad_read_raw_data(TOUCH_PAD_NUM, &touch_value_raw) == ESP_OK) {
            touch_value = touch_value_raw;
            
            // 判断是否触摸 (使用自适应阈值)
            if (is_calibrated) {
                bool previous_touched = is_touched;
                
                if (touch_value > touch_threshold) {
                    is_touched = true;
                    ESP_LOGI(TAG, "触摸检测: 原始值=%" PRIu32 ", 基准值=%" PRIu32 ", 阈值=%" PRIu32 ", 状态=已触摸", 
                            touch_value_raw, touch_baseline, touch_threshold);
                } else {
                    is_touched = false;
                    //ESP_LOGI(TAG, "触摸检测: 原始值=%" PRIu32 ", 基准值=%" PRIu32 ", 阈值=%" PRIu32 ", 状态=未触摸", 
                            //touch_value_raw, touch_baseline, touch_threshold);
                }
                
                // 检查状态变化并触发中断回调
                if (interrupt_enabled && interrupt_callback && (previous_touched != is_touched)) {
                    ESP_LOGI(TAG, "触摸状态变化，触发中断回调: %s -> %s", 
                            previous_touched ? "已触摸" : "未触摸", 
                            is_touched ? "已触摸" : "未触摸");
                    interrupt_callback(is_touched);
                }
            } else {
                ESP_LOGW(TAG, "触摸检测: 原始值=%" PRIu32 ", 状态=未校准", touch_value_raw);
            }
        } else {
            ESP_LOGE(TAG, "读取原始触摸值失败");
        }
        
        // 延时100ms (0.1秒)
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

esp_err_t touch_sensor_init(void)
{
    esp_err_t ret = ESP_OK;
    
    ESP_LOGI(TAG, "初始化触摸传感器...");
    
    // 初始化触摸传感器驱动
    ret = touch_pad_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "触摸传感器初始化失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 配置触摸传感器GPIO (GPIO6)
    ret = touch_pad_config(TOUCH_PAD_NUM);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "配置触摸传感器GPIO失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 启动触摸传感器FSM
    ret = touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置FSM模式失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 启动触摸传感器
    ret = touch_pad_fsm_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "启动触摸传感器FSM失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 等待触摸传感器稳定
    vTaskDelay(pdMS_TO_TICKS(100));
    
    ESP_LOGI(TAG, "触摸传感器初始化成功");
    
    // 等待系统稳定后进行校准
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 执行初始校准
    ret = touch_sensor_recalibrate();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "初始校准失败");
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t touch_sensor_start_task(void)
{
    if (touch_task_handle != NULL) {
        ESP_LOGW(TAG, "触摸检测任务已存在");
        return ESP_OK;
    }
    
    // 创建触摸检测任务
    BaseType_t ret = xTaskCreate(touch_detection_task, 
                                 "touch_detection", 
                                 TOUCH_TASK_STACK_SIZE, 
                                 NULL, 
                                 TOUCH_TASK_PRIORITY, 
                                 &touch_task_handle);
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "创建触摸检测任务失败");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "触摸检测任务创建成功");
    return ESP_OK;
}

uint32_t touch_sensor_get_value(void)
{
    return touch_value;
}

bool touch_sensor_is_touched(void)
{
    return is_touched;
}

esp_err_t touch_sensor_recalibrate(void)
{
    esp_err_t ret = ESP_OK;
    uint32_t touch_value_raw = 0;
    uint32_t sum = 0;
    
    ESP_LOGI(TAG, "开始校准触摸传感器...");
    
    // 多次采样获取基准值
    for (int i = 0; i < TOUCH_CALIBRATION_SAMPLES; i++) {
        ret = touch_pad_read_raw_data(TOUCH_PAD_NUM, &touch_value_raw);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "校准采样失败: %s", esp_err_to_name(ret));
            return ret;
        }
        
        sum += touch_value_raw;
        ESP_LOGI(TAG, "校准采样 %d: %" PRIu32, i + 1, touch_value_raw);
        
        // 采样间隔
        vTaskDelay(pdMS_TO_TICKS(TOUCH_CALIBRATION_DELAY));
    }
    
    // 计算平均基准值
    touch_baseline = sum / TOUCH_CALIBRATION_SAMPLES;
    
    // 计算动态阈值 (基准值 * 倍数)
    touch_threshold = (uint32_t)(touch_baseline * TOUCH_BASELINE_MULTIPLIER);
    
    is_calibrated = true;
    
    ESP_LOGI(TAG, "校准完成 - 基准值: %" PRIu32 ", 阈值: %" PRIu32, touch_baseline, touch_threshold);
    
    return ESP_OK;
}

uint32_t touch_sensor_get_baseline(void)
{
    return touch_baseline;
}

uint32_t touch_sensor_get_threshold(void)
{
    return touch_threshold;
}

void touch_sensor_set_interrupt_callback(touch_interrupt_callback_t callback)
{
    interrupt_callback = callback;
    ESP_LOGI(TAG, "触摸中断回调函数已设置");
}

esp_err_t touch_sensor_enable_interrupt(void)
{
    interrupt_enabled = true;
    ESP_LOGI(TAG, "触摸中断已启用");
    return ESP_OK;
}

esp_err_t touch_sensor_disable_interrupt(void)
{
    interrupt_enabled = false;
    ESP_LOGI(TAG, "触摸中断已禁用");
    return ESP_OK;
}

 