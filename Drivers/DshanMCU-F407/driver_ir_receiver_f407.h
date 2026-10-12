#ifndef DRIVER_IR_RECEIVER_F407_H
#define DRIVER_IR_RECEIVER_F407_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    uint32_t edge_count;
    uint32_t frame_ok_count;
    uint32_t frame_error_count;
    uint8_t last_device;
    uint8_t last_command;
    uint8_t pin_level;
} F407_IRReceiver_Diagnostics_t;
void F407_IRReceiver_Init(void);
void F407_IRReceiver_EXTI_Callback(uint16_t gpio_pin);
int F407_IRReceiver_Read(uint8_t *device, uint8_t *command);
void F407_IRReceiver_GetDiagnostics(F407_IRReceiver_Diagnostics_t *diagnostics);
#ifdef __cplusplus
}
#endif
#endif
