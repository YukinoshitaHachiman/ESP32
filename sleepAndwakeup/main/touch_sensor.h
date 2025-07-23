#ifndef TOUCH_SENSOR_H
#define TOUCH_SENSOR_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 触摸传感器通道定义 (GPIO6 对应 T6)
#define TOUCH_PAD_NUM TOUCH_PAD_NUM6

// 自适应阈值配置
#define TOUCH_BASELINE_MULTIPLIER 1.3f  // 基准值倍数
#define TOUCH_THRESHOLD_OFFSET    50    // 阈值偏移量
#define TOUCH_CALIBRATION_SAMPLES 10    // 校准采样次数
#define TOUCH_CALIBRATION_DELAY   100   // 校准延时(ms)

// 任务优先级
#define TOUCH_TASK_PRIORITY      5
#define TOUCH_TASK_STACK_SIZE    4096

// 中断配置
#define TOUCH_INTERRUPT_PRIORITY 5



/**
 * @brief 初始化触摸传感器
 * @return ESP_OK 成功, ESP_FAIL 失败
 */
esp_err_t touch_sensor_init(void);

/**
 * @brief 启动触摸检测任务
 * @return ESP_OK 成功, ESP_FAIL 失败
 */
esp_err_t touch_sensor_start_task(void);

/**
 * @brief 获取当前触摸值
 * @return 当前触摸值
 */
uint32_t touch_sensor_get_value(void);

/**
 * @brief 获取当前触摸状态
 * @return true 已触摸, false 未触摸
 */
bool touch_sensor_is_touched(void);

/**
 * @brief 重新校准触摸传感器基准值
 * @return ESP_OK 成功, ESP_FAIL 失败
 */
esp_err_t touch_sensor_recalibrate(void);

/**
 * @brief 获取当前基准值
 * @return 当前基准值
 */
uint32_t touch_sensor_get_baseline(void);

/**
 * @brief 获取当前阈值
 * @return 当前阈值
 */
uint32_t touch_sensor_get_threshold(void);

/**
 * @brief 触摸中断回调函数类型
 */
typedef void (*touch_interrupt_callback_t)(bool is_touched);

/**
 * @brief 设置触摸中断回调函数
 * @param callback 回调函数指针
 */
void touch_sensor_set_interrupt_callback(touch_interrupt_callback_t callback);

/**
 * @brief 启用触摸中断
 * @return ESP_OK 成功, ESP_FAIL 失败
 */
esp_err_t touch_sensor_enable_interrupt(void);

/**
 * @brief 禁用触摸中断
 * @return ESP_OK 成功, ESP_FAIL 失败
 */
esp_err_t touch_sensor_disable_interrupt(void);



#endif // TOUCH_SENSOR_H 