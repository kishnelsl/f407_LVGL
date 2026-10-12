#ifndef DRIVER_COLOR_LED_F407_H
#define DRIVER_COLOR_LED_F407_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void F407_ColorLED_Init(void);
void F407_ColorLED_Set(uint32_t color);
void F407_ColorLED_Test(void);
#ifdef __cplusplus
}
#endif
#endif
