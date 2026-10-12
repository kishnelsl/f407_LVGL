#ifndef ENVIRONMENT_TYPES_H                         /* 防止本头文件被重复包含。 */
#define ENVIRONMENT_TYPES_H                         /* 定义本头文件的保护宏。 */

#include <stdbool.h>                                /* 引入bool、true和false。 */
#include <stdint.h>                                 /* 引入uint32_t、int16_t等固定长度类型。 */

#ifdef __cplusplus                                  /* 如果本头文件被C++源文件包含。 */
extern "C" {                                        /* 让下面的声明使用C语言链接规则。 */
#endif                                              /* 结束C++编译器判断。 */

#define ENV_DISTANCE_ALARM_CM   25U                 /* 距离小于等于25cm时，布防状态进入报警。 */
#define ENV_DISTANCE_WARNING_CM 50U                 /* 距离小于等于50cm时，未布防状态显示预警。 */
#define ENV_IR_CMD_POWER        0x45U               /* 常见NEC小遥控器的POWER键命令码。 */
#define ENV_IR_CMD_PLAY_PAUSE   0x44U               /* 常见NEC小遥控器的PLAY/PAUSE键命令码。 */

typedef enum {                                      /* 定义系统整体状态的枚举类型。 */
    ENV_STATE_STARTUP = 0,                          /* 尚未获得DHT11和MPU6050的首次结果。 */
    ENV_STATE_NORMAL,                               /* 系统正常，且当前没有安全风险。 */
    ENV_STATE_WARNING,                              /* 发现风险但系统尚未布防，只进行预警。 */
    ENV_STATE_ALARM,                                /* 系统已布防且发现风险，报警被锁存。 */
    ENV_STATE_SENSOR_ERROR                          /* DHT11或MPU6050读取失败。 */
} EnvState_t;                                       /* 将枚举类型命名为EnvState_t。 */

typedef enum {                                      /* 定义报警的直接触发来源。 */
    ENV_ALERT_NONE = 0,                             /* 当前没有已锁存的报警原因。 */
    ENV_ALERT_OBSTACLE,                             /* 红外避障模块检测到近距离障碍。 */
    ENV_ALERT_DISTANCE,                             /* 超声波距离低于报警阈值。 */
    ENV_ALERT_MOTION                                /* MPU6050检测到明显运动。 */
} EnvAlertReason_t;                                 /* 将枚举类型命名为EnvAlertReason_t。 */

typedef struct {                                    /* 定义一次DHT11采样的结果。 */
    uint32_t timestamp_ms;                          /* 记录采样时刻，单位为毫秒。 */
    int16_t temperature_x10;                        /* 温度放大10倍保存，例如253表示25.3°C。 */
    uint16_t humidity_x10;                          /* 湿度放大10倍保存，例如605表示60.5%。 */
    int32_t read_status;                            /* 保存DHT11驱动返回的状态码。 */
    bool valid;                                     /* true表示本次温湿度数据有效。 */
} EnvSample_t;                                      /* 将结构体类型命名为EnvSample_t。 */

typedef struct {                                    /* 定义一次MPU6050采样的结果。 */
    uint32_t timestamp_ms;                          /* 记录采样时刻，单位为毫秒。 */
    int16_t accel_x_mg;                             /* X轴加速度，单位为毫g。 */
    int16_t accel_y_mg;                             /* Y轴加速度，单位为毫g。 */
    int16_t accel_z_mg;                             /* Z轴加速度，单位为毫g。 */
    int16_t gyro_x_dps;                             /* X轴角速度，单位为度每秒。 */
    int16_t gyro_y_dps;                             /* Y轴角速度，单位为度每秒。 */
    int16_t gyro_z_dps;                             /* Z轴角速度，单位为度每秒。 */
    int32_t read_status;                            /* 保存MPU6050驱动返回的状态码。 */
    bool motion_detected;                           /* true表示相邻两次加速度变化超过阈值。 */
    bool valid;                                     /* true表示本次MPU6050数据有效。 */
} EnvMotionSample_t;                                /* 将结构体类型命名为EnvMotionSample_t。 */

typedef struct {                                    /* 定义一次HC-SR04测距结果。 */
    uint32_t timestamp_ms;                          /* 记录测距时刻，单位为毫秒。 */
    uint32_t distance_cm;                           /* 保存测得的整数距离，单位为厘米。 */
    int32_t read_status;                            /* 保存HC-SR04驱动返回的状态码。 */
    bool valid;                                     /* true表示本次距离数据有效。 */
} EnvDistanceSample_t;                              /* 将结构体类型命名为EnvDistanceSample_t。 */

typedef struct {                                    /* 定义红外避障传感器的状态。 */
    uint32_t timestamp_ms;                          /* 记录读取避障状态的时刻。 */
    bool obstacle_detected;                         /* true表示模块检测到近距离障碍物。 */
    bool valid;                                     /* true表示已经至少读取过一次该模块。 */
} EnvObstacleSample_t;                              /* 将结构体类型命名为EnvObstacleSample_t。 */

typedef struct {                                    /* 定义一次红外遥控按键结果。 */
    uint32_t timestamp_ms;                          /* 记录解码出按键的时刻。 */
    uint8_t device;                                 /* 保存NEC协议中的设备地址。 */
    uint8_t command;                                /* 保存NEC协议中的按键命令码。 */
    bool is_repeat;                                 /* true表示这是遥控器长按时产生的重复帧。 */
} EnvIrKey_t;                                       /* 将结构体类型命名为EnvIrKey_t。 */

typedef struct {                                    /* 定义发给RGB灯任务的灯效命令。 */
    uint32_t color_rgb888;                          /* 使用0xRRGGBB格式保存RGB颜色。 */
    bool blink;                                     /* true表示任务应让该颜色闪烁。 */
} EnvIndicatorCommand_t;                            /* 将结构体类型命名为EnvIndicatorCommand_t。 */

typedef struct {                                    /* 定义ControlTask发给UiTask的完整只读快照。 */
    uint32_t version;                               /* 每处理一次事件递增，用于避免重复刷新界面。 */
    EnvState_t state;                               /* 保存当前系统整体状态。 */
    EnvAlertReason_t alert_reason;                  /* 保存报警锁存时的首个触发原因。 */
    bool armed;                                     /* true表示安全监测已布防。 */
    EnvSample_t environment;                        /* 保存最新DHT11温湿度结果。 */
    EnvMotionSample_t motion;                       /* 保存最新MPU6050运动结果。 */
    EnvDistanceSample_t distance;                   /* 保存最新HC-SR04测距结果。 */
    EnvObstacleSample_t obstacle;                   /* 保存最新红外避障状态。 */
    EnvIrKey_t ir_key;                              /* 保存最后一次红外遥控按键。 */
    EnvIndicatorCommand_t indicator;                /* 保存当前RGB灯应执行的效果。 */
} EnvSnapshot_t;                                    /* 将结构体类型命名为EnvSnapshot_t。 */

#ifdef __cplusplus                                  /* 如果本头文件被C++源文件包含。 */
}                                                   /* 结束extern "C"代码块。 */
#endif                                              /* 结束C++编译器判断。 */

#endif                                              /* 结束ENVIRONMENT_TYPES_H头文件保护。 */
