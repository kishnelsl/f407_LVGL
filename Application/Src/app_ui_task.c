#include "app_runtime.h"

#include "app_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "f407_lvgl.h"
#include "ui_app.h"

/**
 * @brief 初始化并运行唯一的LVGL界面任务。
 * @return None
 */
void App_UiTaskRun(void)
{
    EnvSnapshot_t snapshot;                  /* snapshot保存ControlTask发布的最新快照。 */

    if(F407_LVGL_Init() != 0) {
        for(;;) {
            vTaskDelay(
                pdMS_TO_TICKS(1000U));
        }
    }

    UI_AppCreate();

    for(;;) {
        if(App_TakeLatestSnapshot(
               &snapshot) == 0) {
            UI_AppUpdate(&snapshot);
        }

        F407_LVGL_Loop();
        UI_AppProcessNavigation();

        vTaskDelay(
            pdMS_TO_TICKS(
                APP_UI_LOOP_PERIOD_MS));
    }
}
