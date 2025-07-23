/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "touch_sensor.h"

// 触摸中断回调函数
static void touch_interrupt_handler(bool is_touched)
{
    if (is_touched) {
        ESP_LOGI("MAIN", "🔴 触摸中断: 检测到触摸!");
    } else {
        ESP_LOGI("MAIN", "🟢 触摸中断: 触摸释放!");
    }
}

void app_main(void)
{
    ESP_LOGI("MAIN", "ESP32-S3 触摸传感器演示程序启动");
    
    // 初始化触摸传感器
    if (touch_sensor_init() != ESP_OK) {
        ESP_LOGE("MAIN", "触摸传感器初始化失败，程序退出");
        return;
    }
    
    // 设置触摸中断回调函数
    touch_sensor_set_interrupt_callback(touch_interrupt_handler);
    
    // 启用触摸中断
    touch_sensor_enable_interrupt();
    
    // 启动触摸检测任务
    if (touch_sensor_start_task() != ESP_OK) {
        ESP_LOGE("MAIN", "触摸检测任务启动失败，程序退出");
        return;
    }
    
    ESP_LOGI("MAIN", "触摸检测任务已创建，开始检测GPIO6触摸状态");
    ESP_LOGI("MAIN", "触摸中断已启用，状态变化时会触发回调");
    
    // 主任务循环
    while (1) {
        // 主任务延时
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
