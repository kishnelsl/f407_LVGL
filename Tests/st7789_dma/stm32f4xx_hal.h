#ifndef TEST_STM32F4XX_HAL_H
#define TEST_STM32F4XX_HAL_H

#include <stdint.h>

#define __weak __attribute__((weak))
#define DMA_SxCR_EN 1U
#define GPIO_PIN_0 (1U << 0)
#define GPIO_PIN_1 (1U << 1)
#define GPIO_PIN_8 (1U << 8)
#define GPIO_PIN_9 (1U << 9)
#define GPIO_PIN_10 (1U << 10)
#define GPIO_PIN_12 (1U << 12)
#define GPIO_MODE_AF_PP 1U
#define GPIO_MODE_OUTPUT_PP 2U
#define GPIO_NOPULL 0U
#define GPIO_SPEED_FREQ_VERY_HIGH 3U
#define GPIO_AF6_SPI3 6U
#define __HAL_RCC_GPIOC_CLK_ENABLE() ((void)0)

typedef enum { HAL_OK, HAL_ERROR, HAL_BUSY, HAL_TIMEOUT } HAL_StatusTypeDef;
typedef enum { GPIO_PIN_RESET, GPIO_PIN_SET } GPIO_PinState;
typedef enum { HAL_SPI_STATE_READY, HAL_SPI_STATE_BUSY_TX } HAL_SPI_StateTypeDef;
typedef struct { uint32_t CR; } DMA_Stream_TypeDef;
typedef struct { DMA_Stream_TypeDef *Instance; } DMA_HandleTypeDef;
typedef struct {
    DMA_HandleTypeDef *hdmatx;
    HAL_SPI_StateTypeDef State;
} SPI_HandleTypeDef;
typedef struct { int unused; } I2C_HandleTypeDef;
typedef struct { int unused; } TIM_HandleTypeDef;
typedef struct { int unused; } UART_HandleTypeDef;
typedef struct { int unused; } GPIO_TypeDef;
typedef struct {
    uint32_t Pin, Mode, Pull, Speed, Alternate;
} GPIO_InitTypeDef;

extern GPIO_TypeDef test_gpio_c;
#define GPIOC (&test_gpio_c)

void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state);
void HAL_GPIO_Init(GPIO_TypeDef *port, GPIO_InitTypeDef *init);
void HAL_Delay(uint32_t ms);
HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *spi, uint8_t *data,
                                    uint16_t size, uint32_t timeout);
HAL_StatusTypeDef HAL_SPI_Transmit_DMA(SPI_HandleTypeDef *spi, const uint8_t *data,
                                        uint16_t size);
HAL_SPI_StateTypeDef HAL_SPI_GetState(SPI_HandleTypeDef *spi);
HAL_StatusTypeDef HAL_SPI_Abort(SPI_HandleTypeDef *spi);
HAL_StatusTypeDef HAL_DMA_Abort(DMA_HandleTypeDef *dma);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *spi);
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *spi);

#endif
