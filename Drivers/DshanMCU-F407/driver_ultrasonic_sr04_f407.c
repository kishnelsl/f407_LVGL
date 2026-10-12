#include "driver_ultrasonic_sr04_f407.h"
#include "driver_f407_pins.h"
#include "driver_timer_f407.h"

typedef enum {
    F407_SR04_IDLE = 0,
    F407_SR04_WAIT_RISE,
    F407_SR04_WAIT_FALL,
    F407_SR04_COMPLETE
} F407_SR04_State_t;

#define F407_SR04_TIMEOUT_US 30000U
#define F407_SR04_MIN_ECHO_US   100U

static volatile F407_SR04_State_t s_state;         /* 保存中断和任务共享的测距状态。 */
static volatile uint32_t s_trigger_cycles;         /* 保存发送TRIG时的DWT计数。 */
static volatile uint32_t s_rise_cycles;            /* 保存ECHO上升沿的DWT计数。 */
static volatile uint32_t s_echo_width_cycles;      /* 保存ECHO高电平持续的周期数。 */

/**
 * @brief 初始化HC-SR04触发和回波中断引脚。
 * @return None
 */
void F407_SR04_Init(void)
{
    GPIO_InitTypeDef init = {0};                   /* 创建并清零HAL GPIO配置结构体。 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    init.Pin = F407_SR04_TRIG_PIN;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(F407_SR04_TRIG_PORT, &init);
    HAL_GPIO_WritePin(F407_SR04_TRIG_PORT,
                      F407_SR04_TRIG_PIN,
                      GPIO_PIN_RESET);

    init.Pin = F407_SR04_ECHO_PIN;
    init.Mode = GPIO_MODE_IT_RISING_FALLING;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(F407_SR04_ECHO_PORT, &init);
    HAL_NVIC_SetPriority(EXTI1_IRQn, 5U, 0U);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    s_state = F407_SR04_IDLE;
}

/**
 * @brief 触发一次非阻塞超声波测距。
 * @return 函数处理结果或计算值。
 */
int F407_SR04_Start(void)
{
    uint32_t primask;                              /* 保存进入短临界区前的中断状态。 */

    if(s_state == F407_SR04_WAIT_RISE ||
       s_state == F407_SR04_WAIT_FALL) {
        return 1;
    }

    primask = __get_PRIMASK();
    __disable_irq();
    s_echo_width_cycles = 0U;
    s_trigger_cycles = F407_Timer_Cycles();
    s_state = F407_SR04_WAIT_RISE;
    __set_PRIMASK(primask);

    HAL_GPIO_WritePin(F407_SR04_TRIG_PORT,
                      F407_SR04_TRIG_PIN,
                      GPIO_PIN_SET);
    F407_udelay(10U);
    HAL_GPIO_WritePin(F407_SR04_TRIG_PORT,
                      F407_SR04_TRIG_PIN,
                      GPIO_PIN_RESET);
    return 0;
}

/**
 * @brief 查询当前超声波测距结果。
 * @param distance_cm 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_SR04_GetResult(uint32_t *distance_cm)
{
    uint32_t width_cycles;                         /* 保存安全复制出的回波宽度。 */
    uint32_t elapsed_cycles;                       /* 保存从触发至今经过的周期数。 */
    uint32_t cycles_per_us;                        /* 保存CPU每微秒的周期数。 */
    uint32_t primask;                              /* 保存短临界区前的中断状态。 */

    if(distance_cm == NULL) {
        return -2;
    }

    cycles_per_us = F407_Timer_CyclesPerUs();
    if(s_state == F407_SR04_COMPLETE) {
        primask = __get_PRIMASK();
        __disable_irq();
        width_cycles = s_echo_width_cycles;
        s_state = F407_SR04_IDLE;
        __set_PRIMASK(primask);
        *distance_cm =
            (width_cycles / cycles_per_us) / 58U;
        return 0;
    }

    if(s_state == F407_SR04_WAIT_RISE ||
       s_state == F407_SR04_WAIT_FALL) {
        elapsed_cycles =
            F407_Timer_Cycles() - s_trigger_cycles;
        if(elapsed_cycles >
           F407_SR04_TIMEOUT_US * cycles_per_us) {
            s_state = F407_SR04_IDLE;
            return -1;
        }
        return 1;
    }

    return -1;
}

/**
 * @brief 处理HC-SR04回波边沿中断。
 * @param gpio_pin 函数输入或输出参数。
 * @return None
 */
void F407_SR04_EXTI_Callback(uint16_t gpio_pin)
{
    uint32_t now;                                  /* 保存当前DWT周期计数。 */
    GPIO_PinState level;                           /* 保存ECHO边沿后的实际电平。 */

    if(gpio_pin != F407_SR04_ECHO_PIN) {
        return;
    }

    now = F407_Timer_Cycles();
    level = HAL_GPIO_ReadPin(F407_SR04_ECHO_PORT,
                             F407_SR04_ECHO_PIN);
    if(s_state == F407_SR04_WAIT_RISE &&
       level == GPIO_PIN_SET) {
        s_rise_cycles = now;
        s_state = F407_SR04_WAIT_FALL;
    }
    else if(s_state == F407_SR04_WAIT_FALL &&
            level == GPIO_PIN_RESET) {
        uint32_t width = now - s_rise_cycles;      /* 计算ECHO高电平宽度。 */

        if(width >= F407_SR04_MIN_ECHO_US *
                    F407_Timer_CyclesPerUs()) {
            s_echo_width_cycles = width;
            s_state = F407_SR04_COMPLETE;
        }
        else {
            s_state = F407_SR04_WAIT_RISE;
        }
    }
}

/**
 * @brief 兼容旧接口并查询超声波测距结果。
 * @param distance_cm 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_SR04_Read(uint32_t *distance_cm)
{
    return F407_SR04_GetResult(distance_cm);
}
