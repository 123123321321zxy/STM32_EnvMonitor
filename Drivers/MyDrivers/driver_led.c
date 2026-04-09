#include "driver_led.h"
#include "tim.h"

void LED_TOGGLE(void)
{
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
}

void LED_ON(void)
{
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_RESET);
}

void LED_OFF(void)
{
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);
}

void LED_Test(void)
{
    //已在main初始化
    while(1)
    {
        LED_TOGGLE();
        HAL_Delay(200);
    }
}

void LED_Alarm(void)
{
    //已在main初始化
    while(1)
    {
        LED_TOGGLE();
        HAL_Delay(50);
    }
}







