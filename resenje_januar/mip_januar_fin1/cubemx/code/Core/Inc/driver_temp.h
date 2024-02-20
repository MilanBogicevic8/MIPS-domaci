/*
 * driver_temp.h
 *
 *  Created on: Jan 6, 2022
 *      Author: Marko Micovic
 */

#ifndef CORE_INC_DRIVER_TEMP_H_
#define CORE_INC_DRIVER_TEMP_H_


extern float volatile tempSensor;
extern float volatile windvaneSensor;
extern float volatile brzinaSensor;
void temp_init();

#endif /* CORE_INC_DRIVER_TEMP_H_ */