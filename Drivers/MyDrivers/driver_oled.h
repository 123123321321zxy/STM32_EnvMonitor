#ifndef __DRIVER_OLED_H
#define __DRIVER_OLED_H

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "ascii_font.h"



#define OLED_QUEUE_LENGTH	20

//states
#define SYS_STATE_STOPPED			1
#define SYS_STATE_RUNNING			2
#define SYS_STATE_READY				3

typedef struct Oled_DataT{
	uint8_t mState;
	uint16_t hum;
	int16_t tmp;
}Oled_DataT;


void OLED_Init(void);
void OLED_Clear(void);
//void OLED_DrawPoint(uint8_t x, uint8_t y);
void OLED_ShowString(uint8_t x, uint8_t y, char *str);
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t ch);
void OLED_ShowNum(uint8_t x, uint8_t y, int32_t num);
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t decimals);
void OLED_Test(void);



#endif

