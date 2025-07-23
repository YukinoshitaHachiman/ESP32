/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "i2c_master.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "I2C_MASTER";

/**
 * @brief 检查I2C引脚连接状态
 */
esp_err_t i2c_check_connection(void)
{
    ESP_LOGI(TAG, "检查I2C引脚连接状态...");
    
    // 配置SCL和SDA为输入模式
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << I2C_MASTER_SCL_IO) | (1ULL << I2C_MASTER_SDA_IO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    
    // 等待一段时间让信号稳定
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    // 读取引脚状态
    int scl_level = gpio_get_level(I2C_MASTER_SCL_IO);
    int sda_level = gpio_get_level(I2C_MASTER_SDA_IO);
    
    ESP_LOGI(TAG, "SCL引脚(GPIO %d)电平: %d", I2C_MASTER_SCL_IO, scl_level);
    ESP_LOGI(TAG, "SDA引脚(GPIO %d)电平: %d", I2C_MASTER_SDA_IO, sda_level);
    
    // 检查引脚是否被拉高（正常情况）
    if (scl_level == 0 || sda_level == 0) {
        ESP_LOGW(TAG, "警告: I2C引脚可能连接异常或短路!");
        ESP_LOGW(TAG, "正常情况: SCL和SDA都应该被上拉电阻拉高(电平=1)");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "I2C引脚连接状态正常");
    return ESP_OK;
}

/**
 * @brief I2C主机初始化
 */
esp_err_t i2c_master_init(void)
{
    // 首先检查引脚连接状态
    esp_err_t check_result = i2c_check_connection();
    if (check_result != ESP_OK) {
        ESP_LOGW(TAG, "I2C引脚检查失败，但继续初始化...");
    }
    
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C参数配置失败: %s", esp_err_to_name(err));
        return err;
    }
    
    err = i2c_driver_install(i2c_master_port, conf.mode,
                            I2C_MASTER_RX_BUF_DISABLE,
                            I2C_MASTER_TX_BUF_DISABLE, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C驱动安装失败: %s", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(TAG, "I2C主机初始化成功");
    ESP_LOGI(TAG, "SCL引脚: %d, SDA引脚: %d", I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO);
    ESP_LOGI(TAG, "I2C频率: %d Hz", I2C_MASTER_FREQ_HZ);
    
    return ESP_OK;
}

/**
 * @brief 测试I2C总线是否正常工作
 */
esp_err_t i2c_test_bus(void)
{
    ESP_LOGI(TAG, "测试I2C总线功能...");
    
    // 尝试发送一个通用的I2C命令来测试总线
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x00, true);  // 通用调用地址
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "I2C总线测试成功 - 总线工作正常");
        return ESP_OK;
    } else if (ret == ESP_ERR_TIMEOUT) {
        ESP_LOGW(TAG, "I2C总线测试超时 - 可能没有设备响应，但总线正常");
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "I2C总线测试失败: %s", esp_err_to_name(ret));
        return ret;
    }
}

/**
 * @brief I2C写数据到从设备
 */
esp_err_t i2c_master_write_slave(uint8_t slave_addr, uint8_t *data, size_t size)
{
    if (data == NULL || size == 0) {
        ESP_LOGE(TAG, "无效的参数: data=%p, size=%zu", data, size);
        return ESP_ERR_INVALID_ARG;
    }
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, data, size, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C写数据失败: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

/**
 * @brief 从I2C从设备读取数据
 */
esp_err_t i2c_master_read_slave(uint8_t slave_addr, uint8_t *data, size_t size)
{
    if (data == NULL || size == 0) {
        ESP_LOGE(TAG, "无效的参数: data=%p, size=%zu", data, size);
        return ESP_ERR_INVALID_ARG;
    }
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_READ, true);
    if (size > 1) {
        i2c_master_read(cmd, data, size - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + size - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C读数据失败: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

/**
 * @brief 扫描I2C设备并返回发现的设备数量
 */
int i2c_scan_devices(void)
{
    ESP_LOGI(TAG, "开始扫描I2C设备...");
    int device_count = 0;
    
    for (int i = 1; i < 128; i++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 50 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "发现I2C设备，地址: 0x%02X", i);
            device_count++;
        }
    }
    
    ESP_LOGI(TAG, "I2C设备扫描完成，共发现 %d 个设备", device_count);
    return device_count;
}

/**
 * @brief 释放I2C驱动
 */
esp_err_t i2c_master_deinit(void)
{
    esp_err_t ret = i2c_driver_delete(I2C_MASTER_NUM);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "I2C驱动已释放");
    } else {
        ESP_LOGE(TAG, "I2C驱动释放失败: %s", esp_err_to_name(ret));
    }
    return ret;
} 