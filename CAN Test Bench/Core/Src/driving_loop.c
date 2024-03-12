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

extern enum vehicle_state_t vehicle_state;

void driving_loop(){//this is where the main driving stuff happens
	//code that executes once upon entry to driving loop goes here


	//Howdy folks, here is the driving loop. Everything here is fun and cool. the way we
	//exit the driving loop, is if the vehicle state changes
	while(vehicle_state == driving){
		//Things that occur for us to drive



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
