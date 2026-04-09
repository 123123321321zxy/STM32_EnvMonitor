#include "driver_dht22.h"
#include <string.h>
#include <stdio.h>
#include "tim.h"

/**
 * @brief:产生dht22所需的复位脉冲，主机将总线拉低>=1ms,然后释放总线
 */
void DHT22_Reset(void)
{
	//开漏模式输出电平
    HAL_GPIO_WritePin(DHT22_GPIO_PORT, DHT22_GPIO_PIN, GPIO_PIN_RESET);

    //延时>=1ms
    delay_us(2000);
	
	//开漏模式，释放总线，高阻态相当于输入模式
	HAL_GPIO_WritePin(DHT22_GPIO_PORT, DHT22_GPIO_PIN, GPIO_PIN_SET);

	delay_us(20);//等待传感器就绪

}

/**
 * @brief 检查传感器的应答信号,传感器拉低总线80us,再拉高总线80us表示应答
 * @return 1 成功接受到应答信号，否则返回0
 */ 
uint8_t Is_Receive_Ack(void)
{
	int8_t time_out = 100;
	
	//等待下降沿
	while(HAL_GPIO_ReadPin(DHT22_GPIO_PORT, DHT22_GPIO_PIN) == GPIO_PIN_SET && time_out--)
	{
		delay_us(1);
	}
	if(time_out == 0) //超时
		return 0;

	time_out = 100;
	delay_us(80);//80us低电平
	
	//等下上升沿
	while(HAL_GPIO_ReadPin(DHT22_GPIO_PORT, DHT22_GPIO_PIN) == GPIO_PIN_RESET && time_out--)
	{
		delay_us(1);
	}
	
	if(time_out == 0)
		return 0;

	delay_us(80);//80us高电平
	return 1;
}




/**
 * @brief:读取传感器数据,传感器先拉低总线50us,再拉高总线26-28us表示读取到0或拉高70us表示读取到1
 */
static uint8_t DHT22_Data_Read_Bit(uint32_t* pData)
{
	int8_t count = 0;
	int8_t time_out = 100;
	
	//等待下降沿
	while(HAL_GPIO_ReadPin(DHT22_GPIO_PORT, DHT22_GPIO_PIN) == GPIO_PIN_SET && time_out--)
	{
		delay_us(1);
	}
	if(time_out == 0)
		return 0;

	
	delay_us(50);//50us低电平
	
	//计算高电平时间
	while(HAL_GPIO_ReadPin(DHT22_GPIO_PORT, DHT22_GPIO_PIN) == GPIO_PIN_SET && time_out--)
	{
		count++;
		delay_us(1);
	}
	if(time_out == 0)
		return 0;

	*pData = (count > 30) ? 1 : 0;
	return 1;
}

void DHT22_Data_Read(dht22_dataT* data)
{
	uint32_t tmp;
	uint32_t value1 = 0;
	uint32_t value2 = 0;
	
	for(int8_t i = 39; i >= 0; i--)
	{
		if(DHT22_Data_Read_Bit(&tmp) == 0)
			return;
		
		if(i > 7)
			value1 |= (tmp << (i - 8));
		else
			value2 |= (tmp << i);
			
	}
			
	data->humidity = ((value1 & 0xFFFF0000) >> 16);
	data->temperature = (value1 & 0x0000FFFF);
	data->checkSum = value2 & 0xFF;

}


/**
 * @brief:检查接收的校验和是否正确
 * @return true校验和正确，否则返回false
 */
uint8_t Check_CheckSum(dht22_dataT* pData)
{
    uint8_t result = 0;
    int tempSum = (pData->humidity >> 8) + (pData->humidity & 0xFF) + (pData->temperature >> 8) + (pData->temperature & 0xFF);

    if(pData->checkSum == (tempSum & 0xFF))
        result = 1;

    return result;
}


/**
 * @brief 获取传感器数据
 * @param data 保存获取的传感
 */
uint8_t Get_Dht22_Data(dht22_dataT* data)
{

	memset((uint8_t*)data, 0, sizeof(dht22_dataT));
	
    //产生复位信号
    DHT22_Reset();

    if(Is_Receive_Ack())
    {
        DHT22_Data_Read(data);

        if(Check_CheckSum(data)) //检查接收的校验和
            return 1;
    
    }
    return 0;

}





