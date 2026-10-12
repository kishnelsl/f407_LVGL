#include "app_tasks.h"

#include "app_messages.h"
#include "app_runtime.h"
#include "environment_service.h"

#include <stddef.h>

static bool s_has_last_indicator;                /* 标记是否已经向RGB任务发布过灯效。 */
static EnvIndicatorCommand_t s_last_indicator;   /* 保存上一次真正发布的RGB灯效。 */

/**
 * @brief 发布当前业务快照及变化后的RGB灯效。
 * @param service 函数输入或输出参数。
 * @return None
 */
static void App_ControlPublish(
    const EnvironmentService_t *service)         /* service是已经更新完成的业务服务。 */
{
    const EnvSnapshot_t *snapshot =              /* 获取业务服务内部的最新只读快照。 */
        EnvironmentService_GetSnapshot(service);

    if(snapshot != NULL) {
        (void)App_PublishSnapshot(snapshot);
        if(!s_has_last_indicator ||
           s_last_indicator.color_rgb888 !=
           snapshot->indicator.color_rgb888 ||
           s_last_indicator.blink !=
           snapshot->indicator.blink) {
            (void)App_PublishIndicator(
                &snapshot->indicator);
            s_last_indicator = snapshot->indicator;
            s_has_last_indicator = true;
        }
    }
}

/**
 * @brief 处理应用事件、更新业务状态并发布UI和灯效。
 * @param argument 函数输入或输出参数。
 * @return None
 */
void App_ControlTask(void *argument)
{
    EnvironmentService_t service;                /* 仅ControlTask拥有并修改该业务对象。 */

    (void)argument;

    EnvironmentService_Init(&service);
    App_ControlPublish(&service);

    for(;;) {
        AppEvent_t event;                        /* event用于保存一条收到的应用事件。 */

        if(App_WaitEvent(&event,
                         APP_WAIT_FOREVER) != 0) {
            continue;
        }

        switch(event.type) {
        case APP_EVENT_DHT11_SAMPLE:
            EnvironmentService_ApplyEnvironmentSample(
                &service,
                &event.data.environment);
            break;
        case APP_EVENT_MPU6050_SAMPLE:
            EnvironmentService_ApplyMotionSample(
                &service,
                &event.data.motion);
            break;
        case APP_EVENT_DISTANCE_SAMPLE:
            EnvironmentService_ApplyDistanceSample(
                &service,
                &event.data.distance);
            break;
        case APP_EVENT_OBSTACLE_SAMPLE:
            EnvironmentService_ApplyObstacleSample(
                &service,
                &event.data.obstacle);
            break;
        case APP_EVENT_IR_KEY:
            EnvironmentService_ApplyIrKey(
                &service,
                &event.data.ir_key);
            break;
        default:
            break;
        }

        App_ControlPublish(&service);
    }
}
