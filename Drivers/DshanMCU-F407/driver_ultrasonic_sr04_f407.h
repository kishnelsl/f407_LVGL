#ifndef DRIVER_ULTRASONIC_SR04_F407_H
#define DRIVER_ULTRASONIC_SR04_F407_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void F407_SR04_Init(void);
int F407_SR04_Start(void);
int F407_SR04_GetResult(uint32_t *distance_cm);
void F407_SR04_EXTI_Callback(uint16_t gpio_pin);
int F407_SR04_Read(uint32_t *distance_cm);
#ifdef __cplusplus
}
#endif
#endif
