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
#include "esp_sleep.h"
#include "esp_timer.h"
#include "driver/gpio.h"

static const char *TAG = "SLEEP_WAKEUP";

// 定义GPIO引脚
#define WAKEUP_GPIO_NUM    GPIO_NUM_0    // 唤醒按钮连接到GPIO0
#define SLEEP_DELAY_MS     10000         // 10秒后进入睡眠

// 全局定时器句柄
static esp_timer_handle_t sleep_timer = NULL;

// 睡眠定时器回调函数
static void sleep_timer_callback(void* arg)
{   
    ESP_LOGI(TAG, "定时器触发，准备进入轻度睡眠模式...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    // 停止定时器
    if (sleep_timer != NULL) {
        esp_timer_stop(sleep_timer);
    }
    
    // 进入轻度睡眠模式
    esp_light_sleep_start();
    
    // 从睡眠中唤醒后继续执行
    ESP_LOGI(TAG, "从轻度睡眠中唤醒！");
    
    // 检查唤醒原因
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    switch(wakeup_reason)
    {
        case ESP_SLEEP_WAKEUP_GPIO:
            ESP_LOGI(TAG, "唤醒原因: GPIO唤醒 (按钮按下)");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            ESP_LOGI(TAG, "唤醒原因: 定时器唤醒");
            break;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            ESP_LOGI(TAG, "唤醒原因: 未定义");
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
            ESP_LOGI(TAG, "唤醒原因: EXT0唤醒 (GPIO0按钮)");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            ESP_LOGI(TAG, "唤醒原因: EXT1唤醒");
            break;
        default:
            ESP_LOGI(TAG, "唤醒原因: 其他原因 (%d)", wakeup_reason);
            break;
    }
    
    // 检查GPIO0当前状态
    int gpio_level = gpio_get_level(WAKEUP_GPIO_NUM);
    ESP_LOGI(TAG, "GPIO%d 当前状态: %s", WAKEUP_GPIO_NUM, gpio_level == 0 ? "低电平(按钮按下)" : "高电平(按钮释放)");
    
    // 重新设置定时器，10秒后再次进入睡眠
    esp_timer_start_once(sleep_timer, SLEEP_DELAY_MS * 1000);
    
    ESP_LOGI(TAG, "已重新设置定时器，%d秒后将再次进入睡眠模式", SLEEP_DELAY_MS / 1000);
}

void app_main(void)
{
    ESP_LOGI(TAG, "系统启动，开始休眠唤醒功能演示");
    
    // 配置GPIO0为输入，启用内部上拉电阻
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << WAKEUP_GPIO_NUM),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "GPIO%d 已配置为输入模式，启用内部上拉电阻", WAKEUP_GPIO_NUM);
    
    // 配置GPIO唤醒源
    // 当GPIO0接地（按钮按下）时唤醒
    esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO_NUM, 0);
    ESP_LOGI(TAG, "已启用GPIO%d作为唤醒源，低电平触发", WAKEUP_GPIO_NUM);
    
    // 创建定时器，10秒后进入睡眠
    esp_timer_create_args_t timer_args = {
        .callback = &sleep_timer_callback,
        .arg = NULL,
        .name = "sleep_timer"
    };
    esp_timer_create(&timer_args, &sleep_timer);
    esp_timer_start_once(sleep_timer, SLEEP_DELAY_MS * 1000);
    
    ESP_LOGI(TAG, "定时器已启动，%d秒后将自动进入轻度睡眠模式", SLEEP_DELAY_MS / 1000);
    ESP_LOGI(TAG, "按下GPIO%d按钮可以立即唤醒系统", WAKEUP_GPIO_NUM);
    
    // 主循环
    while (1) {
        // 检查按钮状态
        // int button_state = gpio_get_level(WAKEUP_GPIO_NUM);
        // ESP_LOGI(TAG, "系统运行中... GPIO%d状态: %s", WAKEUP_GPIO_NUM, 
        //          button_state == 0 ? "低电平(按钮按下)" : "高电平(按钮释放)");
        vTaskDelay(pdMS_TO_TICKS(1000)); // 每1秒打印一次状态
    }
}
