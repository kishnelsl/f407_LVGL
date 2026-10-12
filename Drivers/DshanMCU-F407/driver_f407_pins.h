/* STM32F407VET6扩展板的默认引脚映射。
 * 映射依据用户提供的GPIO复用图；如实物接线不同，只需修改本文件。
 * LCD连接器只有4根SPI线和4根I2C线，因此屏幕的RST/背光由屏幕板自身处理。 */
#ifndef DRIVER_F407_PINS_H
#define DRIVER_F407_PINS_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CubeMX生成的外设句柄。启用对应驱动时，请在CubeMX中生成同名句柄。 */
extern TIM_HandleTypeDef htim3;      /* PC6/PC7/PC8: TIM3_CH1/2/3，RGB LED */
extern I2C_HandleTypeDef hi2c1;      /* PB6/PB7: I2C1_SCL/SDA，MPU6050 */
extern SPI_HandleTypeDef hspi1;      /* PA5/PA6/PA7: SPI1_SCK/MISO/MOSI，W25Q64 */
extern UART_HandleTypeDef huart1;    /* PA9/PA10: USART1_TX/RX */
extern SPI_HandleTypeDef hspi3;      /* PC10/PC12: SPI3_SCK/MOSI，ST7789 */
extern I2C_HandleTypeDef hi2c3;      /* PA8/PC9: I2C3_SCL/SDA，CST816D */

#ifdef __cplusplus
}
#endif

#define F407_LED_PORT                 GPIOC
#define F407_LED_PIN                  GPIO_PIN_13

#define F407_RGB_RED_CHANNEL          TIM_CHANNEL_1
#define F407_RGB_GREEN_CHANNEL        TIM_CHANNEL_2
#define F407_RGB_BLUE_CHANNEL         TIM_CHANNEL_3
#define F407_RGB_ACTIVE_LOW           1

#define F407_DHT11_PORT               GPIOA
#define F407_DHT11_PIN                GPIO_PIN_1

#define F407_IR_OBSTACLE_PORT         GPIOB
#define F407_IR_OBSTACLE_PIN          GPIO_PIN_13
#define F407_IR_RECEIVER_PORT         GPIOB
#define F407_IR_RECEIVER_PIN          GPIO_PIN_10

#define F407_SR04_TRIG_PORT           GPIOB
#define F407_SR04_TRIG_PIN            GPIO_PIN_0
#define F407_SR04_ECHO_PORT           GPIOB
#define F407_SR04_ECHO_PIN            GPIO_PIN_1

#define F407_W25Q64_CS_PORT           GPIOA
#define F407_W25Q64_CS_PIN            GPIO_PIN_4

/* ST7789 connector: MOSI, DC, SCK, CS (3.3-V logic). */
#define F407_ST7789_SPI               hspi3
#define F407_ST7789_SCK_PORT          GPIOC
#define F407_ST7789_SCK_PIN           GPIO_PIN_10
#define F407_ST7789_MOSI_PORT         GPIOC
#define F407_ST7789_MOSI_PIN          GPIO_PIN_12
#define F407_ST7789_DC_PORT           GPIOC
#define F407_ST7789_DC_PIN            GPIO_PIN_0
#define F407_ST7789_CS_PORT           GPIOC
#define F407_ST7789_CS_PIN            GPIO_PIN_1

/* CST816D connector: GND, 3V3, SCL, SDA.  The controller address is 0x15. */
#define F407_CST816D_I2C              hi2c3
#define F407_CST816D_SCL_PORT         GPIOA
#define F407_CST816D_SCL_PIN          GPIO_PIN_8
#define F407_CST816D_SDA_PORT         GPIOC
#define F407_CST816D_SDA_PIN          GPIO_PIN_9
#define F407_CST816D_I2C_ADDR         (0x15U << 1)

#endif
