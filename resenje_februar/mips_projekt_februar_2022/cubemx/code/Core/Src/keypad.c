/*
 * keypad.c
 *
 *  Created on: Jan 9, 2024
 *      Author: Korisnik
 */


#include "keypad.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "gpio.h"

static TimerHandle_t timer;
static TaskHandle_t task;
static char Key_map[4][3]={
		{'1','2','3'},
		{'4','5','6'},
		{'7','8','9'},
		{'*','0','#'}
};

char volatile keys[2];
int passed=0;
uint32_t KEY_Released=1;
uint32_t volatile temp_granica=30;

static void KEY_Task(void* parameters){
	keys[0]='?';
	keys[1]='?';
	while(1){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		//vTaskDelay(pdMS_TO_TICKS(50));
		if(KEY_Released==1){
			GPIOB->ODR=0;
			for(int row=0;row<4;row++){
				GPIOB->ODR=(0x01<<row);
				uint32_t state=(GPIOB->IDR>>4)&0x07;
				for(int column=0;column<3;column++){
					if((state&(0x01<<column))!=0){
						keys[passed]=Key_map[row][column];
						if(passed==1){
							temp_granica=(keys[0]-'0')*10+(keys[1]-'0');
							keys[0]='?';
							keys[1]='?';
						}
						passed=1-passed;
						KEY_Released=0;
						xTimerStart(timer,portMAX_DELAY);
					}
				}
			}
			GPIOB->ODR=0x0F;
		}

	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin==GPIO_PIN_7){
		BaseType_t woken=pdFALSE;
		vTaskNotifyGiveFromISR(task, &woken);
		portYIELD_FROM_ISR(woken);
	}

}
void key_timer(TimerHandle_t t){
	if(!KEY_Released){
		if(((GPIOB->IDR>>4)&0x07)==0){
			KEY_Released=1;
		}else{
			xTimerStart(timer,portMAX_DELAY);
		}
	}
}


void Key_init(){
	GPIOB->ODR=0x0F;
	xTaskCreate(KEY_Task, "key task", 128, NULL, 5, &task);
	timer=xTimerCreate("timer key", pdMS_TO_TICKS(50), pdFALSE, NULL, key_timer);
}

