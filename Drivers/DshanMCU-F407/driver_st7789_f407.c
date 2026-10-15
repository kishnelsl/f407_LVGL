#include "driver_st7789_f407.h"
#include "driver_f407_pins.h"
#include "stm32f4xx_hal.h"

#define ST7789_SWRESET 0x01U
#define ST7789_SLPOUT  0x11U
#define ST7789_COLMOD  0x3AU
#define ST7789_MADCTL  0x36U
#define ST7789_CASET   0x2AU
#define ST7789_RASET   0x2BU
#define ST7789_RAMWR   0x2CU
#define ST7789_INVON   0x21U
#define ST7789_NORON   0x13U
#define ST7789_DISPON  0x29U

static uint8_t g_rotation;
static volatile uint8_t s_dma_active;
static uint8_t *s_dma_next;
static uint32_t s_dma_remaining;

static void lcd_cs(GPIO_PinState state)
{ HAL_GPIO_WritePin(F407_ST7789_CS_PORT, F407_ST7789_CS_PIN, state); }

static void lcd_dc(GPIO_PinState state)
{ HAL_GPIO_WritePin(F407_ST7789_DC_PORT, F407_ST7789_DC_PIN, state); }

/**
 * @brief 执行lcd_write相关功能。
 * @param data 函数输入或输出参数。
 * @param len 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
static int lcd_write(const uint8_t *data, uint16_t len)
{
    return HAL_SPI_Transmit(&F407_ST7789_SPI, (uint8_t *)data, len, 100U) == HAL_OK ? 0 : -1;
}

/**
 * @brief 执行lcd_command相关功能。
 * @param command 函数输入或输出参数。
 * @param data 函数输入或输出参数。
 * @param len 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
static int lcd_command(uint8_t command, const uint8_t *data, uint16_t len)
{
    int result;
    if (s_dma_active) return -1;
    lcd_cs(GPIO_PIN_RESET);
    lcd_dc(GPIO_PIN_RESET);
    result = lcd_write(&command, 1U);
    if (result == 0 && data != NULL && len != 0U) {
        lcd_dc(GPIO_PIN_SET);
        result = lcd_write(data, len);
    }
    lcd_cs(GPIO_PIN_SET);
    return result;
}

/**
 * @brief 执行F407_ST7789_Init相关功能。
 * @return 函数处理结果或计算值。
 */
int F407_ST7789_Init(void)
{
    GPIO_InitTypeDef gpio = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    gpio.Pin = F407_ST7789_SCK_PIN | F407_ST7789_MOSI_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOC, &gpio);
    gpio.Pin = F407_ST7789_DC_PIN | F407_ST7789_CS_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOC, &gpio);
    lcd_cs(GPIO_PIN_SET);
    lcd_dc(GPIO_PIN_SET);

    if (lcd_command(ST7789_SWRESET, NULL, 0U) != 0) return -1;
    HAL_Delay(150U);
    if (lcd_command(ST7789_SLPOUT, NULL, 0U) != 0) return -1;
    HAL_Delay(120U);
    { uint8_t format = 0x55U; if (lcd_command(ST7789_COLMOD, &format, 1U) != 0) return -1; }
    F407_ST7789_SetRotation(0U);
    if (lcd_command(ST7789_INVON, NULL, 0U) != 0) return -1;
    if (lcd_command(ST7789_NORON, NULL, 0U) != 0) return -1;
    HAL_Delay(10U);
    if (lcd_command(ST7789_DISPON, NULL, 0U) != 0) return -1;
    HAL_Delay(100U);
    return 0;
}

/**
 * @brief 执行F407_ST7789_SetRotation相关功能。
 * @param rotation 函数输入或输出参数。
 * @return None
 */
void F407_ST7789_SetRotation(uint8_t rotation)
{
    static const uint8_t madctl[4] = {0x00U, 0x60U, 0xC0U, 0xA0U};
    g_rotation = (uint8_t)(rotation & 3U);
    (void)lcd_command(ST7789_MADCTL, &madctl[g_rotation], 1U);
}

/**
 * @brief 执行F407_ST7789_SetAddressWindow相关功能。
 * @param x0 函数输入或输出参数。
 * @param y0 函数输入或输出参数。
 * @param x1 函数输入或输出参数。
 * @param y1 函数输入或输出参数。
 * @return None
 */
int F407_ST7789_SetAddressWindow(uint16_t x0, uint16_t y0,
                                  uint16_t x1, uint16_t y1)
{
    uint8_t data[4];
    data[0] = (uint8_t)((x0 + F407_ST7789_X_OFFSET) >> 8);
    data[1] = (uint8_t)(x0 + F407_ST7789_X_OFFSET);
    data[2] = (uint8_t)((x1 + F407_ST7789_X_OFFSET) >> 8);
    data[3] = (uint8_t)(x1 + F407_ST7789_X_OFFSET);
    if (lcd_command(ST7789_CASET, data, sizeof(data)) != 0) return -1;
    data[0] = (uint8_t)((y0 + F407_ST7789_Y_OFFSET) >> 8);
    data[1] = (uint8_t)(y0 + F407_ST7789_Y_OFFSET);
    data[2] = (uint8_t)((y1 + F407_ST7789_Y_OFFSET) >> 8);
    data[3] = (uint8_t)(y1 + F407_ST7789_Y_OFFSET);
    if (lcd_command(ST7789_RASET, data, sizeof(data)) != 0) return -1;
    return lcd_command(ST7789_RAMWR, NULL, 0U);
}

/**
 * @brief 执行F407_ST7789_DrawPixel相关功能。
 * @param x 函数输入或输出参数。
 * @param y 函数输入或输出参数。
 * @param color 函数输入或输出参数。
 * @return None
 */
void F407_ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    uint8_t data[2];
    if (x >= F407_ST7789_WIDTH || y >= F407_ST7789_HEIGHT) return;
    if (F407_ST7789_SetAddressWindow(x, y, x, y) != 0) return;
    data[0] = (uint8_t)(color >> 8); data[1] = (uint8_t)color;
    lcd_cs(GPIO_PIN_RESET); lcd_dc(GPIO_PIN_SET);
    (void)lcd_write(data, sizeof(data));
    lcd_cs(GPIO_PIN_SET);
}

/**
 * @brief 执行F407_ST7789_Fill相关功能。
 * @param color 函数输入或输出参数。
 * @return None
 */
void F407_ST7789_Fill(uint16_t color)
{
    uint8_t data[512];
    uint32_t pixels = (uint32_t)F407_ST7789_WIDTH * F407_ST7789_HEIGHT;
    uint32_t i;
    for (i = 0; i < sizeof(data); i += 2U) { data[i] = (uint8_t)(color >> 8); data[i + 1U] = (uint8_t)color; }
    if (F407_ST7789_SetAddressWindow(0U, 0U, F407_ST7789_WIDTH - 1U,
                                    F407_ST7789_HEIGHT - 1U) != 0) return;
    lcd_cs(GPIO_PIN_RESET); lcd_dc(GPIO_PIN_SET);
    while (pixels != 0U) {
        uint16_t count = pixels > 256U ? 256U : (uint16_t)pixels;
        if (lcd_write(data, (uint16_t)(count * 2U)) != 0) break;
        pixels -= count;
    }
    lcd_cs(GPIO_PIN_SET);
}

/**
 * @brief 执行F407_ST7789_Flush相关功能。
 * @param x1 函数输入或输出参数。
 * @param y1 函数输入或输出参数。
 * @param x2 函数输入或输出参数。
 * @param y2 函数输入或输出参数。
 * @param color_p 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_ST7789_Flush(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                      const uint16_t *color_p)
{
    uint8_t tx[512];
    uint32_t count, i, done = 0U;
    if (color_p == NULL || x1 > x2 || y1 > y2 || x2 >= F407_ST7789_WIDTH ||
        y2 >= F407_ST7789_HEIGHT) return -1;
    count = (uint32_t)(x2 - x1 + 1U) * (uint32_t)(y2 - y1 + 1U);
    if (F407_ST7789_SetAddressWindow(x1, y1, x2, y2) != 0) return -1;
    lcd_cs(GPIO_PIN_RESET); lcd_dc(GPIO_PIN_SET);
    while (done < count) {
        uint32_t part = count - done > 256U ? 256U : count - done;
        for (i = 0U; i < part; ++i) {
            uint16_t pixel = color_p[done + i];
            tx[2U * i] = (uint8_t)(pixel >> 8);
            tx[2U * i + 1U] = (uint8_t)pixel;
        }
        if (lcd_write(tx, (uint16_t)(part * 2U)) != 0) {
            lcd_cs(GPIO_PIN_SET);
            return -1;
        }
        done += part;
    }
    lcd_cs(GPIO_PIN_SET);
    return 0;
}

static int lcd_start_dma_chunk(void)
{
    /* HAL's count is uint16_t; keep each chunk on an RGB565 pixel boundary. */
    uint16_t bytes = s_dma_remaining > 65534U ? 65534U : (uint16_t)s_dma_remaining;
    uint8_t *data = s_dma_next;

    s_dma_next += bytes;
    s_dma_remaining -= bytes;
    return HAL_SPI_Transmit_DMA(&F407_ST7789_SPI, data, bytes) == HAL_OK ? 0 : -1;
}

int F407_ST7789_FlushDMA(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                         uint16_t *color_p)
{
    uint32_t count;
    uint8_t *bytes = (uint8_t *)color_p;

    if (s_dma_active || F407_ST7789_SPI.hdmatx == NULL ||
        HAL_SPI_GetState(&F407_ST7789_SPI) != HAL_SPI_STATE_READY ||
        color_p == NULL || x1 > x2 || y1 > y2 ||
        x2 >= F407_ST7789_WIDTH || y2 >= F407_ST7789_HEIGHT) return -1;
    if (F407_ST7789_SetAddressWindow(x1, y1, x2, y2) != 0) return -1;

    count = (uint32_t)(x2 - x1 + 1U) * (uint32_t)(y2 - y1 + 1U);
    for (uint32_t i = 0U; i < count; ++i) {
        uint16_t pixel = color_p[i];
        bytes[2U * i] = (uint8_t)(pixel >> 8);
        bytes[2U * i + 1U] = (uint8_t)pixel;
    }

    s_dma_next = bytes;
    s_dma_remaining = count * 2U;
    s_dma_active = 1U;
    lcd_cs(GPIO_PIN_RESET);
    lcd_dc(GPIO_PIN_SET);
    if (lcd_start_dma_chunk() != 0) {
        /* The caller must abort before releasing the buffer after a HAL error. */
        s_dma_active = 0U;
        lcd_cs(GPIO_PIN_SET);
        return -1;
    }
    return 0;
}

int F407_ST7789_AbortDMA(void)
{
    HAL_StatusTypeDef result;

    s_dma_active = 0U;
    s_dma_remaining = 0U;
    result = HAL_SPI_Abort(&F407_ST7789_SPI);
    /* Some HAL error paths clear TXDMAEN before aborting the DMA stream. */
    if (F407_ST7789_SPI.hdmatx != NULL) {
        (void)HAL_DMA_Abort(F407_ST7789_SPI.hdmatx);
        if ((F407_ST7789_SPI.hdmatx->Instance->CR & DMA_SxCR_EN) != 0U) {
            result = HAL_ERROR;
        }
    }
    lcd_cs(GPIO_PIN_SET);
    return result == HAL_OK ? 0 : -1;
}

__weak void F407_ST7789_FlushCompleteCallback(int status)
{
    (void)status;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    int status = 0;

    if (hspi != &F407_ST7789_SPI || !s_dma_active) return;
    if (s_dma_remaining != 0U) {
        if (lcd_start_dma_chunk() == 0) return;
        status = -1;
    }
    s_dma_active = 0U;
    lcd_cs(GPIO_PIN_SET);
    F407_ST7789_FlushCompleteCallback(status);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi != &F407_ST7789_SPI || !s_dma_active) return;
    s_dma_active = 0U;
    lcd_cs(GPIO_PIN_SET);
    F407_ST7789_FlushCompleteCallback(-1);
}

int F407_LCD_Init(void)
{ return F407_ST7789_Init(); }
