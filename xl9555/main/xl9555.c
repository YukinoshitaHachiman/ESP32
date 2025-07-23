/*
 * XL9555 IO扩展芯片驱动实现
 * 
 * XL9555是一个16位I/O扩展器，支持I2C接口
 * 芯片地址: 0x20 (7位地址)
 */

#include "xl9555.h"
#include "i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "XL9555";

// 内部函数声明
static esp_err_t xl9555_write_register(uint8_t reg, uint8_t data);
static esp_err_t xl9555_read_register(uint8_t reg, uint8_t *data);
static uint8_t xl9555_pin_to_port(xl9555_pin_t pin);
static uint8_t xl9555_pin_to_bit(xl9555_pin_t pin);

/**
 * @brief XL9555初始化
 */
esp_err_t xl9555_init(void)
{
    ESP_LOGI(TAG, "初始化XL9555 IO扩展芯片...");
    
    // 检查芯片是否存在
    esp_err_t ret = xl9555_check_presence();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "XL9555芯片未找到，请检查硬件连接");
        return ret;
    }
    
    // 默认配置：所有引脚设为输入模式
    ret = xl9555_set_port_direction(0xFF, 0xFF);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "XL9555默认配置失败");
        return ret;
    }
    
    // 设置所有输出引脚为低电平
    ret = xl9555_set_port_level(0x00, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "XL9555输出初始化失败");
        return ret;
    }
    
    ESP_LOGI(TAG, "XL9555初始化成功");
    return ESP_OK;
}

/**
 * @brief 检查XL9555是否存在
 */
esp_err_t xl9555_check_presence(void)
{
    // 尝试读取配置寄存器来检查芯片是否存在
    uint8_t config0, config1;
    esp_err_t ret = xl9555_read_register(XL9555_REG_CONFIG_PORT_0, &config0);
    if (ret != ESP_OK) {
        return ret;
    }
    
    ret = xl9555_read_register(XL9555_REG_CONFIG_PORT_1, &config1);
    if (ret != ESP_OK) {
        return ret;
    }
    
    ESP_LOGI(TAG, "XL9555芯片检测成功，当前配置: Port0=0x%02X, Port1=0x%02X", config0, config1);
    return ESP_OK;
}

/**
 * @brief 设置引脚方向
 */
esp_err_t xl9555_set_pin_direction(xl9555_pin_t pin, xl9555_direction_t direction)
{
    if (pin >= XL9555_PIN_MAX) {
        ESP_LOGE(TAG, "无效的引脚号: %d", pin);
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t port = xl9555_pin_to_port(pin);
    uint8_t bit = xl9555_pin_to_bit(pin);
    uint8_t reg = (port == 0) ? XL9555_REG_CONFIG_PORT_0 : XL9555_REG_CONFIG_PORT_1;
    
    // 读取当前配置
    uint8_t current_config;
    esp_err_t ret = xl9555_read_register(reg, &current_config);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // 修改指定位
    if (direction == XL9555_DIR_INPUT) {
        current_config |= (1 << bit);  // 设为输入 (1)
    } else {
        current_config &= ~(1 << bit); // 设为输出 (0)
    }
    
    // 写回配置
    ret = xl9555_write_register(reg, current_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "引脚P%d方向设置为: %s", pin, 
                 (direction == XL9555_DIR_INPUT) ? "输入" : "输出");
    }
    
    return ret;
}

/**
 * @brief 批量设置引脚方向
 */
esp_err_t xl9555_set_port_direction(uint8_t port0_mask, uint8_t port1_mask)
{
    esp_err_t ret;
    
    // 设置端口0方向
    ret = xl9555_write_register(XL9555_REG_CONFIG_PORT_0, port0_mask);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置端口0方向失败");
        return ret;
    }
    
    // 设置端口1方向
    ret = xl9555_write_register(XL9555_REG_CONFIG_PORT_1, port1_mask);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置端口1方向失败");
        return ret;
    }
    
    ESP_LOGI(TAG, "端口方向设置成功: Port0=0x%02X, Port1=0x%02X", port0_mask, port1_mask);
    return ESP_OK;
}

/**
 * @brief 设置输出引脚电平
 */
esp_err_t xl9555_set_pin_level(xl9555_pin_t pin, xl9555_level_t level)
{
    if (pin >= XL9555_PIN_MAX) {
        ESP_LOGE(TAG, "无效的引脚号: %d", pin);
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t port = xl9555_pin_to_port(pin);
    uint8_t bit = xl9555_pin_to_bit(pin);
    uint8_t reg = (port == 0) ? XL9555_REG_OUTPUT_PORT_0 : XL9555_REG_OUTPUT_PORT_1;
    
    // 读取当前输出状态
    uint8_t current_output;
    esp_err_t ret = xl9555_read_register(reg, &current_output);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // 修改指定位
    if (level == XL9555_LEVEL_HIGH) {
        current_output |= (1 << bit);  // 设为高电平
    } else {
        current_output &= ~(1 << bit); // 设为低电平
    }
    
    // 写回输出状态
    ret = xl9555_write_register(reg, current_output);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "引脚P%d电平设置为: %s", pin, 
                 (level == XL9555_LEVEL_HIGH) ? "高" : "低");
    }
    
    return ret;
}

/**
 * @brief 批量设置输出引脚电平
 */
esp_err_t xl9555_set_port_level(uint8_t port0_level, uint8_t port1_level)
{
    esp_err_t ret;
    
    // 设置端口0电平
    ret = xl9555_write_register(XL9555_REG_OUTPUT_PORT_0, port0_level);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置端口0电平失败");
        return ret;
    }
    
    // 设置端口1电平
    ret = xl9555_write_register(XL9555_REG_OUTPUT_PORT_1, port1_level);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置端口1电平失败");
        return ret;
    }
    
    ESP_LOGI(TAG, "端口电平设置成功: Port0=0x%02X, Port1=0x%02X", port0_level, port1_level);
    return ESP_OK;
}

/**
 * @brief 读取输入引脚电平
 */
esp_err_t xl9555_get_pin_level(xl9555_pin_t pin, xl9555_level_t *level)
{
    if (pin >= XL9555_PIN_MAX || level == NULL) {
        ESP_LOGE(TAG, "无效的参数: pin=%d, level=%p", pin, level);
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t port = xl9555_pin_to_port(pin);
    uint8_t bit = xl9555_pin_to_bit(pin);
    uint8_t reg = (port == 0) ? XL9555_REG_INPUT_PORT_0 : XL9555_REG_INPUT_PORT_1;
    
    // 读取输入状态
    uint8_t input_status;
    esp_err_t ret = xl9555_read_register(reg, &input_status);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // 提取指定位的状态
    *level = (input_status & (1 << bit)) ? XL9555_LEVEL_HIGH : XL9555_LEVEL_LOW;
    
    ESP_LOGI(TAG, "引脚P%d电平读取: %s", pin, 
             (*level == XL9555_LEVEL_HIGH) ? "高" : "低");
    
    return ESP_OK;
}

/**
 * @brief 批量读取输入引脚电平
 */
esp_err_t xl9555_get_port_level(uint8_t *port0_level, uint8_t *port1_level)
{
    if (port0_level == NULL || port1_level == NULL) {
        ESP_LOGE(TAG, "无效的参数");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret;
    
    // 读取端口0输入状态
    ret = xl9555_read_register(XL9555_REG_INPUT_PORT_0, port0_level);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取端口0输入状态失败");
        return ret;
    }
    
    // 读取端口1输入状态
    ret = xl9555_read_register(XL9555_REG_INPUT_PORT_1, port1_level);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取端口1输入状态失败");
        return ret;
    }
    
    ESP_LOGI(TAG, "端口输入状态: Port0=0x%02X, Port1=0x%02X", *port0_level, *port1_level);
    return ESP_OK;
}

/**
 * @brief 翻转输出引脚电平
 */
esp_err_t xl9555_toggle_pin(xl9555_pin_t pin)
{
    if (pin >= XL9555_PIN_MAX) {
        ESP_LOGE(TAG, "无效的引脚号: %d", pin);
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t port = xl9555_pin_to_port(pin);
    uint8_t bit = xl9555_pin_to_bit(pin);
    uint8_t reg = (port == 0) ? XL9555_REG_OUTPUT_PORT_0 : XL9555_REG_OUTPUT_PORT_1;
    
    // 读取当前输出状态
    uint8_t current_output;
    esp_err_t ret = xl9555_read_register(reg, &current_output);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // 翻转指定位
    current_output ^= (1 << bit);
    
    // 写回输出状态
    ret = xl9555_write_register(reg, current_output);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "引脚P%d电平已翻转", pin);
    }
    
    return ret;
}

/**
 * @brief 读取当前输出引脚状态
 */
esp_err_t xl9555_get_output_status(uint8_t *port0_level, uint8_t *port1_level)
{
    if (port0_level == NULL || port1_level == NULL) {
        ESP_LOGE(TAG, "无效的参数");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret;
    
    // 读取端口0输出状态
    ret = xl9555_read_register(XL9555_REG_OUTPUT_PORT_0, port0_level);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取端口0输出状态失败");
        return ret;
    }
    
    // 读取端口1输出状态
    ret = xl9555_read_register(XL9555_REG_OUTPUT_PORT_1, port1_level);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取端口1输出状态失败");
        return ret;
    }
    
    ESP_LOGI(TAG, "端口输出状态: Port0=0x%02X, Port1=0x%02X", *port0_level, *port1_level);
    return ESP_OK;
}

/**
 * @brief 读取当前配置状态
 */
esp_err_t xl9555_get_config(uint8_t *port0_config, uint8_t *port1_config)
{
    if (port0_config == NULL || port1_config == NULL) {
        ESP_LOGE(TAG, "无效的参数");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret;
    
    // 读取端口0配置
    ret = xl9555_read_register(XL9555_REG_CONFIG_PORT_0, port0_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取端口0配置失败");
        return ret;
    }
    
    // 读取端口1配置
    ret = xl9555_read_register(XL9555_REG_CONFIG_PORT_1, port1_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取端口1配置失败");
        return ret;
    }
    
    ESP_LOGI(TAG, "端口配置: Port0=0x%02X, Port1=0x%02X", *port0_config, *port1_config);
    return ESP_OK;
}



/**
 * @brief 初始化按钮 (设置P12-P15为输入，启用内部上拉)
 */
esp_err_t xl9555_keys_init(void)
{
    ESP_LOGI(TAG, "初始化按钮 (P12-P15)...");
    
    // 设置P12-P15为输入模式
    esp_err_t ret = xl9555_set_pin_direction(XL9555_KEY0_PIN, XL9555_DIR_INPUT);
    ret |= xl9555_set_pin_direction(XL9555_KEY1_PIN, XL9555_DIR_INPUT);
    ret |= xl9555_set_pin_direction(XL9555_KEY2_PIN, XL9555_DIR_INPUT);
    ret |= xl9555_set_pin_direction(XL9555_KEY3_PIN, XL9555_DIR_INPUT);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置按钮引脚为输入模式失败");
        return ret;
    }
    
    // 等待一段时间让信号稳定
    vTaskDelay(10 / portTICK_PERIOD_MS);
    
    // 读取初始状态
    bool key_states[4];
    ret = xl9555_keys_read_all(key_states);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "按钮初始状态: KEY0=%s, KEY1=%s, KEY2=%s, KEY3=%s",
                 key_states[0] ? "按下" : "释放",
                 key_states[1] ? "按下" : "释放", 
                 key_states[2] ? "按下" : "释放",
                 key_states[3] ? "按下" : "释放");
    }
    
    ESP_LOGI(TAG, "按钮初始化完成");
    return ESP_OK;
}

/**
 * @brief 读取单个按钮状态
 */
esp_err_t xl9555_key_read(xl9555_pin_t key_pin, bool *is_pressed)
{
    if (is_pressed == NULL) {
        ESP_LOGE(TAG, "无效的参数: is_pressed=NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    // 检查是否为有效的按钮引脚
    if (key_pin != XL9555_KEY0_PIN && key_pin != XL9555_KEY1_PIN && 
        key_pin != XL9555_KEY2_PIN && key_pin != XL9555_KEY3_PIN) {
        ESP_LOGE(TAG, "无效的按钮引脚: %d", key_pin);
        return ESP_ERR_INVALID_ARG;
    }
    
    xl9555_level_t level;
    esp_err_t ret = xl9555_get_pin_level(key_pin, &level);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // 按钮按下时接地，所以低电平表示按下
    *is_pressed = (level == XL9555_LEVEL_LOW);
    
    return ESP_OK;
}

/**
 * @brief 读取所有按钮状态
 */
esp_err_t xl9555_keys_read_all(bool key_states[4])
{
    if (key_states == NULL) {
        ESP_LOGE(TAG, "无效的参数: key_states=NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    // 读取端口1的输入状态 (P8-P15)
    uint8_t port1_input;
    esp_err_t ret = xl9555_read_register(XL9555_REG_INPUT_PORT_1, &port1_input);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取端口1输入状态失败");
        return ret;
    }
    
    // 提取按钮状态 (P15-P12对应高4位)
    // 按钮按下时接地，所以低电平表示按下
    key_states[0] = ((port1_input & (1 << 7)) == 0);  // KEY0 (P15)
    key_states[1] = ((port1_input & (1 << 6)) == 0);  // KEY1 (P14)
    key_states[2] = ((port1_input & (1 << 5)) == 0);  // KEY2 (P13)
    key_states[3] = ((port1_input & (1 << 4)) == 0);  // KEY3 (P12)
    
    return ESP_OK;
}

/**
 * @brief 等待按钮按下 (阻塞方式)
 */
esp_err_t xl9555_key_wait_press(xl9555_pin_t key_pin, uint32_t timeout_ms)
{
    // 检查是否为有效的按钮引脚
    if (key_pin != XL9555_KEY0_PIN && key_pin != XL9555_KEY1_PIN && 
        key_pin != XL9555_KEY2_PIN && key_pin != XL9555_KEY3_PIN) {
        ESP_LOGE(TAG, "无效的按钮引脚: %d", key_pin);
        return ESP_ERR_INVALID_ARG;
    }
    
    uint32_t start_time = xTaskGetTickCount();
    bool is_pressed;
    
    while (1) {
        esp_err_t ret = xl9555_key_read(key_pin, &is_pressed);
        if (ret != ESP_OK) {
            return ret;
        }
        
        if (is_pressed) {
            ESP_LOGI(TAG, "按钮按下: P%d", key_pin);
            return ESP_OK;
        }
        
        // 检查超时
        if (timeout_ms > 0) {
            uint32_t elapsed = (xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS;
            if (elapsed >= timeout_ms) {
                ESP_LOGW(TAG, "等待按钮按下超时");
                return ESP_ERR_TIMEOUT;
            }
        }
        
        // 短暂延时，避免过度占用CPU
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}



// ==================== 内部辅助函数 ====================

/**
 * @brief 写寄存器
 */
static esp_err_t xl9555_write_register(uint8_t reg, uint8_t data)
{
    uint8_t write_data[2] = {reg, data};
    return i2c_master_write_slave(XL9555_I2C_ADDR, write_data, 2);
}

/**
 * @brief 读寄存器
 */
static esp_err_t xl9555_read_register(uint8_t reg, uint8_t *data)
{
    // 先写入寄存器地址
    esp_err_t ret = i2c_master_write_slave(XL9555_I2C_ADDR, &reg, 1);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // 然后读取数据
    return i2c_master_read_slave(XL9555_I2C_ADDR, data, 1);
}

/**
 * @brief 引脚号转换为端口号
 */
static uint8_t xl9555_pin_to_port(xl9555_pin_t pin)
{
    return (pin < 8) ? 0 : 1;
}

/**
 * @brief 引脚号转换为位号
 */
static uint8_t xl9555_pin_to_bit(xl9555_pin_t pin)
{
    return pin % 8;
} 