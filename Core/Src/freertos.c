/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "queue.h"
#include "driver_key.h"
#include "driver_oled.h"
#include "driver_dht22.h"
#include "driver_led.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
TaskHandle_t g_xSensorTaskHandle;
TaskHandle_t g_xOledTaskHandle;
TaskHandle_t g_xKeyTaskHandle;
TaskHandle_t g_xLedTaskHandle;

QueueHandle_t g_xOledQueueHandle;
QueueHandle_t g_xKeyQueueHandle;

extern char g_xStateList[][10];

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void KeyTask(void* params);
void SensorTask(void* params);
void OledTask(void* params);
void LedTask(void* params);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
	g_xOledQueueHandle = xQueueCreate(OLED_QUEUE_LENGTH, sizeof(Oled_DataT));
	g_xKeyQueueHandle = xQueueCreate(KEY_QUEUE_LEN, sizeof(Key_DataT));
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  
  xTaskCreate(KeyTask, "KeyTask", 128, NULL, osPriorityNormal+1, &g_xKeyTaskHandle);
  xTaskCreate(SensorTask, "SensorTask", 128, NULL, osPriorityNormal+2, &g_xSensorTaskHandle);
  xTaskCreate(OledTask, "OledTask", 128, NULL, osPriorityNormal+1, &g_xOledTaskHandle);
  xTaskCreate(LedTask, "LedTask", 128, NULL, osPriorityNormal, &g_xLedTaskHandle);
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  vTaskDelete(NULL);
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void KeyTask(void* params)
{
	Key_DataT rdata;
	Oled_DataT idata;
	uint16_t pre_time;
	uint8_t key_event = 0;

	while(1)
	{
		xQueueReceive(g_xKeyQueueHandle, &rdata, portMAX_DELAY); //等待按下事件
		
		if(rdata.event_type == KEY_EVENT_TYPE_PRESS)//按下事件
		{
			pre_time = rdata.cnt;   //记录按下事件的时间戳
			
			//等待松开事件,松开事件1s超时
			if(xQueueReceive(g_xKeyQueueHandle, &rdata, KEY_EVENT_TIME_OUT))
			{
				if(rdata.cnt - pre_time < KEY_LONG_PRESS_TIME)//短按事件
					key_event = KEY_EVENT_SHORT_PRESS;
				else //长按事件
					key_event = KEY_EVENT_LONG_PRESS;
			}
			else//超时事件
				key_event = KEY_EVENT_LONG_PRESS;
		}
		
				

		 //当前发生短按事件且采集任务处于挂起
		if(key_event == KEY_EVENT_SHORT_PRESS && eTaskGetState(g_xSensorTaskHandle) == eSuspended)//若当前任务被挂起
			vTaskResume(g_xSensorTaskHandle);
		  
		//超时将强制为长按事件，长按挂起传感器任务
		if(key_event == KEY_EVENT_LONG_PRESS && eTaskGetState(g_xSensorTaskHandle) != eSuspended)
		{
			idata.mState = SYS_STATE_STOPPED;
			vTaskSuspend(g_xSensorTaskHandle);
			xQueueSend(g_xOledQueueHandle, &idata, portMAX_DELAY);
		}
		
		if(key_event != 0)
		{
			//清除状态数据
			key_event = 0;
			pre_time = 0;
			key_event = 0;
		}
		
		vTaskDelay(1);

	}
}


void SensorTask(void* params)
{
	dht22_dataT sensor_data;
	Oled_DataT idata;
	
	idata.mState = SYS_STATE_READY;
	xQueueSend(g_xOledQueueHandle, &idata, 0);
	vTaskSuspend(NULL);
	
	while(1)
	{		
		if(Get_Dht22_Data(&sensor_data))
		{	
			if(sensor_data.humidity == 0 || sensor_data.temperature == 0)
				continue;
			
			idata.mState = SYS_STATE_RUNNING;
			idata.hum = sensor_data.humidity;
			idata.tmp = sensor_data.temperature;
			xQueueSend(g_xOledQueueHandle, &idata, 0);
			xTaskNotify(g_xLedTaskHandle, idata.tmp, eSetValueWithOverwrite);
		}
		
		vTaskDelay(2000);
	}
	

}

 
void OledTask(void* params)
{
	Oled_DataT rdata;
	char* pcState;
	float tmp = 0;
	float hum = 0;
	
	while(1)
	{
		xQueueReceive(g_xOledQueueHandle, &rdata,portMAX_DELAY);
		pcState = (char*)&g_xStateList[rdata.mState - 1];
		OLED_Clear();
		OLED_ShowString(0, 0, "State:");
		
		switch(rdata.mState)
		{	
			case SYS_STATE_RUNNING:
			{
				tmp = rdata.tmp/10.0f;
				hum = rdata.hum/10.0f;
				OLED_ShowString(48, 0, pcState);
				OLED_ShowString(0, 16, "Tmp:      C");
				OLED_ShowFloat(32, 16, tmp, 1);
				OLED_ShowChar(72, 16, SYM_DEGREE);
				OLED_ShowString(0, 32, "Hum:     %RH");
				OLED_ShowFloat(32, 32, hum, 1);
				break;
			}
			
			case SYS_STATE_STOPPED:
			{
				OLED_ShowString(48, 0, pcState);
				break;
			}
			
			case SYS_STATE_READY:
			{
				OLED_ShowString(48, 0, pcState);
				break;
			}
		}

		
		vTaskDelay(5);
	}
}


/**
*
*/
void LedTask(void* params)
{
	uint8_t ledSate = 0;
	uint32_t t;
	float tmp;
	
	while(1)
	{
		xTaskNotifyWait(0, 0x0000FFFF, &t, portMAX_DELAY);
		tmp = t / 10.0f;
		
		if((tmp > HIGH_TEMPERURATUE_LIMIT) && ledSate != LED_LIGHT) 
			ledSate = LED_LIGHT;
		
		if((tmp <= HIGH_TEMPERURATUE_LIMIT) && ledSate == LED_LIGHT) 
			ledSate = 0;
		
		if(ledSate == LED_LIGHT)
			LED_TOGGLE();
		else
			LED_OFF();
		
		vTaskDelay(5);
	}
}



/* USER CODE END Application */

