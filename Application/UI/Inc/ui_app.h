#ifndef UI_APP_H                                    /* 防止本头文件被重复包含。 */
#define UI_APP_H                                    /* 定义本头文件的保护宏。 */

#include "environment_types.h"                     /* 引入界面需要显示的EnvSnapshot_t类型。 */

#ifdef __cplusplus                                  /* 如果当前文件由C++编译器编译。 */
extern "C" {                                        /* 让下面的函数使用C语言链接规则。 */
#endif                                              /* 结束C++编译器判断。 */

void UI_AppCreate(void);                            /* 创建温湿度页面中的所有LVGL控件。 */
void UI_AppUpdate(const EnvSnapshot_t *snapshot);   /* 根据最新环境快照更新页面文字和颜色。 */
void UI_AppProcessNavigation(void);                 /* 在LVGL事件处理结束后执行按钮请求的切页。 */

#ifdef __cplusplus                                  /* 如果当前文件由C++编译器编译。 */
}                                                   /* 结束extern "C"代码区域。 */
#endif                                              /* 结束C++编译器判断。 */

#endif                                              /* 结束UI_APP_H头文件保护。 */
