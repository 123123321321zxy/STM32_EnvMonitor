#ifndef __DRIVER_LED_H
#define __DRIVER_LED_H

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"


#define LED_GPIO_PORT GPIOC
#define LED_GPIO_PIN  GPIO_PIN_13

#define HIGH_TEMPERURATUE_LIMIT 35
#define LED_LIGHT	1



void LED_TOGGLE(void);
void LED_ON(void);
void LED_OFF(void);
void LED_Test(void);
void LED_Alarm(void);
#endif
