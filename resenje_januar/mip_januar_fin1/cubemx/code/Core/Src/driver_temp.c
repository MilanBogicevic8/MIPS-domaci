/*
 * driver_temp.c
 *
 *  Created on: Jan 6, 2022
 *      Author: Marko Micovic
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "adc.h"
#include "gpio.h"
#include "tim.h"



// MAX VOLTAGE = 5V
#define MAX_VOLTAGE 5.0
// ADC RESOLUTION = 2^12 = 4096 (12-bit resolution)
#define RESOLUTION 4096.0


static TaskHandle_t task;

float volatile tempSensor=0;
float volatile windvaneSensor=0;
float volatile brzinaSensor=0;
float previousBrzina=0;
int overflow=0;

static float voltageThresholds[16] =
{ 0.52635, 0.8136, 1.0611, 1.36555, 1.74565, 1.9979, 2.4099, 2.88215, 3.3071,
		3.7009, 3.95155, 4.24015, 4.4637, 4.56805, 4.6344, 5 };
static float windBearings[16] =
{ 270, 315, 292.5, 0, 337.5, 225, 247.5, 45, 22.5, 180, 202.5, 135, 157.5, 90,
		67.5, 112.5 };
static float voltage2WindBearing(float voltage)
{
	for (uint32_t i = 0; i < 15; i++)
	{
		if (voltage < voltageThresholds[i])
		{
			return windBearings[i];
		}
	}
	return windBearings[15];
}


static void temp_task(void* parameters){
	while(1){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		HAL_ADC_Start_IT(&hadc1);
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		tempSensor=HAL_ADC_GetValue(&hadc1);
		tempSensor*=100;
		tempSensor*=MAX_VOLTAGE;
		tempSensor/=RESOLUTION;

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		HAL_ADC_Start_IT(&hadc1);
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		windvaneSensor=HAL_ADC_GetValue(&hadc1);
		windvaneSensor*=MAX_VOLTAGE;
		windvaneSensor/=RESOLUTION;
		windvaneSensor=voltage2WindBearing(windvaneSensor);

		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(hadc->Instance!=hadc1.Instance) return;

	BaseType_t woken=pdFALSE;
	vTaskNotifyGiveFromISR(task, &woken);
	portYIELD_FROM_ISR(woken);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance!=htim1.Instance) return;

	brzinaSensor=htim1.Instance->CCR1+overflow*65535-previousBrzina;
	previousBrzina=htim1.Instance->CCR1;
	overflow=0;
	brzinaSensor=1000/brzinaSensor*2.4;
}

void temp_init(){
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);

	xTaskCreate(temp_task, "temp task", 128, NULL, 2, &task);
}
