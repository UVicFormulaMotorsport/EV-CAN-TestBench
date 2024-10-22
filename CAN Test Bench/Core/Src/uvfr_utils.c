/*
 * uvfr_utils.c
 *
 *  Created on: Oct 7, 2024
 *      Author: byo10
 */

/** @brief This function will initialize all of the uvfr wrappers, and system variables, as well as performing
 * internal diagnostics.
 *
 * This function has 3 phases.
 * 1) It loads up settings from flash if they exist.
 * 2) It starts up the uv_state_engine
 *
 */
#include "uvfr_utils.h"

enum uv_status_t init_uvfr_utils(){
	if(uvSettingsInit() != UV_OK){
		//If we end up here we have failed to initialize the settings in some catastrophic manner.
		//This is really bad.
	}else if(uvInitStateMachine() != UV_OK){

	}

	return UV_OK;
};

/**@brief This function is a soft-reboot of the uv_utils_backend and OS abstraction.
 *
 * The idea here is to basically start from a blank slate and boot up everything. So therefore we must:
 * Halt state machine.
 * Nuke vehicle operation related tasks.
 * Nuke the state machine
 * Nuke old settings
 *
 * reinitialize uv_utils
 *
 *
 */
enum uv_status_t uv_utils_reset(){
	return UV_OK;
}


//this works exactly the same way that malloc does
/* This should do the following:
 * 1)use _sbrk() to allocate space from the heap if neccessary
 * 2)if sufficient heap space is already in the user space, then allocate a
 * lil chunk of it to the user.
 *
 *
 *
 */
//void *uvMalloc(size_t size){
//
//}

//this works exactly the same way that free() does
//uv_status_t uvFree(void * mem){
//
//}

//this works the same way that realloc() does
//void *uvRealloc(){
//
//}

//uv_status_t __uv_register_task(){
//
//}
//
//uv_status_t uvUtilsInit(uv_init_struct){
//
//}
