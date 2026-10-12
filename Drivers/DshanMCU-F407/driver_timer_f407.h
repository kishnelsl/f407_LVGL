#ifndef DRIVER_TIMER_F407_H
#define DRIVER_TIMER_F407_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
uint32_t F407_Timer_Cycles(void);
uint32_t F407_Timer_CyclesPerUs(void);
void F407_udelay(uint32_t us);
void F407_mdelay(uint32_t ms);
uint64_t F407_Timer_Ns(void);
#ifdef __cplusplus
}
#endif
#endif
