#ifndef DRIVER_UART_F407_H
#define DRIVER_UART_F407_H
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
void F407_UART_Init(void);
int F407_UART_PutChar(int c);
int F407_UART_GetChar(void);
#ifdef __cplusplus
}
#endif
#endif
