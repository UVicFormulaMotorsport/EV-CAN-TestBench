/*
 * suspended.c
 *
 *  Created on: Mar 12, 2024
 *      Author: byo10
 */
#include "main.h"
#include "errors.h"
#include "motor_controller.h"
#include "bms.h"

extern enum vehicle_state_t vehicle_state;

//this is a stack with various conditions that need to get met in order to un-suspend the vehicle
uint8_t (*resume_conditions[8])() = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL}; // evil array of function pointers
uint8_t stack_ptr = 0;

uint16_t countdown_to_shutdown = 0;

void _add_condition(uint8_t (*cond)()){
	if(stack_ptr <7){
		resume_conditions[stack_ptr] = cond;
		stack_ptr++;

	}else{
		//cry, piss yourself, shit and cum
		throw_error(peepeepoopoo); //this will shut the car fully down
	}
}

_check_suspension_conditions(){

}

void suspended_loop(){
	//things that happen once upon suspension

	while(vehicle_state == suspended){
		//stuff and things I guess



	}

	switch(vehicle_state){
		case error: // things that should happen if we are transitioning from driving to error

			break;
		case driving: // things that should happen if we are transitioning from driving to suspended


			break;
		case ready:
			// things that should happen if we are transitioning from driving to ready, i.e. voluntarily powering down


			break;
		case init:
			//this one probably should not happen, ngl, but I am including code for the possibility regardless

			break;
		default: //Physically how did we get here?

			break;
		}



}
