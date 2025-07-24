/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2C配置参数
#define I2C_MASTER_SCL_IO           2     // SCL引脚
#define I2C_MASTER_SDA_IO           1      // SDA引脚
#define I2C_MASTER_NUM              I2C_NUM_0 // I2C端口号
#define I2C_MASTER_FREQ_HZ          100000  // I2C主频
#define I2C_MASTER_TX_BUF_DISABLE   0       // 禁用发送缓冲区
#define I2C_MASTER_RX_BUF_DISABLE   0       // 禁用接收缓冲区
#define I2C_MASTER_TIMEOUT_MS       1000    // 超时时间

/**
 * @brief 检查I2C引脚连接状态
 * @return ESP_OK 成功, ESP_ERR_INVALID_STATE 引脚连接异常
 */
esp_err_t i2c_check_connection(void);

/**
 * @brief I2C主机初始化
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t i2c_master_init(void);

/**
 * @brief 测试I2C总线是否正常工作
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t i2c_test_bus(void);

/**
 * @brief I2C写数据到从设备
 * @param slave_addr 从设备地址
 * @param data 要写入的数据
 * @param size 数据大小
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t i2c_master_write_slave(uint8_t slave_addr, uint8_t *data, size_t size);

/**
 * @brief 从I2C从设备读取数据
 * @param slave_addr 从设备地址
 * @param data 接收数据的缓冲区
 * @param size 要读取的数据大小
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t i2c_master_read_slave(uint8_t slave_addr, uint8_t *data, size_t size);

/**
 * @brief 扫描I2C设备并返回发现的设备数量
 * @return 发现的I2C设备数量
 */
int i2c_scan_devices(void);

/**
 * @brief 释放I2C驱动
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t i2c_master_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // I2C_MASTER_H 