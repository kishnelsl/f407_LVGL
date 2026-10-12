#ifndef DRIVER_MPU6050_F407_H
#define DRIVER_MPU6050_F407_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int F407_MPU6050_Init(void);
int F407_MPU6050_GetID(void);
int F407_MPU6050_ReadRegister(uint8_t reg, uint8_t *data);
int F407_MPU6050_ReadData(int16_t *acc_x, int16_t *acc_y, int16_t *acc_z,
                          int16_t *gyro_x, int16_t *gyro_y, int16_t *gyro_z);
#ifdef __cplusplus
}
#endif
#endif
