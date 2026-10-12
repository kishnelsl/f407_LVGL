#include "driver_spiflash_w25q64_f407.h"
#include "driver_f407_pins.h"
#include <stddef.h>

#define F407_W25_TIMEOUT 100U
#define W25_WRITE_ENABLE 0x06U
#define W25_READ_STATUS  0x05U
#define W25_READ_DATA    0x03U
#define W25_PAGE_PROGRAM 0x02U
#define W25_SECTOR_ERASE 0x20U
#define W25_JEDEC_ID     0x9FU

static void F407_W25_Select(void) { HAL_GPIO_WritePin(F407_W25Q64_CS_PORT, F407_W25Q64_CS_PIN, GPIO_PIN_RESET); }
static void F407_W25_Deselect(void) { HAL_GPIO_WritePin(F407_W25Q64_CS_PORT, F407_W25Q64_CS_PIN, GPIO_PIN_SET); }
static int F407_W25_Tx(const uint8_t *data, uint16_t len) { return HAL_SPI_Transmit(&hspi1, (uint8_t *)data, len, F407_W25_TIMEOUT) == HAL_OK ? 0 : -1; }
static int F407_W25_Rx(uint8_t *data, uint16_t len) { return HAL_SPI_Receive(&hspi1, data, len, F407_W25_TIMEOUT) == HAL_OK ? 0 : -1; }

/**
 * @brief 执行F407_W25_WaitReady相关功能。
 * @return 函数处理结果或计算值。
 */
static int F407_W25_WaitReady(void)
{
    uint8_t cmd = W25_READ_STATUS, status = 0;
    uint32_t start = HAL_GetTick();
    do {
        F407_W25_Select();
        if (F407_W25_Tx(&cmd, 1) != 0 || F407_W25_Rx(&status, 1) != 0) { F407_W25_Deselect(); return -1; }
        F407_W25_Deselect();
        if ((status & 1U) == 0U) return 0;
        HAL_Delay(1);
    } while ((HAL_GetTick() - start) < 500U);
    return -1;
}

/**
 * @brief 执行F407_W25_WriteEnable相关功能。
 * @return 函数处理结果或计算值。
 */
static int F407_W25_WriteEnable(void)
{
    uint8_t cmd = W25_WRITE_ENABLE;
    F407_W25_Select(); int result = F407_W25_Tx(&cmd, 1); F407_W25_Deselect(); return result;
}

/**
 * @brief 执行F407_W25Q64_Init相关功能。
 * @return None
 */
void F407_W25Q64_Init(void)
{
    GPIO_InitTypeDef init = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    init.Pin = F407_W25Q64_CS_PIN; init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL; init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(F407_W25Q64_CS_PORT, &init); F407_W25_Deselect();
}

/**
 * @brief 执行F407_W25Q64_ReadID相关功能。
 * @param id 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_W25Q64_ReadID(uint32_t *id)
{
    uint8_t cmd = W25_JEDEC_ID, data[3];
    if (id == NULL) return -1;
    F407_W25_Select();
    int result = F407_W25_Tx(&cmd, 1) || F407_W25_Rx(data, 3);
    F407_W25_Deselect();
    if (result != 0) return -1;
    *id = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | data[2];
    return 0;
}

/**
 * @brief 执行F407_W25Q64_Read相关功能。
 * @param offset 函数输入或输出参数。
 * @param buf 函数输入或输出参数。
 * @param len 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_W25Q64_Read(uint32_t offset, uint8_t *buf, uint32_t len)
{
    uint32_t done = 0;
    if (buf == NULL || len == 0U) return -1;
    while (done < len) {
        uint32_t part = (len - done > 65535U) ? 65535U : (len - done);
        uint8_t cmd[4] = {W25_READ_DATA, (uint8_t)((offset + done) >> 16),
                          (uint8_t)((offset + done) >> 8), (uint8_t)(offset + done)};
        F407_W25_Select();
        int result = F407_W25_Tx(cmd, 4) || F407_W25_Rx(buf + done, (uint16_t)part);
        F407_W25_Deselect();
        if (result != 0) return -1;
        done += part;
    }
    return (int)done;
}

/**
 * @brief 执行F407_W25Q64_Write相关功能。
 * @param offset 函数输入或输出参数。
 * @param buf 函数输入或输出参数。
 * @param len 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_W25Q64_Write(uint32_t offset, const uint8_t *buf, uint32_t len)
{
    uint32_t done = 0;
    if (buf == NULL || len == 0U) return -1;
    while (done < len) {
        uint32_t page_left = 256U - ((offset + done) & 255U);
        uint32_t part = (len - done < page_left) ? len - done : page_left;
        uint8_t cmd[4] = {W25_PAGE_PROGRAM, (uint8_t)((offset + done) >> 16), (uint8_t)((offset + done) >> 8), (uint8_t)(offset + done)};
        if (F407_W25_WriteEnable() != 0) return -1;
        F407_W25_Select();
        if (F407_W25_Tx(cmd, 4) != 0 || F407_W25_Tx(buf + done, (uint16_t)part) != 0) { F407_W25_Deselect(); return -1; }
        F407_W25_Deselect();
        if (F407_W25_WaitReady() != 0) return -1;
        done += part;
    }
    return (int)done;
}

/**
 * @brief 执行F407_W25Q64_Erase相关功能。
 * @param offset 函数输入或输出参数。
 * @param len 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
int F407_W25Q64_Erase(uint32_t offset, uint32_t len)
{
    uint32_t done = 0;
    if ((offset & 0xFFFU) != 0U || (len & 0xFFFU) != 0U || len == 0U) return -1;
    while (done < len) {
        uint8_t cmd[4] = {W25_SECTOR_ERASE, (uint8_t)((offset + done) >> 16), (uint8_t)((offset + done) >> 8), (uint8_t)(offset + done)};
        if (F407_W25_WriteEnable() != 0) return -1;
        F407_W25_Select();
        if (F407_W25_Tx(cmd, 4) != 0) { F407_W25_Deselect(); return -1; }
        F407_W25_Deselect();
        if (F407_W25_WaitReady() != 0) return -1;
        done += 4096U;
    }
    return (int)done;
}
