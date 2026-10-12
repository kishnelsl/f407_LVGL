#include "driver_ir_obstacle_f407.h"
#include "driver_f407_pins.h"

/**
 * @brief 执行F407_IRObstacle_Init相关功能。
 * @return None
 */
void F407_IRObstacle_Init(void)
{
    GPIO_InitTypeDef init = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    init.Pin = F407_IR_OBSTACLE_PIN; init.Mode = GPIO_MODE_INPUT;
    init.Pull = GPIO_PULLUP; init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(F407_IR_OBSTACLE_PORT, &init);
}

/**
 * @brief 执行F407_IRObstacle_Read相关功能。
 * @return 函数处理结果或计算值。
 */
int F407_IRObstacle_Read(void)
{
    return HAL_GPIO_ReadPin(F407_IR_OBSTACLE_PORT, F407_IR_OBSTACLE_PIN) == GPIO_PIN_RESET ? 0 : 1;
}
