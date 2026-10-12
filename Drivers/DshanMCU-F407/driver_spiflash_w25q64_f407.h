#ifndef DRIVER_SPIFLASH_W25Q64_F407_H
#define DRIVER_SPIFLASH_W25Q64_F407_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void F407_W25Q64_Init(void);
int F407_W25Q64_ReadID(uint32_t *id);
int F407_W25Q64_Read(uint32_t offset, uint8_t *buf, uint32_t len);
int F407_W25Q64_Write(uint32_t offset, const uint8_t *buf, uint32_t len);
int F407_W25Q64_Erase(uint32_t offset, uint32_t len);
#ifdef __cplusplus
}
#endif
#endif
