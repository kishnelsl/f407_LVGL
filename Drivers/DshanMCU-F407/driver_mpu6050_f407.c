#include "driver_mpu6050_f407.h"
#include "driver_f407_pins.h"
#include <stddef.h>

#define F407_MPU_ADDR       0xD0U
#define F407_MPU_TIMEOUT_MS 50U
#define MPU_PWR_MGMT_1      0x6B
#define MPU_PWR_MGMT_2      0x6C
#define MPU_SMPLRT_DIV      0x19
#define MPU_CONFIG           0x1A
#define MPU_GYRO_CONFIG      0x1B
#define MPU_ACCEL_CONFIG     0x1C
#define MPU_ACCEL_XOUT_H     0x3B
#define MPU_WHO_AM_I         0x75

/**
 * @brief 执行F407_MPU_Write相关功能。
 * @param reg 函数输入或输出参数。
 * @param value 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
static int F407_MPU_Write(uint8_t reg, uint8_t value)
{
    uint8_t data[2] = {reg, value};
    return HAL_I2C_Master_Transmit(&hi2c1, F407_MPU_ADDR, data, 2, F407_MPU_TIMEOUT_MS) == HAL_OK ? 0 : -1;
}

/**
 * @brief 读取MPU6050指定寄存器。
 * @param reg 函数输入或输出参数。
 * @param data 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_MPU6050_ReadRegister(uint8_t reg, uint8_t *data)
{
    if (data == NULL) return -1;
    return HAL_I2C_Mem_Read(&hi2c1, F407_MPU_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                            data, 1, F407_MPU_TIMEOUT_MS) == HAL_OK ? 0 : -1;
}

/**
 * @brief 初始化MPU6050工作参数。
 * @return 函数处理结果或计算值。
 */
int F407_MPU6050_Init(void)
{
    if (F407_MPU_Write(MPU_PWR_MGMT_1, 0x00) != 0) return -1;
    if (F407_MPU_Write(MPU_PWR_MGMT_2, 0x00) != 0) return -1;
    if (F407_MPU_Write(MPU_SMPLRT_DIV, 0x09) != 0) return -1;
    if (F407_MPU_Write(MPU_CONFIG, 0x06) != 0) return -1;
    if (F407_MPU_Write(MPU_GYRO_CONFIG, 0x18) != 0) return -1;
    return F407_MPU_Write(MPU_ACCEL_CONFIG, 0x18);
}

/**
 * @brief 读取MPU6050芯片标识。
 * @return 函数处理结果或计算值。
 */
int F407_MPU6050_GetID(void)
{
    uint8_t id = 0;
    return F407_MPU6050_ReadRegister(MPU_WHO_AM_I, &id) == 0 ? (int)id : -1;
}

/**
 * @brief 读取MPU6050加速度和角速度数据。
 * @param ax 函数输入或输出参数。
 * @param ay 函数输入或输出参数。
 * @param az 函数输入或输出参数。
 * @param gx 函数输入或输出参数。
 * @param gy 函数输入或输出参数。
 * @param gz 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_MPU6050_ReadData(int16_t *ax, int16_t *ay, int16_t *az,
                          int16_t *gx, int16_t *gy, int16_t *gz)
{
    uint8_t data[14];
    if (HAL_I2C_Mem_Read(&hi2c1, F407_MPU_ADDR, MPU_ACCEL_XOUT_H, I2C_MEMADD_SIZE_8BIT,
                         data, sizeof(data), F407_MPU_TIMEOUT_MS) != HAL_OK) return -1;
    if (ax) *ax = (int16_t)((data[0] << 8) | data[1]);
    if (ay) *ay = (int16_t)((data[2] << 8) | data[3]);
    if (az) *az = (int16_t)((data[4] << 8) | data[5]);
    if (gx) *gx = (int16_t)((data[8] << 8) | data[9]);
    if (gy) *gy = (int16_t)((data[10] << 8) | data[11]);
    if (gz) *gz = (int16_t)((data[12] << 8) | data[13]);
    return 0;
}
