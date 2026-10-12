#ifndef DRIVER_DHT11_F407_H
#define DRIVER_DHT11_F407_H
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    F407_DHT11_OK = 0,
    F407_DHT11_NOT_READ = 1,
    F407_DHT11_INVALID_ARGUMENT = -1,
    F407_DHT11_NO_ACK = -2,
    F407_DHT11_TIMEOUT = -3,
    F407_DHT11_CHECKSUM = -4
} F407_DHT11_Status_t;
void F407_DHT11_Init(void);
int F407_DHT11_Read(int *humidity, int *temperature);
#ifdef __cplusplus
}
#endif
#endif
