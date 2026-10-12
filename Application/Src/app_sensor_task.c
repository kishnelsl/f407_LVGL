#include "app_tasks.h"

#include "app_config.h"
#include "app_messages.h"
#include "app_runtime.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver_dht11_f407.h"
#include "driver_mpu6050_f407.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief 将整数限制在int16_t的有效范围内。
 * @param value 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
static int16_t App_ClampInt16(int32_t value)
{
    if(value > INT16_MAX) {
        return INT16_MAX;
    }
    if(value < INT16_MIN) {
        return INT16_MIN;
    }
    return (int16_t)value;
}

/**
 * @brief 周期采集DHT11和MPU6050数据并投递事件。
 * @param argument 函数输入或输出参数。
 * @return None
 */
void App_SensorTask(void *argument)
{
    TickType_t next_dht_wake;                    /* 保存DHT11固定周期的基准时刻。 */
    TickType_t next_mpu_wake;                    /* 保存MPU6050固定周期的基准时刻。 */
    int16_t previous_ax = 0;                      /* 保存上一次X轴加速度，用于运动判断。 */
    int16_t previous_ay = 0;                      /* 保存上一次Y轴加速度，用于运动判断。 */
    int16_t previous_az = 0;                      /* 保存上一次Z轴加速度，用于运动判断。 */
    bool has_previous_accel = false;             /* 标记是否已经保存过一份加速度基准。 */
    bool mpu_initialized = false;                /* 标记MPU6050是否已经初始化成功。 */
    uint32_t last_mpu_init_attempt_ms = 0U;      /* 记录最近一次MPU6050初始化尝试的时间。 */

    (void)argument;

    F407_DHT11_Init();
    next_dht_wake = xTaskGetTickCount();
    next_mpu_wake = next_dht_wake;

    vTaskDelay(pdMS_TO_TICKS(
        APP_DHT11_STARTUP_DELAY_MS));

    next_dht_wake = xTaskGetTickCount();
    next_mpu_wake = next_dht_wake;

    for(;;) {
        TickType_t now = xTaskGetTickCount();    /* 获取当前FreeRTOS节拍。 */

        if((int32_t)(now - next_dht_wake) >= 0) {
            AppEvent_t event;                    /* event用于保存即将发送的DHT11事件。 */
            int humidity = 0;                    /* humidity用于接收DHT11整数湿度。 */
            int temperature = 0;                 /* temperature用于接收DHT11整数温度。 */
            int result = F407_DHT11_TIMEOUT;     /* result默认保存一个明确的失败状态码。 */

            memset(&event, 0, sizeof(event));
            event.type = APP_EVENT_DHT11_SAMPLE;
            event.data.environment.timestamp_ms = App_TimeMs();
            if(App_TimingLock(APP_WAIT_FOREVER) == 0) {
                UBaseType_t old_priority = uxTaskPriorityGet(NULL); /* 保存SensorTask原有优先级。 */

                vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 4U);
                result = F407_DHT11_Read(
                    &humidity,
                    &temperature);
                vTaskPrioritySet(NULL, old_priority);
                App_TimingUnlock();
            }
            event.data.environment.read_status = result;

            if(result == F407_DHT11_OK) {
                event.data.environment.temperature_x10 =
                    (int16_t)(temperature * 10);
                event.data.environment.humidity_x10 =
                    (uint16_t)(humidity * 10);
                event.data.environment.valid = true;
            }
            else {
                event.data.environment.valid = false;
            }

            (void)App_PostEvent(&event, 20U);
            next_dht_wake += pdMS_TO_TICKS(
                APP_DHT11_SAMPLE_PERIOD_MS);
        }

        if((int32_t)(now - next_mpu_wake) >= 0) {
            AppEvent_t event;                    /* event用于保存即将发送的MPU6050事件。 */
            int16_t ax = 0;                      /* 保存MPU6050原始X轴加速度。 */
            int16_t ay = 0;                      /* 保存MPU6050原始Y轴加速度。 */
            int16_t az = 0;                      /* 保存MPU6050原始Z轴加速度。 */
            int16_t gx = 0;                      /* 保存MPU6050原始X轴角速度。 */
            int16_t gy = 0;                      /* 保存MPU6050原始Y轴角速度。 */
            int16_t gz = 0;                      /* 保存MPU6050原始Z轴角速度。 */
            int result = -1;                     /* 默认将本次MPU6050读取标记为失败。 */

            if(!mpu_initialized &&
               App_TimeMs() - last_mpu_init_attempt_ms >=
               APP_MPU6050_RETRY_PERIOD_MS) {
                last_mpu_init_attempt_ms = App_TimeMs();
                mpu_initialized =
                    F407_MPU6050_Init() == 0;
            }

            memset(&event, 0, sizeof(event));
            event.type = APP_EVENT_MPU6050_SAMPLE;
            event.data.motion.timestamp_ms = App_TimeMs();

            if(mpu_initialized) {
                result = F407_MPU6050_ReadData(
                    &ax, &ay, &az,
                    &gx, &gy, &gz);
            }

            event.data.motion.read_status = result;

            if(result == 0) {
                event.data.motion.accel_x_mg =
                    App_ClampInt16(((int32_t)ax * 1000) / 2048);
                event.data.motion.accel_y_mg =
                    App_ClampInt16(((int32_t)ay * 1000) / 2048);
                event.data.motion.accel_z_mg =
                    App_ClampInt16(((int32_t)az * 1000) / 2048);
                event.data.motion.gyro_x_dps =
                    (int16_t)((int32_t)gx / 16);
                event.data.motion.gyro_y_dps =
                    (int16_t)((int32_t)gy / 16);
                event.data.motion.gyro_z_dps =
                    (int16_t)((int32_t)gz / 16);
                event.data.motion.motion_detected =
                    has_previous_accel &&
                    (abs((int)event.data.motion.accel_x_mg - previous_ax) > APP_MPU6050_MOTION_DELTA_MG ||
                     abs((int)event.data.motion.accel_y_mg - previous_ay) > APP_MPU6050_MOTION_DELTA_MG ||
                     abs((int)event.data.motion.accel_z_mg - previous_az) > APP_MPU6050_MOTION_DELTA_MG);
                previous_ax = event.data.motion.accel_x_mg;
                previous_ay = event.data.motion.accel_y_mg;
                previous_az = event.data.motion.accel_z_mg;
                has_previous_accel = true;
                event.data.motion.valid = true;
            }
            else {
                mpu_initialized = false;
                has_previous_accel = false;
                event.data.motion.valid = false;
            }

            (void)App_PostEvent(&event, 20U);
            next_mpu_wake += pdMS_TO_TICKS(
                APP_MPU6050_SAMPLE_PERIOD_MS);
        }

        vTaskDelay(pdMS_TO_TICKS(10U));
    }
}
