/*
 * driving_loop.h
 *
 *  Created on: Mar 12, 2024
 *      Author: byo10
 */

#ifndef INC_DRIVING_LOOP_H_
#define INC_DRIVING_LOOP_H_

#include "main.h"
#include "can.h"
#include "errors.h"
#include"motor_controller.h"

//temp placeholders
#define MAX_PEDAL_ADC_VAL 0x0FF
#define MIN_PEDAL_ADC_VAL 0x000

//enable flag for torque control
#define TORQUE_CONTROL 0

#define MAX_PERMISSABLE_TORQUE
#define MAX_RPM 500

void driving_loop();



#endif /* INC_DRIVING_LOOP_H_ */
