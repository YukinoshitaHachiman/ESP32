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
#include "i2c_master.h"
#include "xl9555.h"


static const char *TAG = "MAIN";

// 按钮监控任务
static void button_monitor_task(void *pvParameters)
{
    static const char *TASK_TAG = "BUTTON_TASK";
    bool last_key_states[4] = {false, false, false, false};
    uint32_t press_time[4] = {0, 0, 0, 0};  // 记录按下时间
    bool long_press_detected[4] = {false, false, false, false};  // 长按检测标志
    
    ESP_LOGI(TASK_TAG, "按钮监控任务启动");
    
    while (1) {
        bool current_key_states[4];
        esp_err_t ret = xl9555_keys_read_all(current_key_states);
        
        if (ret == ESP_OK) {
            uint32_t current_time = xTaskGetTickCount();
            
            for (uint32_t i = 0; i < 4; i++) {
                // 检测按钮状态变化
                if (current_key_states[i] != last_key_states[i]) {
                    if (current_key_states[i]) {
                        // 按钮按下
                        press_time[i] = current_time;
                        long_press_detected[i] = false;
                        ESP_LOGI(TASK_TAG, "KEY%" PRIu32 " 按下", i);
                    } else {
                        // 按钮释放
                        uint32_t press_duration = (current_time - press_time[i]) * portTICK_PERIOD_MS;
                        if (press_duration < 1000) {
                            ESP_LOGI(TASK_TAG, "KEY%" PRIu32 " 短按释放 (持续时间: %" PRIu32 "ms)", i, press_duration);
                        } else {
                            ESP_LOGI(TASK_TAG, "KEY%" PRIu32 " 长按释放 (持续时间: %" PRIu32 "ms)", i, press_duration);
                        }
                    }
                    last_key_states[i] = current_key_states[i];
                }
                
                // 检测长按 (1秒)
                if (current_key_states[i] && !long_press_detected[i]) {
                    uint32_t press_duration = (current_time - press_time[i]) * portTICK_PERIOD_MS;
                    if (press_duration >= 1000) {
                        ESP_LOGI(TASK_TAG, "KEY%" PRIu32 " 长按检测 (持续时间: %" PRIu32 "ms)", i, press_duration);
                        long_press_detected[i] = true;
                    }
                }
            }
            
            // 检测按钮组合
            int pressed_count = 0;
            for (uint32_t i = 0; i < 4; i++) {
                if (current_key_states[i]) pressed_count++;
            }
            
            if (pressed_count >= 2) {
                ESP_LOGI(TASK_TAG, "检测到多按钮按下: %d个按钮", pressed_count);
                for (uint32_t i = 0; i < 4; i++) {
                    if (current_key_states[i]) {
                        ESP_LOGI(TASK_TAG, "  - KEY%" PRIu32 " 按下", i);
                    }
                }
            }
        } else {
            ESP_LOGE(TASK_TAG, "读取按钮状态失败: %s", esp_err_to_name(ret));
        }
        
        // 任务延时 (20Hz采样率)
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // 初始化I2C
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C初始化失败");
        return;
    }
    
    ESP_LOGI(TAG, "I2C初始化完成，可以开始使用I2C通信");
    
    // 扫描I2C设备
    int device_count = i2c_scan_devices();
    
    if (device_count == 0) {
        ESP_LOGW(TAG, "未发现任何I2C设备，请检查:");
        ESP_LOGW(TAG, "1. 设备是否正确连接到GPIO %d(SCL)和GPIO %d(SDA)", 
                 I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO);
        ESP_LOGW(TAG, "2. 设备是否上电");
        ESP_LOGW(TAG, "3. 引脚定义是否正确");
        ESP_LOGW(TAG, "4. 是否需要外部上拉电阻");
        return;
    } else {
        ESP_LOGI(TAG, "成功发现 %d 个I2C设备，硬件连接正常", device_count);
    }
    
    // 初始化XL9555 IO扩展芯片
    ESP_LOGI(TAG, "开始初始化XL9555 IO扩展芯片...");
    ret = xl9555_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "XL9555初始化失败");
        return;
    }
    
    // 初始化按钮
    ESP_LOGI(TAG, "初始化按钮...");
    ret = xl9555_keys_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "按钮初始化失败");
        return;
    }
    
    
    // 创建按钮检测任务
    ESP_LOGI(TAG, "创建按钮检测任务...");
    xTaskCreate(button_monitor_task, "button_monitor", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "程序初始化完成，按钮检测任务已启动!");
}
