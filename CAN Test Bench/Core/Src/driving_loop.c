/*
 * Driving_Loop.c
 *
 *  Created on: Mar 12, 2024
 *      Author: byo10
 */

#include "main.h"
#include "errors.h"
#include "motor_controller.h"
#include "bms.h"
#include "driving_loop.h"

enum driving_mode_t driving_mode = normal;

extern enum vehicle_state_t vehicle_state;
extern uint16_t adc1_APPS1;
extern uint16_t adc1_APPS2;
extern uint16_t adc1_BPS1;
extern uint16_t adc1_BPS2;

//use for easy toggle between torque control and speed control


#if !TORQUE_CONTROL
//This function takes ADC values and outputs a 16 bit integer which represents the requested speed of the controller.
//Only works with linear speed control.
static int16_t linear_pedal_speed_map(){
	//check to make sure random values below the minimum dont put the car into reverse
	if(adc1_APPS1 < MIN_PEDAL_ADC_VAL) return 0;
	int16_t speed = 0;
	switch(driving_mode){
	case normal:

		speed = ((adc1_APPS1 - MIN_PEDAL_ADC_VAL)*(MAX_RPM)) / (MAX_PEDAL_ADC_VAL - MIN_PEDAL_ADC_VAL);
		break;
	default:

		break;
	}

	return speed;

}

#else

int16_t pedal_torque_map(){
	int16_t torque = 0;

}

#endif


void driving_loop(){//this is where the main driving stuff happens
	//code that executes once upon entry to driving loop goes here


	//Howdy folks, here is the driving loop. Everything here is fun and cool. the way we
	//exit the driving loop, is if the vehicle state changes
	while(vehicle_state == driving){
		//Things that occur for us to drive


		/* Step 1: Pedal position to motor controller output
		 * Step 2: Send to motor controller
		 *
		 *
		 *
		 */

		linear_pedal_speed_map();



	}

	switch(vehicle_state){
	case error: // things that should happen if we are transitioning from driving to error

		break;
	case suspended: // things that should happen if we are transitioning from driving to suspended


		break;
	case ready:
		// things that should happen if we are transitioning from driving to ready, i.e. voluntarily powering down


		break;
	case init:

		break;
	default: //Physically how did we get here?

		break;
	}


}
