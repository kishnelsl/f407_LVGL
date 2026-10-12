#ifndef F407_PERIPHERALS_H
#define F407_PERIPHERALS_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

extern SPI_HandleTypeDef hspi3;
extern I2C_HandleTypeDef hi2c3;

/* These two functions are intentionally separate from MX_GPIO_Init(). */
void MX_F407_SPI3_Init(void);
void MX_F407_I2C3_Init(void);

#ifdef __cplusplus
}
#endif
#endif
