#include "driver_led_f407.h"
#include "driver_f407_pins.h"

/**
 * @brief 执行F407_Led_Init相关功能。
 * @return 函数处理结果或计算值。
 */
int F407_Led_Init(void)
{
    GPIO_InitTypeDef init = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_WritePin(F407_LED_PORT, F407_LED_PIN, GPIO_PIN_SET);
    init.Pin = F407_LED_PIN;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(F407_LED_PORT, &init);
    return 0;
}

/**
 * @brief 执行F407_Led_Control相关功能。
 * @param on 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_Led_Control(int on)
{
    HAL_GPIO_WritePin(F407_LED_PORT, F407_LED_PIN, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
    return 0;
}

/**
 * @brief 执行F407_Led_Test相关功能。
 * @return None
 */
void F407_Led_Test(void)
{
    uint32_t i;
    F407_Led_Init();
    for (i = 0U; i < 3U; ++i) {
        F407_Led_Control(1); HAL_Delay(500);
        F407_Led_Control(0); HAL_Delay(500);
    }
}
