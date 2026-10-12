#include "driver_timer_f407.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief 执行F407_Timer_EnableDwt相关功能。
 * @return None
 */
static void F407_Timer_EnableDwt(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0U) {
        DWT->CYCCNT = 0U;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

/**
 * @brief 执行F407_Timer_Cycles相关功能。
 * @return 函数处理结果或计算值。
 */
uint32_t F407_Timer_Cycles(void)
{
    F407_Timer_EnableDwt();
    return DWT->CYCCNT;
}

/**
 * @brief 执行F407_Timer_CyclesPerUs相关功能。
 * @return 函数处理结果或计算值。
 */
uint32_t F407_Timer_CyclesPerUs(void)
{
    uint32_t value = SystemCoreClock / 1000000U;
    return value == 0U ? 1U : value;
}

/**
 * @brief 执行F407_udelay相关功能。
 * @param us 函数输入或输出参数。
 * @return None
 */
void F407_udelay(uint32_t us)
{
    uint32_t start = F407_Timer_Cycles();
    uint32_t cycles = us * F407_Timer_CyclesPerUs();
    while ((uint32_t)(F407_Timer_Cycles() - start) < cycles) { }
}

/**
 * @brief 执行F407_mdelay相关功能。
 * @param ms 函数输入或输出参数。
 * @return None
 */
void F407_mdelay(uint32_t ms)
{
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        vTaskDelay(pdMS_TO_TICKS(ms));
    }
    else {
        HAL_Delay(ms);
    }
}

/**
 * @brief 执行F407_Timer_Ns相关功能。
 * @return 函数处理结果或计算值。
 */
uint64_t F407_Timer_Ns(void)
{
    return ((uint64_t)F407_Timer_Cycles() * 1000000000ULL) /
           (SystemCoreClock == 0U ? 1U : SystemCoreClock);
}
