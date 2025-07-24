# ESP32-S3 I2C 主机驱动

一个用于ESP32-S3的完整I2C主机驱动库，提供I2C设备初始化、通信和设备扫描功能。

## 功能特性

- ✅ I2C主机模式初始化
- ✅ 引脚连接状态检测
- ✅ I2C总线功能测试
- ✅ 自动设备扫描
- ✅ 读写数据功能
- ✅ 完整的错误处理
- ✅ 详细的日志输出



## 快速开始

### 1. 编译项目

```bash
idf.py build
```

### 2. 烧录程序

```bash
idf.py flash monitor
```

### 3. 查看输出

程序启动后会自动：
- 检查I2C引脚连接状态
- 初始化I2C主机
- 测试I2C总线功能
- 扫描连接的I2C设备

## 项目结构

```
i2c/
├── main/
│   ├── i2c_master.h      # I2C驱动头文件
│   ├── i2c_master.c      # I2C驱动实现
│   ├── hello_world_main.c # 主程序
│   └── CMakeLists.txt    # 构建配置
├── CMakeLists.txt        # 项目配置
└── README.md            # 项目说明
```

## API 使用

### 初始化I2C

```c
#include "i2c_master.h"

// 初始化I2C主机
esp_err_t ret = i2c_master_init();
if (ret != ESP_OK) {
    // 处理错误
}
```

### 扫描设备

```c
// 扫描并返回设备数量
int device_count = i2c_scan_devices();
```

### 读写数据

```c
// 写入数据
uint8_t data[] = {0x01, 0x02, 0x03};
esp_err_t ret = i2c_master_write_slave(0x10, data, sizeof(data));

// 读取数据
uint8_t read_data[3];
ret = i2c_master_read_slave(0x10, read_data, sizeof(read_data));
```

## 配置参数

在 `i2c_master.h` 中可以修改以下配置：

```c
#define I2C_MASTER_SCL_IO           42      // SCL引脚
#define I2C_MASTER_SDA_IO           41      // SDA引脚
#define I2C_MASTER_FREQ_HZ          100000  // I2C频率 (100kHz)
#define I2C_MASTER_TIMEOUT_MS       1000    // 超时时间
```


