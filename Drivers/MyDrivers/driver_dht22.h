#ifndef __DRIVER_DHT22_H
#define __DRIVER_DHT22_H

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"


#define DHT22_GPIO_PORT GPIOA
#define DHT22_GPIO_PIN  GPIO_PIN_0


typedef struct dht22_dataT{
    uint16_t humidity;
    int16_t temperature;
    uint8_t checkSum;

}dht22_dataT;

uint8_t Get_Dht22_Data(dht22_dataT* data);
uint8_t DHT22_ReadData(dht22_dataT *data);

#endif

