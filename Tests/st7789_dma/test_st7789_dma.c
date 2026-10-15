#include "stm32f4xx_hal.h"
#include "driver_st7789_f407.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static DMA_Stream_TypeDef stream;
static DMA_HandleTypeDef dma = {&stream};
SPI_HandleTypeDef hspi3 = {&dma, HAL_SPI_STATE_READY};
GPIO_TypeDef test_gpio_c;

static GPIO_PinState cs = GPIO_PIN_SET;
static const uint8_t *pending;
static uint16_t pending_size;
static unsigned starts, fail_start, completions, blocking_calls, aborts;
static int completion_status, fail_command;
static uint8_t last_blocking[512];
static uint16_t last_blocking_size;
static uint8_t wire[240U * 296U * 2U];
static size_t wire_size;

void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state)
{
    (void)port;
    if (pin == GPIO_PIN_1) cs = state;
}

void HAL_GPIO_Init(GPIO_TypeDef *port, GPIO_InitTypeDef *init)
{
    (void)port;
    (void)init;
}

void HAL_Delay(uint32_t ms) { (void)ms; }

HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *spi, uint8_t *data,
                                    uint16_t size, uint32_t timeout)
{
    (void)spi;
    (void)timeout;
    blocking_calls++;
    assert(cs == GPIO_PIN_RESET && size <= sizeof(last_blocking));
    memcpy(last_blocking, data, size);
    last_blocking_size = size;
    return fail_command ? HAL_ERROR : HAL_OK;
}

HAL_StatusTypeDef HAL_SPI_Transmit_DMA(SPI_HandleTypeDef *spi, const uint8_t *data,
                                        uint16_t size)
{
    assert(cs == GPIO_PIN_RESET && pending == NULL);
    assert(spi->State == HAL_SPI_STATE_READY);
    assert(size != 0U && size <= 65534U && size % 2U == 0U);
    starts++;
    spi->State = HAL_SPI_STATE_BUSY_TX;
    if (starts == fail_start) return HAL_ERROR;
    pending = data;
    pending_size = size;
    stream.CR = DMA_SxCR_EN;
    return HAL_OK;
}

HAL_SPI_StateTypeDef HAL_SPI_GetState(SPI_HandleTypeDef *spi) { return spi->State; }

HAL_StatusTypeDef HAL_SPI_Abort(SPI_HandleTypeDef *spi)
{
    aborts++;
    spi->State = HAL_SPI_STATE_READY;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_DMA_Abort(DMA_HandleTypeDef *handle)
{
    handle->Instance->CR = 0U;
    pending = NULL;
    return HAL_OK;
}

void F407_ST7789_FlushCompleteCallback(int status)
{
    assert(cs == GPIO_PIN_SET);
    completions++;
    completion_status = status;
}

static void complete_dma(void)
{
    assert(pending != NULL && wire_size + pending_size <= sizeof(wire));
    memcpy(wire + wire_size, pending, pending_size);
    wire_size += pending_size;
    pending = NULL;
    stream.CR = 0U;
    hspi3.State = HAL_SPI_STATE_READY;
    HAL_SPI_TxCpltCallback(&hspi3);
}

static void reset_mock(void)
{
    assert(F407_ST7789_AbortDMA() == 0);
    starts = fail_start = completions = blocking_calls = aborts = 0U;
    fail_command = 0;
    completion_status = 99;
    wire_size = 0U;
}

int main(void)
{
    uint16_t pixels[4] = {0xF800U, 0x07E0U, 0x001FU, 0xFFFFU};
    const uint8_t expected[] = {0xF8, 0x00, 0x07, 0xE0, 0x00, 0x1F, 0xFF, 0xFF};
    static uint16_t full[240U * 296U];

    reset_mock();
    assert(F407_ST7789_Flush(0, 0, 1, 1, pixels) == 0);
    assert(last_blocking_size == sizeof(expected));
    assert(memcmp(last_blocking, expected, sizeof(expected)) == 0);
    assert(pixels[0] == 0xF800U && cs == GPIO_PIN_SET);

    reset_mock();
    assert(F407_ST7789_FlushDMA(0, 0, 1, 1, pixels) == 0);
    assert(pending == (const uint8_t *)pixels && completions == 0U);
    assert(memcmp(pending, expected, sizeof(expected)) == 0);
    unsigned calls = blocking_calls;
    assert(F407_ST7789_FlushDMA(0, 0, 1, 1, pixels) == -1);
    assert(F407_ST7789_Flush(0, 0, 1, 1, pixels) == -1);
    assert(blocking_calls == calls && cs == GPIO_PIN_RESET);
    complete_dma();
    assert(completions == 1U && completion_status == 0);
    assert(memcmp(wire, expected, sizeof(expected)) == 0);

    reset_mock();
    for (unsigned i = 0U; i < 240U * 296U; ++i) full[i] = (uint16_t)i;
    assert(F407_ST7789_FlushDMA(0, 0, 239, 295, full) == 0);
    assert(pending_size == 65534U);
    complete_dma();
    assert(completions == 0U && pending_size == 65534U && cs == GPIO_PIN_RESET);
    complete_dma();
    assert(completions == 0U && pending_size == 11012U && cs == GPIO_PIN_RESET);
    complete_dma();
    assert(starts == 3U && completions == 1U && completion_status == 0);
    assert(wire_size == sizeof(full));
    for (unsigned i = 0U; i < 240U * 296U; ++i) {
        assert(wire[2U * i] == (uint8_t)((uint16_t)i >> 8));
        assert(wire[2U * i + 1U] == (uint8_t)i);
    }

    reset_mock();
    fail_command = 1;
    assert(F407_ST7789_FlushDMA(0, 0, 1, 1, pixels) == -1);
    assert(starts == 0U && completions == 0U && cs == GPIO_PIN_SET);

    reset_mock();
    fail_start = 1U;
    assert(F407_ST7789_FlushDMA(0, 0, 1, 1, pixels) == -1);
    assert(completions == 0U && F407_ST7789_AbortDMA() == 0);
    assert(aborts == 1U && stream.CR == 0U);
    fail_start = 0U;
    assert(F407_ST7789_FlushDMA(0, 0, 1, 1, pixels) == 0);
    complete_dma();
    assert(completions == 1U && completion_status == 0);

    reset_mock();
    fail_start = 2U;
    assert(F407_ST7789_FlushDMA(0, 0, 239, 295, full) == 0);
    complete_dma();
    assert(completions == 1U && completion_status == -1);
    assert(F407_ST7789_AbortDMA() == 0);
    HAL_SPI_TxCpltCallback(&hspi3);
    assert(completions == 1U);

    reset_mock();
    assert(F407_ST7789_FlushDMA(0, 0, 1, 1, pixels) == 0);
    HAL_SPI_ErrorCallback(&hspi3);
    assert(completions == 1U && completion_status == -1);
    assert(F407_ST7789_AbortDMA() == 0 && pending == NULL && stream.CR == 0U);

    reset_mock();
    assert(F407_ST7789_FlushDMA(0, 0, 1, 1, pixels) == 0);
    SPI_HandleTypeDef other_spi = {0};
    HAL_SPI_TxCpltCallback(&other_spi);
    HAL_SPI_ErrorCallback(&other_spi);
    assert(completions == 0U && cs == GPIO_PIN_RESET);
    assert(F407_ST7789_AbortDMA() == 0);
    HAL_SPI_TxCpltCallback(&hspi3);
    assert(completions == 0U && pending == NULL);

    reset_mock();
    assert(F407_ST7789_FlushDMA(0, 0, 0, 0, NULL) == -1);
    assert(F407_ST7789_FlushDMA(1, 0, 0, 0, pixels) == -1);
    assert(F407_ST7789_FlushDMA(0, 1, 0, 0, pixels) == -1);
    assert(F407_ST7789_FlushDMA(0, 0, 240, 0, pixels) == -1);
    assert(F407_ST7789_FlushDMA(0, 0, 0, 296, pixels) == -1);
    assert(starts == 0U && blocking_calls == 0U);

    puts("PASS: blocking API, RGB565 order, async lifetime, busy rejection, full-frame chunks,");
    puts("      command/start/chained/SPI failures, abort, unrelated IRQs, invalid areas");
    return 0;
}
