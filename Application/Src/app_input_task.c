#include "app_tasks.h"

#include "app_config.h"
#include "app_messages.h"
#include "app_runtime.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver_ir_receiver_f407.h"

#include <string.h>

/**
 * @brief 读取已解码的红外按键并投递输入事件。
 * @param argument 函数输入或输出参数。
 * @return None
 */
void App_InputTask(void *argument)
{
    TickType_t last_wake_time;                   /* 保存20毫秒固定轮询周期的基准。 */

    (void)argument;

    F407_IRReceiver_Init();
    last_wake_time = xTaskGetTickCount();

    for(;;) {
        uint8_t device;                          /* 保存NEC协议设备地址。 */
        uint8_t command;                         /* 保存NEC协议按键命令。 */

        while(F407_IRReceiver_Read(
                  &device,
                  &command) == 0) {
            AppEvent_t event;                    /* event用于保存即将发送的遥控事件。 */

            memset(&event, 0, sizeof(event));
            event.type = APP_EVENT_IR_KEY;
            event.data.ir_key.timestamp_ms = App_TimeMs();
            event.data.ir_key.device = device;
            event.data.ir_key.command = command;
            event.data.ir_key.is_repeat =
                device == 0U && command == 0U;
            (void)App_PostEvent(&event, 0U);
        }

        vTaskDelayUntil(&last_wake_time,
                        pdMS_TO_TICKS(
                            APP_INPUT_TASK_PERIOD_MS));
    }
}
