#ifndef APP_TASKS_H                         /* 防止本头文件被重复包含。 */
#define APP_TASKS_H                         /* 定义本头文件的保护宏。 */

#ifdef __cplusplus                          /* 如果本头文件被C++源文件包含。 */
extern "C" {                                /* 让下面的任务函数使用C语言链接规则。 */
#endif                                      /* 结束C++编译器判断。 */

void App_SensorTask(void *argument);        /* 周期读取DHT11和MPU6050。 */
void App_MeasureTask(void *argument);       /* 周期读取HC-SR04。 */
void App_InputTask(void *argument);         /* 读取红外接收驱动已解码的遥控键。 */
void App_ControlTask(void *argument);       /* 独占处理业务状态、报警和快照发布。 */
void App_IndicatorTask(void *argument);     /* 独占驱动RGB报警灯的颜色和闪烁。 */

#ifdef __cplusplus                          /* 如果本头文件被C++源文件包含。 */
}                                           /* 结束extern "C"代码块。 */
#endif                                      /* 结束C++编译器判断。 */

#endif                                      /* 结束APP_TASKS_H头文件保护。 */
