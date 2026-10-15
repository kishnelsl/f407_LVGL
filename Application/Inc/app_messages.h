#ifndef APP_MESSAGES_H                               /* 防止本头文件被重复包含。 */
#define APP_MESSAGES_H                               /* 定义本头文件的保护宏。 */

#include "environment_types.h"                      /* 引入所有任务之间传递的业务数据类型。 */

#ifdef __cplusplus                                  /* 如果本头文件被C++源文件包含。 */
extern "C" {                                        /* 让下面的声明使用C语言链接规则。 */
#endif                                              /* 结束C++编译器判断。 */

typedef enum {                                      /* 定义应用事件的类别。 */
    APP_EVENT_DHT11_SAMPLE = 0,                     /* 表示SensorTask完成一次DHT11读取。 */
    APP_EVENT_MPU6050_SAMPLE,                       /* 表示SensorTask完成一次MPU6050读取。 */
    APP_EVENT_DISTANCE_SAMPLE,                      /* 表示MeasureTask完成一次HC-SR04测距。 */
    APP_EVENT_IR_KEY                                /* 表示InputTask读到一颗红外遥控按键。 */
} AppEventType_t;                                   /* 将枚举类型命名为AppEventType_t。 */

typedef struct {                                    /* 定义放入事件队列的统一消息结构体。 */
    AppEventType_t type;                            /* type说明当前联合体中的哪一项有效。 */
    union {                                         /* 联合体让不同事件共享同一块消息存储空间。 */
        EnvSample_t environment;                    /* DHT11事件使用这一项。 */
        EnvMotionSample_t motion;                   /* MPU6050事件使用这一项。 */
        EnvDistanceSample_t distance;               /* HC-SR04事件使用这一项。 */
        EnvIrKey_t ir_key;                          /* 红外遥控事件使用这一项。 */
    } data;                                         /* 将联合体成员命名为data。 */
} AppEvent_t;                                       /* 将整个事件结构体类型命名为AppEvent_t。 */

#ifdef __cplusplus                                  /* 如果本头文件被C++源文件包含。 */
}                                                   /* 结束extern "C"代码块。 */
#endif                                              /* 结束C++编译器判断。 */

#endif                                              /* 结束APP_MESSAGES_H头文件保护。 */
