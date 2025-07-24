/*
 * PCA9557PW IO扩展芯片驱动实现
 * 8位I2C IO扩展器
 */

#include "pca9557.h"
#include "i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "PCA9557";

// 当前配置状态
static uint8_t current_config = 0xFF;  // 默认所有IO为输入
static uint8_t current_output = 0x00;  // 默认所有输出为低电平

/**
 * @brief 写入寄存器
 */
static esp_err_t pca9557_write_register(uint8_t reg, uint8_t data)
{
    uint8_t write_data[2];
    write_data[0] = reg;
    write_data[1] = data;
    
    esp_err_t ret = i2c_master_write_slave(PCA9557_I2C_ADDR, write_data, 2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "写入寄存器0x%02X失败: %s", reg, esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

/**
 * @brief 读取寄存器
 */
static esp_err_t pca9557_read_register(uint8_t reg, uint8_t *data)
{
    // 先写入寄存器地址
    esp_err_t ret = i2c_master_write_slave(PCA9557_I2C_ADDR, &reg, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "写入寄存器地址0x%02X失败: %s", reg, esp_err_to_name(ret));
        return ret;
    }
    
    // 然后读取数据
    ret = i2c_master_read_slave(PCA9557_I2C_ADDR, data, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取寄存器0x%02X失败: %s", reg, esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

/**
 * @brief PCA9557PW初始化
 */
esp_err_t pca9557_init(void)
{
    ESP_LOGI(TAG, "初始化PCA9557PW IO扩展芯片...");
    
    // 检查设备是否存在
    esp_err_t ret = pca9557_check_device();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCA9557PW设备检查失败");
        return ret;
    }
    
    // 初始化配置：所有IO设为输入模式
    ret = pca9557_config_all_inputs();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCA9557PW初始化配置失败");
        return ret;
    }
    
    ESP_LOGI(TAG, "PCA9557PW初始化成功");
    ESP_LOGI(TAG, "I2C地址: 0x%02X", PCA9557_I2C_ADDR);
    ESP_LOGI(TAG, "初始配置: 所有IO设为输入模式");
    
    return ESP_OK;
}

/**
 * @brief 检查PCA9557PW是否存在
 */
esp_err_t pca9557_check_device(void)
{
    uint8_t data;
    esp_err_t ret = pca9557_read_register(PCA9557_REG_INPUT, &data);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "PCA9557PW设备检测成功");
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "PCA9557PW设备检测失败: %s", esp_err_to_name(ret));
        return ESP_ERR_NOT_FOUND;
    }
}

/**
 * @brief 设置IO方向
 */
esp_err_t pca9557_set_io_direction(uint8_t io_mask, uint8_t direction)
{
    uint8_t new_config = current_config;
    
    if (direction == PCA9557_IO_INPUT) {
        // 设置为输入模式 (1)
        new_config |= io_mask;
    } else {
        // 设置为输出模式 (0)
        new_config &= ~io_mask;
    }
    
    esp_err_t ret = pca9557_write_register(PCA9557_REG_CONFIG, new_config);
    if (ret == ESP_OK) {
        current_config = new_config;
        ESP_LOGI(TAG, "IO方向设置成功: 掩码=0x%02X, 方向=%s", 
                 io_mask, (direction == PCA9557_IO_INPUT) ? "输入" : "输出");
    }
    
    return ret;
}

/**
 * @brief 设置IO输出电平
 */
esp_err_t pca9557_set_io_level(uint8_t io_mask, uint8_t level)
{
    uint8_t new_output = current_output;
    
    if (level == PCA9557_IO_HIGH) {
        // 设置为高电平 (1)
        new_output |= io_mask;
    } else {
        // 设置为低电平 (0)
        new_output &= ~io_mask;
    }
    
    esp_err_t ret = pca9557_write_register(PCA9557_REG_OUTPUT, new_output);
    if (ret == ESP_OK) {
        current_output = new_output;
        ESP_LOGI(TAG, "IO电平设置成功: 掩码=0x%02X, 电平=%s", 
                 io_mask, (level == PCA9557_IO_HIGH) ? "高" : "低");
    }
    
    return ret;
}

/**
 * @brief 读取IO输入电平
 */
esp_err_t pca9557_read_io_level(uint8_t io_mask, uint8_t *level)
{
    uint8_t input_data;
    esp_err_t ret = pca9557_read_register(PCA9557_REG_INPUT, &input_data);
    
    if (ret == ESP_OK) {
        *level = (input_data & io_mask) ? PCA9557_IO_HIGH : PCA9557_IO_LOW;
        ESP_LOGI(TAG, "IO电平读取成功: 掩码=0x%02X, 电平=%s", 
                 io_mask, (*level == PCA9557_IO_HIGH) ? "高" : "低");
    }
    
    return ret;
}

/**
 * @brief 读取所有IO状态
 */
esp_err_t pca9557_read_all_status(uint8_t *input_levels, uint8_t *output_levels, uint8_t *config)
{
    esp_err_t ret;
    
    // 读取输入状态
    ret = pca9557_read_register(PCA9557_REG_INPUT, input_levels);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // 读取输出状态
    ret = pca9557_read_register(PCA9557_REG_OUTPUT, output_levels);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // 读取配置状态
    ret = pca9557_read_register(PCA9557_REG_CONFIG, config);
    if (ret != ESP_OK) {
        return ret;
    }
    
    ESP_LOGI(TAG, "所有状态读取成功:");
    ESP_LOGI(TAG, "  输入状态: 0x%02X", *input_levels);
    ESP_LOGI(TAG, "  输出状态: 0x%02X", *output_levels);
    ESP_LOGI(TAG, "  配置状态: 0x%02X", *config);
    
    return ESP_OK;
}

/**
 * @brief 反转指定IO的电平
 */
esp_err_t pca9557_toggle_io(uint8_t io_mask)
{
    uint8_t new_output = current_output ^ io_mask;
    
    esp_err_t ret = pca9557_write_register(PCA9557_REG_OUTPUT, new_output);
    if (ret == ESP_OK) {
        current_output = new_output;
        ESP_LOGI(TAG, "IO电平反转成功: 掩码=0x%02X", io_mask);
    }
    
    return ret;
}

/**
 * @brief 配置所有IO为输出模式并设置为指定电平
 */
esp_err_t pca9557_config_all_outputs(uint8_t levels)
{
    esp_err_t ret;
    
    // 设置所有IO为输出模式
    ret = pca9557_write_register(PCA9557_REG_CONFIG, 0x00);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // 设置输出电平
    ret = pca9557_write_register(PCA9557_REG_OUTPUT, levels);
    if (ret == ESP_OK) {
        current_config = 0x00;  // 所有IO为输出
        current_output = levels;
        ESP_LOGI(TAG, "所有IO配置为输出模式成功: 电平=0x%02X", levels);
    }
    
    return ret;
}

/**
 * @brief 配置所有IO为输入模式
 */
esp_err_t pca9557_config_all_inputs(void)
{
    esp_err_t ret = pca9557_write_register(PCA9557_REG_CONFIG, 0xFF);
    if (ret == ESP_OK) {
        current_config = 0xFF;  // 所有IO为输入
        ESP_LOGI(TAG, "所有IO配置为输入模式成功");
    }
    
    return ret;
} 