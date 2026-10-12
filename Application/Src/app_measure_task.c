#include "app_tasks.h"

#include "app_config.h"
#include "app_messages.h"
#include "app_runtime.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver_ir_obstacle_f407.h"
#include "driver_ultrasonic_sr04_f407.h"

#include <string.h>

/**
 * @brief 周期采集超声波距离和红外避障状态。
 * @param argument 函数输入或输出参数。
 * @return None
 */
void App_MeasureTask(void *argument)
{
    TickType_t last_wake_time;                   /* 保存20毫秒固定周期的基准。 */
    uint32_t elapsed_ms = 0U;                    /* 累计经过时间，用于每400毫秒测距一次。 */
    bool sr04_pending = false;                   /* 标记是否正在等待本轮ECHO中断结果。 */
    int last_obstacle = -1;                      /* 保存上次避障状态，-1表示还未读取。 */

    (void)argument;

    F407_IRObstacle_Init();
    F407_SR04_Init();
    last_wake_time = xTaskGetTickCount();

    for(;;) {
        int obstacle = F407_IRObstacle_Read();   /* 读取当前红外避障状态。 */

        if(obstacle != last_obstacle) {
            AppEvent_t event;                    /* event用于保存即将发送的避障事件。 */

            memset(&event, 0, sizeof(event));
            event.type = APP_EVENT_OBSTACLE_SAMPLE;
            event.data.obstacle.timestamp_ms = App_TimeMs();
            event.data.obstacle.obstacle_detected = obstacle == 0;
            event.data.obstacle.valid = true;
            (void)App_PostEvent(&event, 0U);
            last_obstacle = obstacle;
        }

        elapsed_ms += APP_MEASURE_TASK_PERIOD_MS;

        if(sr04_pending) {
            AppEvent_t event;                    /* event用于保存即将发送的距离事件。 */
            uint32_t distance_cm = 0U;           /* distance_cm用于接收测得的厘米距离。 */
            int result = F407_SR04_GetResult(    /* 查询中断是否已经捕获完整脉宽。 */
                &distance_cm);

            if(result != 1) {
                memset(&event, 0, sizeof(event));
                event.type = APP_EVENT_DISTANCE_SAMPLE;
                event.data.distance.timestamp_ms = App_TimeMs();
                event.data.distance.read_status = result;
                event.data.distance.distance_cm = distance_cm;
                event.data.distance.valid = result == 0;
                (void)App_PostEvent(&event, 20U);
                sr04_pending = false;
                App_TimingUnlock();
            }
        }

        if(!sr04_pending &&
           elapsed_ms >= APP_SR04_SAMPLE_PERIOD_MS) {
            if(App_TimingLock(APP_WAIT_FOREVER) == 0) {
                elapsed_ms = 0U;
                sr04_pending = F407_SR04_Start() == 0;
                if(!sr04_pending) {
                    App_TimingUnlock();
                }
            }
        }

        vTaskDelayUntil(&last_wake_time,
                        pdMS_TO_TICKS(
                            APP_MEASURE_TASK_PERIOD_MS));
    }
}
