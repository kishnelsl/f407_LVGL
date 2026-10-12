#include "driver_ir_receiver_f407.h"
#include "driver_f407_pins.h"
#include "driver_timer_f407.h"
#include <stddef.h>

#define F407_IR_BUF_LEN 16U
typedef struct { uint8_t device; uint8_t command; } F407_IR_Key_t;
static volatile F407_IR_Key_t g_queue[F407_IR_BUF_LEN];
static volatile uint8_t g_read, g_write;
static uint32_t g_edges[68];
static uint8_t g_count;
static volatile F407_IRReceiver_Diagnostics_t g_diag;

/**
 * @brief 执行F407_IR_InRange相关功能。
 * @param cycles 函数输入或输出参数。
 * @param low_us 函数输入或输出参数。
 * @param high_us 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
static uint8_t F407_IR_InRange(uint32_t cycles, uint32_t low_us, uint32_t high_us)
{
    uint32_t cpu = F407_Timer_CyclesPerUs();
    return cycles >= low_us * cpu && cycles <= high_us * cpu;
}
/**
 * @brief 执行F407_IR_Push相关功能。
 * @param device 函数输入或输出参数。
 * @param command 函数输入或输出参数。
 * @return None
 */
static void F407_IR_Push(uint8_t device, uint8_t command)
{
    uint8_t next = (uint8_t)((g_write + 1U) % F407_IR_BUF_LEN);
    if (next != g_read) { g_queue[g_write].device = device; g_queue[g_write].command = command; g_write = next; }
}
/**
 * @brief 执行F407_IR_Parse相关功能。
 * @return 函数处理结果或计算值。
 */
static int F407_IR_Parse(void)
{
    uint8_t data[4] = {0}, i;
    if (!F407_IR_InRange(g_edges[1] - g_edges[0], 8000, 10000) || !F407_IR_InRange(g_edges[2] - g_edges[1], 3500, 5500)) return -1;
    for (i = 0; i < 32; ++i) {
        uint32_t width = g_edges[4 + i * 2] - g_edges[3 + i * 2];
        if (!F407_IR_InRange(width, 300, 900) && !F407_IR_InRange(width, 1200, 2200)) return -1;
        data[i / 8] = (uint8_t)(data[i / 8] << 1);
        if (width > 1000U * F407_Timer_CyclesPerUs()) data[i / 8] |= 1U;
    }
    if ((uint8_t)(data[0] ^ data[1]) != 0xFFU || (uint8_t)(data[2] ^ data[3]) != 0xFFU) return -1;
    F407_IR_Push(data[0], data[2]);
    g_diag.last_device = data[0]; g_diag.last_command = data[2];
    return 0;
}

/**
 * @brief 执行F407_IRReceiver_Init相关功能。
 * @return None
 */
void F407_IRReceiver_Init(void)
{
    GPIO_InitTypeDef init = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    g_read = g_write = g_count = 0U;
    g_diag.edge_count = g_diag.frame_ok_count = g_diag.frame_error_count = 0U;
    g_diag.last_device = g_diag.last_command = 0U;
    init.Pin = F407_IR_RECEIVER_PIN; init.Mode = GPIO_MODE_IT_RISING_FALLING;
    init.Pull = GPIO_PULLUP; init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(F407_IR_RECEIVER_PORT, &init);
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
 * @brief 执行F407_IRReceiver_EXTI_Callback相关功能。
 * @param gpio_pin 函数输入或输出参数。
 * @return None
 */
void F407_IRReceiver_EXTI_Callback(uint16_t gpio_pin)
{
    static uint32_t previous;
    uint32_t now, idle;
    if (gpio_pin != F407_IR_RECEIVER_PIN) return;
    now = F407_Timer_Cycles(); idle = 100000U * F407_Timer_CyclesPerUs();
    g_diag.edge_count++;
    if ((uint32_t)(now - previous) > idle) g_count = 0U;
    previous = now;
    if (g_count >= 68U) g_count = 0U;
    g_edges[g_count++] = now;
    if (g_count == 4U && F407_IR_InRange(g_edges[1] - g_edges[0], 8000, 10000) && F407_IR_InRange(g_edges[2] - g_edges[1], 1900, 2700)) {
        F407_IR_Push(0U, 0U); g_diag.frame_ok_count++; g_count = 0U;
    } else if (g_count == 68U) {
        if (F407_IR_Parse() == 0) g_diag.frame_ok_count++; else g_diag.frame_error_count++;
        g_count = 0U;
    }
}

/**
 * @brief 执行F407_IRReceiver_Read相关功能。
 * @param device 函数输入或输出参数。
 * @param command 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_IRReceiver_Read(uint8_t *device, uint8_t *command)
{
    if (device == NULL || command == NULL || g_read == g_write) return -1;
    *device = g_queue[g_read].device; *command = g_queue[g_read].command;
    g_read = (uint8_t)((g_read + 1U) % F407_IR_BUF_LEN);
    return 0;
}

/**
 * @brief 执行F407_IRReceiver_GetDiagnostics相关功能。
 * @param diagnostics 函数输入或输出参数。
 * @return None
 */
void F407_IRReceiver_GetDiagnostics(F407_IRReceiver_Diagnostics_t *diagnostics)
{
    uint32_t primask;
    if (diagnostics == NULL) return;
    primask = __get_PRIMASK();
    __disable_irq(); *diagnostics = g_diag;
    diagnostics->pin_level = HAL_GPIO_ReadPin(F407_IR_RECEIVER_PORT, F407_IR_RECEIVER_PIN) == GPIO_PIN_SET ? 1U : 0U;
    __set_PRIMASK(primask);
}
