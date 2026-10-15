# STM32F407VET6 驱动移植版

本目录是新增的 F407VET6 版本，原 `Drivers/DshanMCU-F103` 文件没有修改。新驱动函数统一使用 `F407_` 前缀，因此可与旧驱动并存。

## 2.1英寸触摸LCD

图片中的屏幕分为两个独立接口：上排 ST7789（MOSI、DC、SCK、CS），下排 CST816D（GND、3V3、SCL、SDA）。默认连接如下：

| 屏幕信号 | F407VET6 引脚 | 外设/说明 |
|---|---|---|
| ST7789 MOSI | PC12 | SPI3_MOSI（AF6） |
| ST7789 SCK | PC10 | SPI3_SCK（AF6） |
| ST7789 DC | PC0 | 普通推挽输出 |
| ST7789 CS | PC1 | 普通推挽输出，低有效 |
| CST816D SCL | PA8 | I2C3_SCL（AF4） |
| CST816D SDA | PC9 | I2C3_SDA（AF4） |
| GND/3V3 | F407 GND/3V3 | 只能使用 3.3 V 逻辑 |

该屏为 240×296 像素，驱动默认 `F407_ST7789_WIDTH=240`、`F407_ST7789_HEIGHT=296`。连接器未引出 RST 和 BL，驱动不会擅自占用主控引脚；这两路由屏幕板处理。若实物需要外部复位或背光控制，应在 `driver_f407_pins.h` 增加实际接线后再扩展初始化。

## 默认接线

| 模块 | F407 引脚/外设 |
|---|---|
| 板载LED | PC13，低电平点亮 |
| RGB LED | PC6/PC7/PC8 = TIM3_CH1/CH2/CH3，低电平点亮 |
| DHT11 | PA1 |
| 红外避障 | PB13 |
| 红外接收 | PB10，EXTI双边沿 |
| MPU6050 | I2C1：PB6=SCL、PB7=SDA |
| W25Q64 | SPI1：PA5=SCK、PA6=MISO、PA7=MOSI，PA4=CS |
| HC-SR04 | PB0=TRIG、PB1=ECHO |
| 调试串口 | USART1：PA9=TX、PA10=RX |

这些选择均来自图片中的复用标注；若扩展板实际丝印接线不同，修改 `driver_f407_pins.h` 即可。

## CubeMX 要求

选择 STM32F407VET6（LQFP100），打开 GPIOA/B/C 时钟；配置 TIM3 PWM CH1-3、I2C1、SPI1、USART1，并把 PB10 配为 GPIO_EXTI 双边沿。初始化函数由 CubeMX 生成，新驱动只使用句柄，不会重复初始化外设。

红外接收的 `F407_IRReceiver_EXTI_Callback()` 需要在用户自己的 `HAL_GPIO_EXTI_Callback()` 中转发；不要在中断里调用打印或延时函数。

## 接入现有工程

本工程根目录的 `CMakeLists.txt` 已自动收集 `Drivers/DshanMCU-F407/driver_*.c`，并加入该目录的头文件搜索路径；`stm32f4xx_hal_conf.h` 和 CMake 已打开并编译 I2C、SPI、TIM、UART HAL 模块。应用层先由 CubeMX 初始化对应句柄（`hspi3`、`hi2c3` 等），再调用 `F407_LCD_Init()`、`F407_CST816D_Init()` 或其他 `F407_...` 接口即可。驱动不重复创建外设句柄，便于以后重新生成 CubeMX 代码。

空模板也提供了 `MX_F407_SPI3_Init()` 和 `MX_F407_I2C3_Init()`（见 `Core/Src/f407_peripherals.c`），最小初始化顺序为：

```c
MX_F407_SPI3_Init();
MX_F407_I2C3_Init();
F407_LCD_Init();
F407_CST816D_Init();
```

LVGL 的 `flush_cb` 可直接调用 `F407_ST7789_Flush(x1, y1, x2, y2, color_p)`，函数会把主机端 RGB565 转成 ST7789 所需的高字节在前格式。

当前 LVGL 移植使用 `F407_ST7789_FlushDMA()` 异步发送像素，原来的 `F407_ST7789_Flush()` 保留为阻塞接口。SPI3 TX 使用 DMA1 Stream5 / Channel0、Normal 模式、字节对齐和内存地址递增；DMA 与 SPI3 中断优先级均为 5，配置已同步到 `.ioc`。

DMA 接口会原地转换 RGB565 字节序，缓冲区必须位于 DMA 可访问的 SRAM，不能位于 CCMRAM。发送期间不能修改缓冲区或调用其他 LCD 接口；超过 65534 字节的区域由 DMA 完成回调分段续传。命令和窗口设置仍使用阻塞发送。

`F407_ST7789_FlushCompleteCallback()` 在中断中通知 UI 任务的静态二值信号量。LVGL 的 `flush_wait_cb` 阻塞等待完成后才调用 `lv_display_flush_ready()`；错误或 250ms 超时会先中止 SPI/DMA，再释放缓冲区。可在调试器观察 `f407_lvgl.c` 中的 `s_flush_error_count`。当前仍使用单个 30 行绘图缓冲区，DMA 等待期间其他 FreeRTOS 任务可以运行；SPI 时钟保持 21MHz。
