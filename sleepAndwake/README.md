# ESP32-S3 休眠唤醒功能演示

这个项目演示了ESP32-S3的休眠唤醒功能，实现了自动轻度睡眠和GPIO按钮唤醒的完整功能。

## 功能特性

- **自动休眠**: 系统上电后10秒自动进入轻度睡眠模式
- **按钮唤醒**: 通过GPIO0按钮可以立即唤醒系统
- **循环休眠**: 唤醒后10秒会再次自动进入睡眠模式
- **唤醒原因检测**: 系统会显示唤醒的具体原因
- **实时状态监控**: 显示GPIO按钮的实时状态

## 硬件连接

### 唤醒按钮连接
- 将按钮一端连接到ESP32-S3的GPIO0引脚
- 将按钮另一端连接到GND（接地）
- 按钮按下时，GPIO0引脚被拉低，触发唤醒

```
ESP32-S3 GPIO0 ────┐
                   │
                  [按钮]
                   │
                  GND
```

**注意**: 确保按钮连接正确，按下时GPIO0确实接地。如果使用面包板，请检查连接是否牢固。

## 软件功能

### 主要组件

1. **GPIO配置**: GPIO0配置为输入模式，启用内部上拉电阻
2. **唤醒源配置**: 使用`esp_sleep_enable_ext0_wakeup()`配置GPIO0为唤醒源
3. **定时器**: 使用ESP高精度定时器在10秒后触发睡眠
4. **轻度睡眠**: 使用`esp_light_sleep_start()`进入轻度睡眠模式
5. **唤醒检测**: 通过`esp_sleep_get_wakeup_cause()`检测唤醒原因

### 工作流程

1. 系统启动，初始化GPIO和定时器
2. 10秒后自动进入轻度睡眠模式
3. 可以通过以下方式唤醒：
   - 按下GPIO0按钮（EXT0唤醒）
   - 其他唤醒源（如果配置了）
4. 唤醒后显示唤醒原因和GPIO状态
5. 重新设置定时器，10秒后再次进入睡眠

## 编译和烧录

### 环境要求
- ESP-IDF v5.4.1 或更高版本
- Python 3.7 或更高版本
- 支持ESP32-S3的开发环境

### 编译步骤

```bash
# 1. 进入项目目录
cd sleepAndwake

# 2. 配置项目（可选，使用默认配置即可）
idf.py menuconfig

# 3. 编译项目
idf.py build

# 4. 烧录到ESP32-S3
idf.py flash

# 5. 监控串口输出
idf.py monitor
```

### 一键编译烧录监控
```bash
idf.py flash monitor
```

## 串口输出示例

### 正常启动
```
I (1234) SLEEP_WAKEUP: 系统启动，开始休眠唤醒功能演示
I (1235) SLEEP_WAKEUP: GPIO0 已配置为输入模式，启用内部上拉电阻
I (1236) SLEEP_WAKEUP: 已启用GPIO0作为唤醒源，低电平触发
I (1237) SLEEP_WAKEUP: 定时器已启动，10秒后将自动进入轻度睡眠模式
I (1238) SLEEP_WAKEUP: 按下GPIO0按钮可以立即唤醒系统
```

### 进入睡眠
```
I (11244) SLEEP_WAKEUP: 定时器触发，准备进入轻度睡眠模式...
```

### 按钮唤醒
```
I (11245) SLEEP_WAKEUP: 从轻度睡眠中唤醒！
I (11246) SLEEP_WAKEUP: 唤醒原因: EXT0唤醒 (GPIO0按钮)
I (11247) SLEEP_WAKEUP: GPIO0 当前状态: 低电平(按钮按下)
I (11248) SLEEP_WAKEUP: 已重新设置定时器，10秒后将再次进入睡眠模式
```

## 技术细节

### 轻度睡眠模式特点
- **功耗**: 比正常工作模式低很多
- **唤醒时间**: 非常短，适合需要快速响应的应用
- **内存保持**: RAM内容保持，唤醒后程序继续执行
- **外设状态**: CPU和大部分外设进入低功耗状态

### GPIO唤醒配置
- **引脚**: GPIO0
- **触发方式**: 低电平触发（按钮按下时接地）
- **配置函数**: `esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0)`
- **内部上拉**: 启用内部上拉电阻，确保按钮释放时引脚为高电平

### 定时器配置
- **类型**: ESP高精度定时器
- **精度**: 微秒级精度
- **回调**: 10秒后触发睡眠回调函数
- **重复**: 唤醒后重新设置定时器

## 故障排除

### 常见问题

1. **按钮按下但无法唤醒**
   - 检查按钮连接是否正确
   - 确认按钮按下时GPIO0确实接地
   - 检查按钮是否有机械故障

2. **唤醒原因显示不正确**
   - 确认使用的是EXT0唤醒源
   - 检查GPIO配置是否正确
   - 验证按钮连接质量

3. **编译错误**
   - 确保ESP-IDF版本正确
   - 检查CMakeLists.txt中的组件依赖
   - 确认所有头文件都能正确包含

4. **定时器不工作**
   - 检查esp_timer组件是否正确包含
   - 确认定时器回调函数正确注册
   - 验证定时器参数设置

### 调试技巧

1. **观察GPIO状态**: 程序会实时显示GPIO0的状态
2. **检查唤醒原因**: 每次唤醒都会显示具体的唤醒原因
3. **串口监控**: 使用`idf.py monitor`实时查看日志输出
4. **硬件测试**: 用万用表测量GPIO0电压，确认按钮工作正常



## 项目结构

```
sleepAndwake/
├── main/
│   ├── hello_world_main.c    # 主程序文件
│   ├── i2c_master.c          # I2C驱动实现
│   ├── i2c_master.h          # I2C驱动头文件
│   └── CMakeLists.txt        # 组件构建配置
├── CMakeLists.txt            # 项目构建配置
├── sdkconfig                 # 项目配置文件
└── README.md                 # 项目说明文档
```

