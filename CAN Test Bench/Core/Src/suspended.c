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
enum vehicle_state_t previous_state;

//this is a stack with various conditions that need to get met in order to un-suspend the vehicle
int8_t (*resume_conditions[8])() = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL}; // evil array of function pointers
uint8_t stack_ptr = 0;

uint16_t countdown_to_shutdown = 0;

void _add_condition(int8_t (*cond)()){
	if(stack_ptr <7){
		resume_conditions[stack_ptr] = cond;
		stack_ptr++;

	}else{
		//cry, piss yourself, shit and cum
		_throw_error(peepeepoopoo); //this will shut the car fully down cause something aint right
	}
}

int8_t _check_suspension_conditions(){ //see if error gone and dip if it has been cleared
	for(int i = stack_ptr-1; i >= 0; i--){
		int8_t val = resume_conditions[i]();
		if(val == 1){//the resume conditions will return 1 if the error is gone
			stack_ptr--;
		}else if(val == 0){
			return 0; // whoops, error is still there. Try again later

		} else {//something evil has occurred, probably gotta shut down the vehicle now cause someone fucked the function up
			return val;
		}
	}

	if(stack_ptr == 0){
		return 1;
	}

}

void suspended_loop(){
	//things that happen once upon suspension

	while(vehicle_state == suspended){
		//stuff and things I guess
		int8_t sus_state = _check_suspension_conditions(); //checks to see if errors are gone now
		switch(sus_state){
		case 0:
			//all errors are present, keep going
			break;
		case 1:
			//all errors cleared

			break;
		default:

			break;
		}



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
