/*
 * homework.c
 *
 *  Created on: Jan 6, 2022
 *      Author: Marko Micovic
 */

#include "homework.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdlib.h>
#include <string.h>

#include "tim.h"
#include "timers.h"
#include "driver_lcd.h"
#include "driver_uart.h"
#include "driver_motor.h"
#include "driver_temp.h"

uint32_t previous=0;
uint32_t current=1;
uint32_t overflow=0;
static uint32_t brzina;
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance!=htim1.Instance) return;
	current=htim->Instance->CCR1+overflow*65535-previous;
	previous=htim->Instance->CCR1;
	overflow=0;
	brzina=1000/current*2.4;
}

FanState fanState = TURNED_OFF;

static uint32_t tempValue;
static uint32_t windvaneValue;
static char tempText[4];
static char windvaneText[10]="123";
static char brzinaText[10];
static void homeworkTask(void *parameters)
{
	char message[16] = "Temperatura: ";
	LCD_CommandEnqueue(LCD_INSTRUCTION,
	LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x10);
	for (uint32_t i = 0; i < 16; i++)
	{
		LCD_CommandEnqueue(LCD_DATA, message[i]);
		//UART_AsyncTransmitCharacter(message[i]);
	}

	char messageAzimur[8]="Azimut: ";
	LCD_CommandEnqueue(LCD_INSTRUCTION, 0x80|0x00);

	for(int i=0;i<8;i++){
		LCD_CommandEnqueue(LCD_DATA, messageAzimur[i]);
	}

	char messageBrzina[8]="Brzina: ";
	LCD_CommandEnqueue(LCD_INSTRUCTION, 0x80|0x40);
	for(int i=0;i<8;i++){
		LCD_CommandEnqueue(LCD_DATA, messageBrzina[i]);
	}
	while (1)
	{
		tempValue = TEMP_GetCurrentValue();
		itoa(tempValue, tempText, 10);
		windvaneValue=(int)Windwane_GetCurrentValue();
		itoa(windvaneValue,windvaneText,10);
		itoa(brzina,brzinaText,10);
		// control fan speed
		FanState fanStateTarget;
		if (tempValue < 30)
		{
			fanStateTarget = TURNED_OFF;
		}
		else if (tempValue < 35)
		{
			fanStateTarget = SLOW;
		}
		else
		{
			fanStateTarget = FAST;
		}
		for (uint32_t i = 0; i < abs(fanStateTarget - fanState); i++)
		{
			if (fanStateTarget > fanState)
			{
				MOTOR_SpeedIncrease();
			}
			else
			{
				MOTOR_SpeedDecrease();
			}
		}
		fanState = fanStateTarget;

		// write temperature value
		LCD_CommandEnqueue(LCD_INSTRUCTION,
		LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x1D);
		for (uint32_t i = 0; i < strlen(tempText); i++)
		{
			LCD_CommandEnqueue(LCD_DATA, tempText[i]);
			//UART_AsyncTransmitCharacter(tempText[i]);
		}
		//write azimut value

		LCD_CommandEnqueue(LCD_INSTRUCTION, 0x80|0x07);
		for(int i=0;i<strlen(windvaneText);i++){
			LCD_CommandEnqueue(LCD_DATA, windvaneText[i]);
		}

		LCD_CommandEnqueue(LCD_INSTRUCTION, 0x80|0x48);
		for(int i=0;i<strlen(brzinaText);i++){
			LCD_CommandEnqueue(LCD_DATA, brzinaText[i]);
		}

		for(uint32_t i=0;i<strlen(windvaneText);i++){
			UART_AsyncTransmitCharacter(windvaneText[i]);
		}
		UART_AsyncTransmitCharacter('/');
		for(uint32_t i=0;i<strlen(brzinaText);i++){
			UART_AsyncTransmitCharacter(brzinaText[i]);
		}
		UART_AsyncTransmitCharacter('/');
		for(uint32_t i=0;i<strlen(tempText);i++){
			UART_AsyncTransmitCharacter(tempText[i]);
		}

		vTaskDelay(pdMS_TO_TICKS(200));

		// clear temperature value
		UART_AsyncTransmitCharacter('\b');
		UART_AsyncTransmitCharacter('\b');
		for(uint32_t i=0;i<strlen(brzinaText);i++){
			UART_AsyncTransmitCharacter('\b');
		}
		for(uint32_t i=0;i<strlen(windvaneText);i++){
			UART_AsyncTransmitCharacter('\b');
		}
		LCD_CommandEnqueue(LCD_INSTRUCTION,
		LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x1D);
		for (uint32_t i = 0; i < strlen(tempText); i++)
		{
			LCD_CommandEnqueue(LCD_DATA, ' ');
			UART_AsyncTransmitCharacter('\b');
		}


	}
}

void led_dioda(TimerHandle_t t){
	if(brzina<50){
		HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_13);
	}else{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_RESET);
	}
}
void homeworkInit()
{
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
	LCD_Init();
	UART_Init();
	MOTOR_Init();
	TEMP_Init();

	xTaskCreate(homeworkTask, "homeworkTask", 64, NULL, 5, NULL);
	TimerHandle_t timi=xTimerCreate("timi", pdMS_TO_TICKS(500), pdTRUE, NULL, led_dioda);
	xTimerStart(timi,portMAX_DELAY);
}

