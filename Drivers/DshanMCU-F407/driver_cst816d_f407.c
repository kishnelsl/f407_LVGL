#include "driver_cst816d_f407.h"
#include "driver_f407_pins.h"
#include "stm32f4xx_hal.h"
#include <stddef.h>
#include <string.h>

#define CST816D_REG_STATUS  0x00U
#define CST816D_REG_CHIP_ID 0xA7U
#define CST816D_REG_DIS_AUTO_SLEEP 0xFEU
#define CST816D_AUTO_SLEEP_DISABLED 0x01U
#define CST816D_I2C_TIMEOUT_MS 5U

static uint8_t s_last_raw[F407_CST816D_RAW_SIZE];

/**
 * @brief 初始化CST816D触摸控制器。
 * @return 函数处理结果或计算值。
 */
int F407_CST816D_Init(void)
{
    GPIO_InitTypeDef gpio = {0};
    uint8_t disable_auto_sleep = CST816D_AUTO_SLEEP_DISABLED;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    gpio.Pin = F407_CST816D_SCL_PIN;
    gpio.Mode = GPIO_MODE_AF_OD; gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH; gpio.Alternate = GPIO_AF4_I2C3;
    HAL_GPIO_Init(F407_CST816D_SCL_PORT, &gpio);
    gpio.Pin = F407_CST816D_SDA_PIN;
    HAL_GPIO_Init(F407_CST816D_SDA_PORT, &gpio);
    if (HAL_I2C_IsDeviceReady(&F407_CST816D_I2C, F407_CST816D_I2C_ADDR,
                              2U, 20U) != HAL_OK) {
        return -1;
    }

    (void)HAL_I2C_Mem_Write(&F407_CST816D_I2C, F407_CST816D_I2C_ADDR,
                            CST816D_REG_DIS_AUTO_SLEEP, I2C_MEMADD_SIZE_8BIT,
                            &disable_auto_sleep, 1U, CST816D_I2C_TIMEOUT_MS);
    return 0;
}

/**
 * @brief 读取CST816D芯片标识。
 * @param id 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_CST816D_GetID(uint8_t *id)
{
    if (id == NULL) return -1;
    return HAL_I2C_Mem_Read(&F407_CST816D_I2C, F407_CST816D_I2C_ADDR,
                            CST816D_REG_CHIP_ID, I2C_MEMADD_SIZE_8BIT,
                            id, 1U, CST816D_I2C_TIMEOUT_MS) == HAL_OK ? 0 : -1;
}

/**
 * @brief 读取一次CST816D触摸坐标。
 * @param point 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_CST816D_Read(F407_CST816D_Point_t *point)
{
    uint8_t start_register = CST816D_REG_STATUS;
    uint8_t data[F407_CST816D_RAW_SIZE];
    if (point == NULL) return -1;
    point->x = 0U;
    point->y = 0U;
    point->fingers = 0U;
    point->gesture = 0U;
    if (HAL_I2C_Master_Transmit(&F407_CST816D_I2C, F407_CST816D_I2C_ADDR,
                                &start_register, 1U, CST816D_I2C_TIMEOUT_MS) != HAL_OK) return -1;
    if (HAL_I2C_Master_Receive(&F407_CST816D_I2C, F407_CST816D_I2C_ADDR,
                               data, sizeof(data), CST816D_I2C_TIMEOUT_MS) != HAL_OK) return -1;
    memcpy(s_last_raw, data, sizeof(s_last_raw));
    point->gesture = data[1];
    point->x = (uint16_t)(((data[3] & 0x0FU) << 8) | data[4]);
    point->y = (uint16_t)(((data[5] & 0x0FU) << 8) | data[6]);

    point->fingers = (data[3] == 0x80U && data[4] > 1U) ? 1U : 0U;
    if (point->x >= 240U) point->x = 239U;
    if (point->y >= 296U) point->y = 295U;
    return point->fingers == 0U ? 1 : 0;
}

/**
 * @brief 获取最近一次触摸原始数据。
 * @param raw 函数输入或输出参数。
 * @return None
 */
void F407_CST816D_GetLastRaw(uint8_t raw[F407_CST816D_RAW_SIZE])
{
    if (raw != NULL) {
        memcpy(raw, s_last_raw, sizeof(s_last_raw));
    }
}
