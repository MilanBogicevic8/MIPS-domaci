/*
 * driver_uart.h
 *
 *  Created on: Jan 6, 2022
 *      Author: Marko Micovic
 */

#ifndef CORE_INC_DRIVER_UART_H_
#define CORE_INC_DRIVER_UART_H_

#include <stdint.h>

extern void UART_Init();

extern void UART_AsyncTransmitCharacter(char character);


#endif /* CORE_INC_DRIVER_UART_H_ */
