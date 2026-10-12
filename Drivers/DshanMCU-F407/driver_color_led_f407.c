#include "driver_color_led_f407.h"
#include "driver_f407_pins.h"

/**
 * @brief 初始化RGB灯PWM输出。
 * @return None
 */
void F407_ColorLED_Init(void)
{
    GPIO_InitTypeDef gpio = {0};
    TIM_OC_InitTypeDef channel = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOC, &gpio);
    channel.OCMode = TIM_OCMODE_PWM1;
    channel.OCPolarity = F407_RGB_ACTIVE_LOW ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH;
    channel.OCFastMode = TIM_OCFAST_DISABLE;
    channel.Pulse = 0;
    HAL_TIM_PWM_ConfigChannel(&htim3, &channel, F407_RGB_RED_CHANNEL);
    HAL_TIM_PWM_ConfigChannel(&htim3, &channel, F407_RGB_GREEN_CHANNEL);
    HAL_TIM_PWM_ConfigChannel(&htim3, &channel, F407_RGB_BLUE_CHANNEL);
    HAL_TIM_PWM_Start(&htim3, F407_RGB_RED_CHANNEL);
    HAL_TIM_PWM_Start(&htim3, F407_RGB_GREEN_CHANNEL);
    HAL_TIM_PWM_Start(&htim3, F407_RGB_BLUE_CHANNEL);
}

/**
 * @brief 设置RGB灯颜色。
 * @param color 函数输入或输出参数。
 * @return None
 */
void F407_ColorLED_Set(uint32_t color)
{
    uint32_t r = (color >> 16) & 0xffU;
    uint32_t g = (color >> 8) & 0xffU;
    uint32_t b = color & 0xffU;
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim3);
    uint32_t rp = r * period / 255U;
    uint32_t gp = g * period / 255U;
    uint32_t bp = b * period / 255U;
    __HAL_TIM_SET_COMPARE(&htim3, F407_RGB_RED_CHANNEL, rp);
    __HAL_TIM_SET_COMPARE(&htim3, F407_RGB_GREEN_CHANNEL, gp);
    __HAL_TIM_SET_COMPARE(&htim3, F407_RGB_BLUE_CHANNEL, bp);
}

/**
 * @brief 执行F407_ColorLED_Test相关功能。
 * @return None
 */
void F407_ColorLED_Test(void)
{
    F407_ColorLED_Init();
    F407_ColorLED_Set(0x00ff0000U); HAL_Delay(700);
    F407_ColorLED_Set(0x0000ff00U); HAL_Delay(700);
    F407_ColorLED_Set(0x000000ffU); HAL_Delay(700);
    F407_ColorLED_Set(0U); HAL_Delay(300);
}
