#ifndef APP_CONFIG_H                                  /* 防止本头文件被重复包含。 */
#define APP_CONFIG_H                                  /* 定义本头文件的保护宏。 */

/* xTaskCreate()的栈深度单位是StackType_t数量，STM32F407上每个字为4字节。 */
#define APP_SENSOR_TASK_STACK_WORDS       384U        /* SensorTask使用1536字节任务栈。 */
#define APP_MEASURE_TASK_STACK_WORDS      384U        /* MeasureTask使用1536字节任务栈。 */
#define APP_INPUT_TASK_STACK_WORDS        256U        /* InputTask使用1024字节任务栈。 */
#define APP_CONTROL_TASK_STACK_WORDS      512U        /* ControlTask使用2048字节任务栈。 */
#define APP_INDICATOR_TASK_STACK_WORDS    256U        /* IndicatorTask使用1024字节任务栈。 */
#define APP_UI_TASK_STACK_WORDS          2048U        /* UiTask使用8192字节任务栈，满足整页LVGL重绘。 */

#define APP_EVENT_QUEUE_LENGTH             16U        /* 事件队列最多缓存16条采样或输入事件。 */
#define APP_UI_QUEUE_LENGTH                 1U        /* UI队列只保留最新的一份快照。 */
#define APP_INDICATOR_QUEUE_LENGTH          1U        /* 灯效队列只保留最新的一条灯效命令。 */

#define APP_DHT11_STARTUP_DELAY_MS       2000U        /* DHT11上电后等待2秒再进行第一次读取。 */
#define APP_DHT11_SAMPLE_PERIOD_MS       2000U        /* DHT11采样周期为2秒。 */
#define APP_MPU6050_SAMPLE_PERIOD_MS      200U        /* MPU6050采样周期为200毫秒。 */
#define APP_MPU6050_RETRY_PERIOD_MS      1000U        /* MPU6050失败后每1秒重新初始化一次。 */
#define APP_MPU6050_MOTION_DELTA_MG      1200         /* 加速度单轴变化超过1200mg视为明显运动。 */

#define APP_MEASURE_TASK_PERIOD_MS         20U        /* MeasureTask每20毫秒查询避障和非阻塞ECHO结果。 */
#define APP_SR04_SAMPLE_PERIOD_MS         400U        /* HC-SR04测距周期为400毫秒。 */
#define APP_INPUT_TASK_PERIOD_MS           20U        /* InputTask每20毫秒取一次已解码的遥控键。 */
#define APP_INDICATOR_BLINK_PERIOD_MS     300U        /* RGB报警灯每300毫秒切换一次亮灭。 */
#define APP_UI_LOOP_PERIOD_MS                5U        /* UiTask每5毫秒执行一次LVGL处理函数。 */

#endif                                               /* 结束APP_CONFIG_H头文件保护。 */
