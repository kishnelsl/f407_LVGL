#include "app_tasks.h"

#include "app_config.h"
#include "app_runtime.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver_color_led_f407.h"

/**
 * @brief 独占执行RGB灯的颜色和闪烁效果。
 * @param argument 函数输入或输出参数。
 * @return None
 */
void App_IndicatorTask(void *argument)
{
    EnvIndicatorCommand_t command = {0U, false}; /* 保存当前生效的RGB灯效命令。 */
    bool light_on = true;                         /* 保存闪烁灯当前是亮还是灭。 */

    (void)argument;

    F407_ColorLED_Init();
    F407_ColorLED_Set(0x2490FFU);

    for(;;) {
        if(App_WaitIndicator(
               &command,
               APP_INDICATOR_BLINK_PERIOD_MS) == 0) {
            light_on = true;
        }
        else if(command.blink) {
            light_on = !light_on;
        }

        if(!command.blink || light_on) {
            F407_ColorLED_Set(command.color_rgb888);
        }
        else {
            F407_ColorLED_Set(0U);
        }
    }
}
