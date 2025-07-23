/*
 * XL9555 IO扩展芯片驱动头文件
 * 
 * XL9555是一个16位I/O扩展器，支持I2C接口
 * 芯片地址: 0x20 (7位地址)
 */

#ifndef XL9555_H
#define XL9555_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// XL9555芯片地址
#define XL9555_I2C_ADDR              0x20

// XL9555寄存器地址
#define XL9555_REG_INPUT_PORT_0      0x00    // 输入端口0 (P0-P7)
#define XL9555_REG_INPUT_PORT_1      0x01    // 输入端口1 (P8-P15)
#define XL9555_REG_OUTPUT_PORT_0     0x02    // 输出端口0 (P0-P7)
#define XL9555_REG_OUTPUT_PORT_1     0x03    // 输出端口1 (P8-P15)
#define XL9555_REG_CONFIG_PORT_0     0x06    // 配置端口0 (P0-P7)
#define XL9555_REG_CONFIG_PORT_1     0x07    // 配置端口1 (P8-P15)

// 引脚定义 (16个IO引脚)
typedef enum {
    XL9555_PIN_P0 = 0,   // 端口0，引脚0
    XL9555_PIN_P1,       // 端口0，引脚1
    XL9555_PIN_P2,       // 端口0，引脚2
    XL9555_PIN_P3,       // 端口0，引脚3
    XL9555_PIN_P4,       // 端口0，引脚4
    XL9555_PIN_P5,       // 端口0，引脚5
    XL9555_PIN_P6,       // 端口0，引脚6
    XL9555_PIN_P7,       // 端口0，引脚7
    XL9555_PIN_P8,       // 端口1，引脚0
    XL9555_PIN_P9,       // 端口1，引脚1
    XL9555_PIN_P10,      // 端口1，引脚2
    XL9555_PIN_P11,      // 端口1，引脚3
    XL9555_PIN_P12,      // 端口1，引脚4
    XL9555_PIN_P13,      // 端口1，引脚5
    XL9555_PIN_P14,      // 端口1，引脚6
    XL9555_PIN_P15,      // 端口1，引脚7
    XL9555_PIN_MAX
} xl9555_pin_t;

// 引脚方向
typedef enum {
    XL9555_DIR_INPUT = 0,    // 输入模式
    XL9555_DIR_OUTPUT = 1    // 输出模式
} xl9555_direction_t;

// 引脚电平
typedef enum {
    XL9555_LEVEL_LOW = 0,    // 低电平
    XL9555_LEVEL_HIGH = 1    // 高电平
} xl9555_level_t;

// 端口掩码 (用于批量操作)
#define XL9555_PORT0_MASK    0xFF    // 端口0掩码 (P0-P7)
#define XL9555_PORT1_MASK    0xFF    // 端口1掩码 (P8-P15)

// 按钮定义 (连接到IO1_7到IO1_4，即P15到P12)
#define XL9555_KEY0_PIN      XL9555_PIN_P15    // KEY0 -> P15 (IO1_7)
#define XL9555_KEY1_PIN      XL9555_PIN_P14    // KEY1 -> P14 (IO1_6)
#define XL9555_KEY2_PIN      XL9555_PIN_P13    // KEY2 -> P13 (IO1_5)
#define XL9555_KEY3_PIN      XL9555_PIN_P12    // KEY3 -> P12 (IO1_4)

// 按钮掩码 (高4位为按钮引脚)
#define XL9555_KEY_MASK      0xF0    // 0xF0 = 11110000，对应P15-P12

/**
 * @brief XL9555初始化
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t xl9555_init(void);

/**
 * @brief 检查XL9555是否存在
 * @return ESP_OK 存在, 其他值表示不存在或错误
 */
esp_err_t xl9555_check_presence(void);

/**
 * @brief 设置引脚方向
 * @param pin 引脚号
 * @param direction 方向 (输入/输出)
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t xl9555_set_pin_direction(xl9555_pin_t pin, xl9555_direction_t direction);

/**
 * @brief 批量设置引脚方向
 * @param port0_mask 端口0方向掩码 (0=输入, 1=输出)
 * @param port1_mask 端口1方向掩码 (0=输入, 1=输出)
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t xl9555_set_port_direction(uint8_t port0_mask, uint8_t port1_mask);

/**
 * @brief 设置输出引脚电平
 * @param pin 引脚号
 * @param level 电平 (高/低)
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t xl9555_set_pin_level(xl9555_pin_t pin, xl9555_level_t level);

/**
 * @brief 批量设置输出引脚电平
 * @param port0_level 端口0电平值
 * @param port1_level 端口1电平值
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t xl9555_set_port_level(uint8_t port0_level, uint8_t port1_level);

/**
 * @brief 读取输入引脚电平
 * @param pin 引脚号
 * @param level 返回的电平值
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t xl9555_get_pin_level(xl9555_pin_t pin, xl9555_level_t *level);

/**
 * @brief 批量读取输入引脚电平
 * @param port0_level 返回端口0电平值
 * @param port1_level 返回端口1电平值
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t xl9555_get_port_level(uint8_t *port0_level, uint8_t *port1_level);

/**
 * @brief 翻转输出引脚电平
 * @param pin 引脚号
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t xl9555_toggle_pin(xl9555_pin_t pin);

/**
 * @brief 读取当前输出引脚状态
 * @param port0_level 返回端口0输出状态
 * @param port1_level 返回端口1输出状态
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t xl9555_get_output_status(uint8_t *port0_level, uint8_t *port1_level);

/**
 * @brief 读取当前配置状态
 * @param port0_config 返回端口0配置 (0=输出, 1=输入)
 * @param port1_config 返回端口1配置 (0=输出, 1=输入)
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t xl9555_get_config(uint8_t *port0_config, uint8_t *port1_config);

/**
 * @brief 初始化按钮 (设置P12-P15为输入，启用内部上拉)
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t xl9555_keys_init(void);

/**
 * @brief 读取单个按钮状态
 * @param key_pin 按钮引脚 (XL9555_KEY0_PIN 到 XL9555_KEY3_PIN)
 * @param is_pressed 返回按钮是否按下 (true=按下, false=释放)
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t xl9555_key_read(xl9555_pin_t key_pin, bool *is_pressed);

/**
 * @brief 读取所有按钮状态
 * @param key_states 返回按钮状态数组 [KEY0, KEY1, KEY2, KEY3]
 * @return ESP_OK 成功, 其他值表示错误
 */
esp_err_t xl9555_keys_read_all(bool key_states[4]);

/**
 * @brief 等待按钮按下 (阻塞方式)
 * @param key_pin 按钮引脚
 * @param timeout_ms 超时时间(毫秒)，0表示无限等待
 * @return ESP_OK 按钮按下, ESP_ERR_TIMEOUT 超时
 */
esp_err_t xl9555_key_wait_press(xl9555_pin_t key_pin, uint32_t timeout_ms);



#ifdef __cplusplus
}
#endif

#endif // XL9555_H 