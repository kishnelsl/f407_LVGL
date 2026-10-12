#include "driver_dht11_f407.h"
#include "driver_f407_pins.h"
#include "driver_timer_f407.h"
#include <stddef.h>

/**
 * @brief 执行F407_DHT11_Output相关功能。
 * @return None
 */
static void F407_DHT11_Output(void)
{
    GPIO_InitTypeDef init = {0};
    HAL_GPIO_WritePin(F407_DHT11_PORT, F407_DHT11_PIN, GPIO_PIN_SET);
    init.Pin = F407_DHT11_PIN; init.Mode = GPIO_MODE_OUTPUT_OD;
    init.Pull = GPIO_PULLUP; init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(F407_DHT11_PORT, &init);
}
/**
 * @brief 执行F407_DHT11_Input相关功能。
 * @return None
 */
static void F407_DHT11_Input(void)
{
    GPIO_InitTypeDef init = {0};
    init.Pin = F407_DHT11_PIN; init.Mode = GPIO_MODE_INPUT;
    init.Pull = GPIO_PULLUP; init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(F407_DHT11_PORT, &init);
}
static GPIO_PinState F407_DHT11_Pin(void)
{ return HAL_GPIO_ReadPin(F407_DHT11_PORT, F407_DHT11_PIN); }
/**
 * @brief 执行F407_DHT11_Wait相关功能。
 * @param state 函数输入或输出参数。
 * @param timeout 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
static int F407_DHT11_Wait(GPIO_PinState state, uint32_t timeout)
{
    while (timeout-- != 0U) {
        if (F407_DHT11_Pin() == state) return 0;
        F407_udelay(1);
    }
    return -1;
}

/**
 * @brief 初始化DHT11数据引脚。
 * @return None
 */
void F407_DHT11_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    F407_DHT11_Output();
}

/**
 * @brief 读取一次DHT11温湿度数据。
 * @param humidity 函数输入或输出参数。
 * @param temperature 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_DHT11_Read(int *humidity, int *temperature)
{
    uint8_t data[5] = {0};
    uint8_t i, bit;
    if (humidity == NULL || temperature == NULL) return F407_DHT11_INVALID_ARGUMENT;
    F407_DHT11_Output();
    HAL_GPIO_WritePin(F407_DHT11_PORT, F407_DHT11_PIN, GPIO_PIN_RESET);
    F407_mdelay(20);
    HAL_GPIO_WritePin(F407_DHT11_PORT, F407_DHT11_PIN, GPIO_PIN_SET);
    F407_udelay(30);
    F407_DHT11_Input();
    if (F407_DHT11_Wait(GPIO_PIN_RESET, 100) != 0) return F407_DHT11_NO_ACK;
    if (F407_DHT11_Wait(GPIO_PIN_SET, 100) != 0 || F407_DHT11_Wait(GPIO_PIN_RESET, 100) != 0)
        return F407_DHT11_TIMEOUT;
    for (i = 0; i < 5; ++i) {
        for (bit = 0; bit < 8; ++bit) {
            if (F407_DHT11_Wait(GPIO_PIN_SET, 70) != 0) return F407_DHT11_TIMEOUT;
            F407_udelay(40);
            data[i] <<= 1;
            if (F407_DHT11_Pin() == GPIO_PIN_SET) data[i] |= 1U;
            if (F407_DHT11_Wait(GPIO_PIN_RESET, 100) != 0) return F407_DHT11_TIMEOUT;
        }
    }
    F407_DHT11_Output();
    if ((uint8_t)(data[0] + data[1] + data[2] + data[3]) != data[4]) return F407_DHT11_CHECKSUM;
    *humidity = data[0]; *temperature = data[2];
    return F407_DHT11_OK;
}
