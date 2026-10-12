# 环境监测装置规划架构

当前已经完成第一版最小闭环，只启用DHT11环境采集功能。

## 分层原则

```text
Application（任务、队列、界面协调）
      |                    |
      v                    v
Business（业务规则）     Drivers（硬件访问）
                              |
                              v
                      STM32 HAL / CubeMX
```

- 驱动层只处理硬件初始化和原始数据读写。
- 业务层只处理数据模型、阈值、报警、设置和历史策略。
- 应用层负责创建任务、建立队列并连接驱动层和业务层。
- 业务层不包含 HAL、FreeRTOS、CMSIS-RTOS2 或 LVGL 头文件。
- UiTask 将来是唯一调用 LVGL API 的任务。
- StorageTask 将来是唯一访问 W25Q64 的任务。

## 第一版任务

- `SensorTask`：每2秒读取一次DHT11并发送采样事件。
- `ControlTask`：保存最新采样，只判断DHT11正常或故障。
- `UiTask`：唯一操作LVGL，显示温度、湿度和读取状态。

## 当前状态

- 已加入CMake：`environment_service.c`、运行时、三个任务相关源码和UI源码。
- `InputTask`和`StorageTask`仍是占位文件，没有加入CMake，也不会运行。
- `settings_service`和`history_policy`仍是占位文件，没有加入CMake。
- MPU6050、HC-SR04、红外、W25Q64和RGB灯驱动均未被第一版应用调用。
- LCD和LVGL属于显示基础设施；第一版不初始化触摸控制器。
