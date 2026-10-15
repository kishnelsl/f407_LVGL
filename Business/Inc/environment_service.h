#ifndef ENVIRONMENT_SERVICE_H                         /* 防止本头文件被重复包含。 */
#define ENVIRONMENT_SERVICE_H                         /* 定义本头文件的保护宏。 */

#include "environment_types.h"                        /* 引入所有环境数据和状态类型。 */

#ifdef __cplusplus                                     /* 如果本头文件被C++源文件包含。 */
extern "C" {                                           /* 让下面的声明使用C语言链接规则。 */
#endif                                                 /* 结束C++编译器判断。 */

typedef struct {                                       /* 定义只由ControlTask拥有的业务服务对象。 */
    EnvSnapshot_t snapshot;                            /* 保存将要发布给UI和RGB任务的最新快照。 */
    bool has_environment_sample;                       /* 记录是否已经收到过DHT11采样。 */
    bool has_motion_sample;                            /* 记录是否已经收到过MPU6050采样。 */
    bool has_distance_sample;                          /* 记录是否已经收到过HC-SR04采样。 */
    bool alarm_latched;                                /* true表示报警已锁存，需遥控器解除。 */
} EnvironmentService_t;                                /* 将结构体类型命名为EnvironmentService_t。 */

void EnvironmentService_Init(EnvironmentService_t *service); /* 初始化业务服务为未布防启动状态。 */
void EnvironmentService_ApplyEnvironmentSample(       /* 应用一份DHT11采样结果。 */
    EnvironmentService_t *service,                     /* service是待修改的业务服务。 */
    const EnvSample_t *sample);                        /* sample是新的温湿度结果。 */
void EnvironmentService_ApplyMotionSample(             /* 应用一份MPU6050采样结果。 */
    EnvironmentService_t *service,                     /* service是待修改的业务服务。 */
    const EnvMotionSample_t *sample);                  /* sample是新的运动结果。 */
void EnvironmentService_ApplyDistanceSample(           /* 应用一份HC-SR04测距结果。 */
    EnvironmentService_t *service,                     /* service是待修改的业务服务。 */
    const EnvDistanceSample_t *sample);                /* sample是新的距离结果。 */
void EnvironmentService_ApplyIrKey(                    /* 应用一次红外遥控按键。 */
    EnvironmentService_t *service,                     /* service是待修改的业务服务。 */
    const EnvIrKey_t *key);                            /* key是解码得到的遥控按键。 */
const EnvSnapshot_t *EnvironmentService_GetSnapshot(   /* 取得当前只读业务快照。 */
    const EnvironmentService_t *service);              /* service是要读取的业务服务。 */

#ifdef __cplusplus                                     /* 如果本头文件被C++源文件包含。 */
}                                                      /* 结束extern "C"代码块。 */
#endif                                                 /* 结束C++编译器判断。 */

#endif                                                 /* 结束ENVIRONMENT_SERVICE_H头文件保护。 */
