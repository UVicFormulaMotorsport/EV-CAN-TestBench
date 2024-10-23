/*
 * uvfr_state_engine.c
 *
 *  Created on: Oct 15, 2024
 *      Author: byo10
 */

#include "uvfr_utils.h"

uv_task_id next_task_id = 0;

enum uv_vehicle_state_t vehicle_state = UV_BOOT;

enum uv_status_t uvCreateTask(uv_task_info** newtask){
	void * idk = malloc(1);
	free(idk);

	return UV_OK;

}

enum uv_status_t uvRegisterTask(uv_task_info* newtask){
	return UV_OK;
}

enum uv_status_t uvInitStateEngine(){
	return UV_OK;
}

enum uv_status_t uvStartStateMachine(){
	return UV_OK;
}

