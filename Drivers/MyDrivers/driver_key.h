#ifndef __DRIVER_KEY_H
#define __DRIVER_KEY_H

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"


#define KEY_GPIO_PORT GPIOB
#define KEY_GPIO_PIN  GPIO_PIN_0
#define KEY_EVENT_SHORT_PRESS	1
#define KEY_EVENT_LONG_PRESS	2

#define KEY_EVENT_TYPE_PRESS		1
#define KEY_EVENT_TYPE_UNPRESS		2

#define KEY_QUEUE_LEN 	10
#define KEY_LONG_PRESS_TIME 		1000 //500ms,以0.5ms为单位
#define KEY_EVENT_TIME_OUT			1000//1S,以ms为单位

typedef struct Key_DataT{
	uint8_t event_type;
	uint16_t cnt;
}Key_DataT;

void KEY_Process(uint8_t* pu8KeyEventInfo);
#endif
