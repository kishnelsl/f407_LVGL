#ifndef DRIVER_LED_F407_H
#define DRIVER_LED_F407_H
#ifdef __cplusplus
extern "C" {
#endif
int F407_Led_Init(void);
int F407_Led_Control(int on);
void F407_Led_Test(void);
#ifdef __cplusplus
}
#endif
#endif
