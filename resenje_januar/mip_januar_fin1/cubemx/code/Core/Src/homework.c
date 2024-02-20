/*
 * homework.c
 *
 *  Created on: Jan 6, 2022
 *      Author: Marko Micovic
 */

#include "homework.h"

#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"
#include "timers.h"


#include <stdlib.h>
#include <string.h>

#include "driver_lcd.h"
#include "driver_uart.h"
#include "driver_motor.h"
#include "driver_temp.h"

static TaskHandle_t task;

static char tempText[4];
static char windvaveText[4];
static char brzinaText[4];

static void homeworkTask(void *parameters)
{
	char messageTemp[8]="Temper: ";
	char messageWind[8]="Azimut: ";
	char messageBrzina[8]="Brzina: ";
	LCD_CommandEnqueue(LCD_INSTRUCTION, 0x80|0x10);
	for(int i=0;i<8;i++){
		LCD_CommandEnqueue(LCD_DATA, messageTemp[i]);
	}
	LCD_CommandEnqueue(LCD_INSTRUCTION, 0x80|0x00);
	for(int i=0;i<8;i++){
		LCD_CommandEnqueue(LCD_DATA, messageWind[i]);
	}
	LCD_CommandEnqueue(LCD_INSTRUCTION, 0x80|0x40);
	for(int i=0;i<8;i++){
		LCD_CommandEnqueue(LCD_DATA, messageBrzina[i]);
	}

	while(1){
		itoa(tempSensor,tempText,10);
		itoa(windvaneSensor,windvaveText,10);
		itoa(brzinaSensor,brzinaText,10);

		LCD_CommandEnqueue(LCD_INSTRUCTION, 0x80|0x18);
		for(int i=0;i<strlen(tempText);i++){
			LCD_CommandEnqueue(LCD_DATA, tempText[i]);
		}
		LCD_CommandEnqueue(LCD_INSTRUCTION, 0x80|0x08);
		for(int i=0;i<strlen(windvaveText);i++){
			LCD_CommandEnqueue(LCD_DATA, windvaveText[i]);
		}
		LCD_CommandEnqueue(LCD_INSTRUCTION, 0x80|0x48);
		for(int i=0;i<strlen(brzinaText);i++){
			LCD_CommandEnqueue(LCD_DATA, brzinaText[i]);
		}

		for(int i=0;i<strlen(windvaveText);i++){
			UART_AsyncTransmitCharacter(windvaveText[i]);
		}
		UART_AsyncTransmitCharacter('/');
		for(int i=0;i<strlen(brzinaText);i++){
			UART_AsyncTransmitCharacter(brzinaText[i]);
		}
		UART_AsyncTransmitCharacter('/');
		for(int i=0;i<strlen(tempText);i++){
			UART_AsyncTransmitCharacter(tempText[i]);
		}

		vTaskDelay(pdMS_TO_TICKS(200));

		LCD_CommandEnqueue(LCD_INSTRUCTION, 0x80|0x18);
		for(int i=0;i<strlen(tempText);i++){
			LCD_CommandEnqueue(LCD_DATA, ' ');
		}
		LCD_CommandEnqueue(LCD_INSTRUCTION, 0x80|0x08);
		for(int i=0;i<strlen(windvaveText);i++){
			LCD_CommandEnqueue(LCD_DATA, ' ');
		}
		LCD_CommandEnqueue(LCD_INSTRUCTION, 0x80|0x48);
		for(int i=0;i<strlen(brzinaText);i++){
			LCD_CommandEnqueue(LCD_DATA, ' ');
		}

		for(int i=0;i<strlen(windvaveText);i++){
			UART_AsyncTransmitCharacter('\b');
		}
		UART_AsyncTransmitCharacter('\b');
		for(int i=0;i<strlen(brzinaText);i++){
			UART_AsyncTransmitCharacter('\b');
		}
		UART_AsyncTransmitCharacter('\b');
		for(int i=0;i<strlen(tempText);i++){
			UART_AsyncTransmitCharacter('\b');
		}
	}
}

void led_dioda(TimerHandle_t tim){
	if(brzinaSensor<50){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_RESET);
	}else{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_13);
	}
}
void homeworkInit()
{
	LCD_Init();
	UART_Init();
	temp_init();
	xTaskCreate(homeworkTask,"hom task", 128, NULL, 2, &task);
	TimerHandle_t timi=xTimerCreate("timi handle", pdMS_TO_TICKS(500), pdTRUE, NULL, led_dioda);
	xTimerStart(timi,portMAX_DELAY);

}

