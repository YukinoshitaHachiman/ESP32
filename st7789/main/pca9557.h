/*
 * PCA9557PW IO扩展芯片驱动头文件
 * 8位I2C IO扩展器
 */

#ifndef PCA9557_H
#define PCA9557_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// PCA9557PW I2C地址 (0x19)
#define PCA9557_I2C_ADDR            0x19

// PCA9557PW寄存器地址
#define PCA9557_REG_INPUT           0x00    // 输入端口寄存器
#define PCA9557_REG_OUTPUT          0x01    // 输出端口寄存器
#define PCA9557_REG_CONFIG          0x03    // 配置寄存器

// IO引脚定义
#define PCA9557_IO0                 0x01    // IO0 (位0)
#define PCA9557_IO1                 0x02    // IO1 (位1)
#define PCA9557_IO2                 0x04    // IO2 (位2)
#define PCA9557_IO3                 0x08    // IO3 (位3)
#define PCA9557_IO4                 0x10    // IO4 (位4)
#define PCA9557_IO5                 0x20    // IO5 (位5)
#define PCA9557_IO6                 0x40    // IO6 (位6)
#define PCA9557_IO7                 0x80    // IO7 (位7)
#define PCA9557_ALL_IO              0xFF    // 所有IO

// IO方向定义
#define PCA9557_IO_INPUT            1       // 输入模式
#define PCA9557_IO_OUTPUT           0       // 输出模式

// IO电平定义
#define PCA9557_IO_LOW              0       // 低电平
#define PCA9557_IO_HIGH             1       // 高电平

/**
 * @brief PCA9557PW初始化
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t pca9557_init(void);

/**
 * @brief 检查PCA9557PW是否存在
 * @return ESP_OK 存在, ESP_ERR_NOT_FOUND 不存在
 */
esp_err_t pca9557_check_device(void);

/**
 * @brief 设置IO方向
 * @param io_mask IO引脚掩码 (使用PCA9557_IOx定义)
 * @param direction 方向 (PCA9557_IO_INPUT 或 PCA9557_IO_OUTPUT)
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t pca9557_set_io_direction(uint8_t io_mask, uint8_t direction);

/**
 * @brief 设置IO输出电平
 * @param io_mask IO引脚掩码 (使用PCA9557_IOx定义)
 * @param level 电平 (PCA9557_IO_LOW 或 PCA9557_IO_HIGH)
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t pca9557_set_io_level(uint8_t io_mask, uint8_t level);

/**
 * @brief 读取IO输入电平
 * @param io_mask IO引脚掩码 (使用PCA9557_IOx定义)
 * @param level 返回的电平值
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t pca9557_read_io_level(uint8_t io_mask, uint8_t *level);

/**
 * @brief 读取所有IO状态
 * @param input_levels 输入IO电平 (8位)
 * @param output_levels 输出IO电平 (8位)
 * @param config 配置寄存器值 (8位)
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t pca9557_read_all_status(uint8_t *input_levels, uint8_t *output_levels, uint8_t *config);

/**
 * @brief 反转指定IO的电平
 * @param io_mask IO引脚掩码 (使用PCA9557_IOx定义)
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t pca9557_toggle_io(uint8_t io_mask);

/**
 * @brief 配置所有IO为输出模式并设置为指定电平
 * @param levels 8位电平值，每位对应一个IO
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t pca9557_config_all_outputs(uint8_t levels);

/**
 * @brief 配置所有IO为输入模式
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t pca9557_config_all_inputs(void);

#ifdef __cplusplus
}
#endif

#endif // PCA9557_H 