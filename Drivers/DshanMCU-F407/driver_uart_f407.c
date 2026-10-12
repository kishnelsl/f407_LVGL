#include "driver_uart_f407.h"
#include "driver_f407_pins.h"

void F407_UART_Init(void) { }

/**
 * @brief 执行F407_UART_PutChar相关功能。
 * @param c 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_UART_PutChar(int c)
{
    uint8_t ch = (uint8_t)c;
    return HAL_UART_Transmit(&huart1, &ch, 1, 500U) == HAL_OK ? c : EOF;
}

/**
 * @brief 执行F407_UART_GetChar相关功能。
 * @return 函数处理结果或计算值。
 */
int F407_UART_GetChar(void)
{
    uint8_t ch = 0;
    return HAL_UART_Receive(&huart1, &ch, 1, HAL_MAX_DELAY) == HAL_OK ? ch : EOF;
}
