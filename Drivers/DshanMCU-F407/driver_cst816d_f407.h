#ifndef DRIVER_CST816D_F407_H
#define DRIVER_CST816D_F407_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define F407_CST816D_RAW_SIZE 8U

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t  fingers;
    uint8_t  gesture;
} F407_CST816D_Point_t;

int F407_CST816D_Init(void);
int F407_CST816D_GetID(uint8_t *id);
int F407_CST816D_Read(F407_CST816D_Point_t *point);
void F407_CST816D_GetLastRaw(uint8_t raw[F407_CST816D_RAW_SIZE]);

#ifdef __cplusplus
}
#endif
#endif
