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

static const char *TAG = "MAIN";

void app_main(void)
{
    // 初始化I2C
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C初始化失败");
        return;
    }
    
    ESP_LOGI(TAG, "I2C初始化完成，可以开始使用I2C通信");
    
    // 测试I2C总线
    ret = i2c_test_bus();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C总线测试失败，请检查硬件连接");
        return;
    }
    
    // 扫描I2C设备
    int device_count = i2c_scan_devices();
    
    if (device_count == 0) {
        ESP_LOGW(TAG, "未发现任何I2C设备，请检查:");
        ESP_LOGW(TAG, "1. 设备是否正确连接到GPIO %d(SCL)和GPIO %d(SDA)", 
                 I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO);
        ESP_LOGW(TAG, "2. 设备是否上电");
        ESP_LOGW(TAG, "3. 引脚定义是否正确");
        ESP_LOGW(TAG, "4. 是否需要外部上拉电阻");
    } else {
        ESP_LOGI(TAG, "成功发现 %d 个I2C设备，硬件连接正常", device_count);
    }
    
    // // 示例：与I2C设备通信
    // if (device_count > 0) {
    //     ESP_LOGI(TAG, "开始I2C通信示例...");
        
    //     // 这里可以添加具体的I2C设备通信代码
    //     // 例如：读取传感器数据、配置设备等
        
    //     // 示例：向地址0x10的设备写入数据
    //     uint8_t test_data[] = {0x01, 0x02, 0x03};
    //     ret = i2c_master_write_slave(0x10, test_data, sizeof(test_data));
    //     if (ret == ESP_OK) {
    //         ESP_LOGI(TAG, "成功向设备0x10写入数据");
    //     } else {
    //         ESP_LOGE(TAG, "向设备0x10写入数据失败");
    //     }
        
    //     // 示例：从地址0x10的设备读取数据
    //     uint8_t read_data[3];
    //     ret = i2c_master_read_slave(0x10, read_data, sizeof(read_data));
    //     if (ret == ESP_OK) {
    //         ESP_LOGI(TAG, "成功从设备0x10读取数据: [0x%02X, 0x%02X, 0x%02X]", 
    //                  read_data[0], read_data[1], read_data[2]);
    //     } else {
    //         ESP_LOGE(TAG, "从设备0x10读取数据失败");
    //     }
    // }
}
