#include "app_runtime.h"

#include "app_config.h"
#include "app_tasks.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include <stddef.h>

static QueueHandle_t s_event_queue;          /* 保存所有采样和输入事件的队列句柄。 */
static QueueHandle_t s_ui_queue;             /* 保存ControlTask发往UiTask的最新快照。 */
static QueueHandle_t s_indicator_queue;      /* 保存ControlTask发往RGB任务的最新灯效。 */
static SemaphoreHandle_t s_timing_mutex;     /* 串行保护DHT11与HC-SR04的微秒级时序。 */

/**
 * @brief 将毫秒超时转换为FreeRTOS节拍。
 * @param timeout_ms 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
static TickType_t App_MillisecondsToTicks(
    uint32_t timeout_ms)                     /* timeout_ms是调用者给出的毫秒超时。 */
{
    TickType_t ticks;                        /* ticks用于保存转换后的系统节拍数。 */

    if(timeout_ms == APP_WAIT_FOREVER) {
        return portMAX_DELAY;
    }

    ticks = pdMS_TO_TICKS(timeout_ms);

    if(timeout_ms != 0U && ticks == 0U) {
        ticks = 1U;
    }

    return ticks;
}

/**
 * @brief 创建一个应用任务。
 * @param entry 函数输入或输出参数。
 * @param name 函数输入或输出参数。
 * @param stack_words 函数输入或输出参数。
 * @param priority 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
static int App_CreateTask(
    TaskFunction_t entry,                    /* entry是任务入口函数。 */
    const char *name,                        /* name是调试器中显示的任务名称。 */
    configSTACK_DEPTH_TYPE stack_words,
    UBaseType_t priority)                    /* priority是FreeRTOS任务优先级。 */
{
    return xTaskCreate(
               entry,
               name,
               stack_words,
               NULL,
               priority,
               NULL) == pdPASS
           ? 0
           : -1;
}

/**
 * @brief 创建应用队列、互斥锁和后台任务。
 * @return 函数处理结果或计算值。
 */
int App_RuntimeInit(void)
{
    s_event_queue = xQueueCreate(
        APP_EVENT_QUEUE_LENGTH,
        sizeof(AppEvent_t));
    s_ui_queue = xQueueCreate(
        APP_UI_QUEUE_LENGTH,
        sizeof(EnvSnapshot_t));
    s_indicator_queue = xQueueCreate(
        APP_INDICATOR_QUEUE_LENGTH,
        sizeof(EnvIndicatorCommand_t));
    s_timing_mutex = xSemaphoreCreateMutex();

    if(s_event_queue == NULL ||
       s_ui_queue == NULL ||
       s_indicator_queue == NULL ||
       s_timing_mutex == NULL) {
        return -1;
    }

    if(App_CreateTask(App_ControlTask,
                      "ControlTask",
                      APP_CONTROL_TASK_STACK_WORDS,
                      tskIDLE_PRIORITY + 3U) != 0) {
        return -1;
    }

    if(App_CreateTask(App_SensorTask,
                      "SensorTask",
                      APP_SENSOR_TASK_STACK_WORDS,
                      tskIDLE_PRIORITY + 3U) != 0) {
        return -1;
    }

    if(App_CreateTask(App_MeasureTask,
                      "MeasureTask",
                      APP_MEASURE_TASK_STACK_WORDS,
                      tskIDLE_PRIORITY + 2U) != 0) {
        return -1;
    }

    if(App_CreateTask(App_InputTask,
                      "InputTask",
                      APP_INPUT_TASK_STACK_WORDS,
                      tskIDLE_PRIORITY + 2U) != 0) {
        return -1;
    }

    if(App_CreateTask(App_IndicatorTask,
                      "IndicatorTask",
                      APP_INDICATOR_TASK_STACK_WORDS,
                      tskIDLE_PRIORITY + 1U) != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief 获取系统运行时间，单位为毫秒。
 * @return 函数处理结果或计算值。
 */
uint32_t App_TimeMs(void)
{
    TickType_t tick = xTaskGetTickCount();   /* 获取当前累计的FreeRTOS节拍。 */

    return (uint32_t)(
        ((uint64_t)tick * 1000U) /
        configTICK_RATE_HZ);
}

/**
 * @brief 向控制任务投递一条应用事件。
 * @param event 函数输入或输出参数。
 * @param timeout_ms 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int App_PostEvent(
    const AppEvent_t *event,                 /* event指向待发送的只读事件。 */
    uint32_t timeout_ms)                     /* timeout_ms指定队列满时等待多久。 */
{
    if(event == NULL ||
       s_event_queue == NULL) {
        return -1;
    }

    return xQueueSend(
               s_event_queue,
               event,
               App_MillisecondsToTicks(
                   timeout_ms)) == pdPASS
           ? 0
           : -1;
}

/**
 * @brief 等待并接收一条应用事件。
 * @param event 函数输入或输出参数。
 * @param timeout_ms 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int App_WaitEvent(
    AppEvent_t *event,                       /* event用于保存接收结果。 */
    uint32_t timeout_ms)                     /* timeout_ms指定最多等待多久。 */
{
    if(event == NULL ||
       s_event_queue == NULL) {
        return -1;
    }

    return xQueueReceive(
               s_event_queue,
               event,
               App_MillisecondsToTicks(
                   timeout_ms)) == pdPASS
           ? 0
           : -1;
}

/**
 * @brief 发布最新业务快照给界面任务。
 * @param snapshot 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int App_PublishSnapshot(
    const EnvSnapshot_t *snapshot)           /* snapshot指向只读业务快照。 */
{
    if(snapshot == NULL ||
       s_ui_queue == NULL) {
        return -1;
    }

    return xQueueOverwrite(
               s_ui_queue,
               snapshot) == pdPASS
           ? 0
           : -1;
}

/**
 * @brief 非阻塞获取最新业务快照。
 * @param snapshot 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int App_TakeLatestSnapshot(
    EnvSnapshot_t *snapshot)                 /* snapshot用于保存接收结果。 */
{
    if(snapshot == NULL ||
       s_ui_queue == NULL) {
        return -1;
    }

    return xQueueReceive(
               s_ui_queue,
               snapshot,
               0U) == pdPASS
           ? 0
           : -1;
}

/**
 * @brief 发布最新RGB灯效命令。
 * @param command 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int App_PublishIndicator(
    const EnvIndicatorCommand_t *command)    /* command指向只读的灯效命令。 */
{
    if(command == NULL ||
       s_indicator_queue == NULL) {
        return -1;
    }

    return xQueueOverwrite(
               s_indicator_queue,
               command) == pdPASS
           ? 0
           : -1;
}

/**
 * @brief 等待RGB灯效命令。
 * @param command 函数输入或输出参数。
 * @param timeout_ms 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int App_WaitIndicator(
    EnvIndicatorCommand_t *command,          /* command用于保存接收到的灯效。 */
    uint32_t timeout_ms)                     /* timeout_ms指定最多等待多久。 */
{
    if(command == NULL ||
       s_indicator_queue == NULL) {
        return -1;
    }

    return xQueueReceive(
               s_indicator_queue,
               command,
               App_MillisecondsToTicks(
                   timeout_ms)) == pdPASS
           ? 0
           : -1;
}

/**
 * @brief 获取微秒时序设备的互斥锁。
 * @param timeout_ms 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int App_TimingLock(uint32_t timeout_ms)
{
    if(s_timing_mutex == NULL) {
        return -1;
    }

    return xSemaphoreTake(
               s_timing_mutex,
               App_MillisecondsToTicks(
                   timeout_ms)) == pdTRUE
           ? 0
           : -1;
}

/**
 * @brief 释放微秒时序设备的互斥锁。
 * @return None
 */
void App_TimingUnlock(void)
{
    if(s_timing_mutex != NULL) {
        (void)xSemaphoreGive(s_timing_mutex);
    }
}
