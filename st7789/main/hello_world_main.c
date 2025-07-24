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
#include "pca9557.h"


static const char *TAG = "MAIN";

void app_main(void)
{
    // 初始化I2C主机
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C主机初始化失败: %s", esp_err_to_name(ret));
        return;
    }

    // 初始化PCA9557PW IO扩展芯片
    pca9557_init();

    // 等待一段时间让屏幕稳定
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
