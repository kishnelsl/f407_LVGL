#ifndef DRIVER_ST7789_F407_H
#define DRIVER_ST7789_F407_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The supplied 2.1-inch TK02... panel is a 240x296 RGB565 glass.  Keep these
 * macros overrideable because ST7789 modules use different address offsets. */
#ifndef F407_ST7789_WIDTH
#define F407_ST7789_WIDTH  240U
#endif
#ifndef F407_ST7789_HEIGHT
#define F407_ST7789_HEIGHT 296U
#endif
#ifndef F407_ST7789_X_OFFSET
#define F407_ST7789_X_OFFSET 0U
#endif
#ifndef F407_ST7789_Y_OFFSET
#define F407_ST7789_Y_OFFSET 0U
#endif

int F407_ST7789_Init(void);
void F407_ST7789_SetRotation(uint8_t rotation);
int F407_ST7789_SetAddressWindow(uint16_t x0, uint16_t y0,
                                  uint16_t x1, uint16_t y1);
void F407_ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void F407_ST7789_Fill(uint16_t color);
/* LVGL flush helper: color_p contains host-endian RGB565 values. */
int F407_ST7789_Flush(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                      const uint16_t *color_p);
/* Swaps RGB565 bytes in place. Buffer must be in DMA-accessible SRAM (not CCM)
 * and remain unchanged until completion/abort. Returns 0 when DMA starts.
 * On a start failure, abort before reusing the buffer or starting another transfer.
 * LCD access is owned by one task; do not call other LCD APIs during DMA. */
int F407_ST7789_FlushDMA(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                         uint16_t *color_p);
int F407_ST7789_AbortDMA(void); /* Task context; no completion callback. */
/* Override this weak hook. Called in ISR context on completion/error. On error,
 * call AbortDMA from task context before reusing the buffer. */
void F407_ST7789_FlushCompleteCallback(int status);
int F407_LCD_Init(void); /* Compatibility name used by LVGL board ports. */

#ifdef __cplusplus
}
#endif
#endif
