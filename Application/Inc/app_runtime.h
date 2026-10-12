#ifndef APP_RUNTIME_H                                      /* 防止本头文件被重复包含。 */
#define APP_RUNTIME_H                                      /* 定义本头文件的保护宏。 */

#include "app_messages.h"                                 /* 引入AppEvent_t和业务快照类型。 */

#include <stdint.h>                                        /* 引入uint32_t与UINT32_MAX。 */

#define APP_WAIT_FOREVER UINT32_MAX                        /* 使用最大值表示永久等待。 */

#ifdef __cplusplus                                         /* 如果本头文件被C++源文件包含。 */
extern "C" {                                               /* 让下面的函数使用C语言链接规则。 */
#endif                                                     /* 结束C++编译器判断。 */

int App_RuntimeInit(void);                                 /* 创建队列与所有后台应用任务。 */
void App_UiTaskRun(void);                                  /* 初始化并运行唯一的LVGL UiTask。 */
uint32_t App_TimeMs(void);                                 /* 返回FreeRTOS启动后的毫秒时间。 */
int App_PostEvent(const AppEvent_t *event, uint32_t timeout_ms); /* 向ControlTask发送应用事件。 */
int App_WaitEvent(AppEvent_t *event, uint32_t timeout_ms); /* 从事件队列接收应用事件。 */
int App_PublishSnapshot(const EnvSnapshot_t *snapshot);    /* 用最新业务快照覆盖UI队列旧数据。 */
int App_TakeLatestSnapshot(EnvSnapshot_t *snapshot);       /* 非阻塞地取出最新UI快照。 */
int App_PublishIndicator(const EnvIndicatorCommand_t *command); /* 用最新灯效覆盖RGB队列旧命令。 */
int App_WaitIndicator(EnvIndicatorCommand_t *command, uint32_t timeout_ms); /* 等待RGB灯效命令。 */
int App_TimingLock(uint32_t timeout_ms);                  /* 独占DHT11和HC-SR04共用的微秒时序窗口。 */
void App_TimingUnlock(void);                              /* 释放DHT11或HC-SR04占用的微秒时序窗口。 */

#ifdef __cplusplus                                         /* 如果本头文件被C++源文件包含。 */
}                                                          /* 结束extern "C"代码块。 */
#endif                                                     /* 结束C++编译器判断。 */

#endif                                                     /* 结束APP_RUNTIME_H头文件保护。 */
