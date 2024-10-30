/*
 * uvfr_state_engine.c
 *
 *  Created on: Oct 15, 2024
 *      Author: byo10
 */

#define UVFR_STATE_MACHINE_IMPLIMENTATION

#include "uvfr_utils.h"

uv_task_id _next_task_id = 0;
uv_task_info* _task_register = NULL;
uv_task_info* _final_task = NULL;

QueueHandle_t state_change_queue = NULL;

enum uv_vehicle_state_t vehicle_state = UV_BOOT;
enum uv_vehicle_state_t previous_state = UV_BOOT;


/** @brief This function gets called when you want to create a task, and register it
 * with the task register. Theres some gnarlyness here, but not unacceptable levels.
 * Pray this thing doesn't hang itself.
 *
 */
uv_task_info *uvCreateTask(){
	if(_task_register == NULL){
		_task_register = malloc(sizeof(uv_task_info));
		if(_task_register == NULL){ //UH OH
			return NULL;
		}
		_final_task = _task_register; //ensure the tail is ok
		_final_task->task_id = _next_task_id;
		_next_task_id++; //ensure next has a unique ID

		_final_task->next = NULL;
		_final_task->task_state = UV_TASK_NOT_STARTED;

		return _final_task;
	//}else if(){

	}else{
		_final_task->next = malloc(sizeof(uv_task_info));
		if(_final_task->next == NULL){//get that null pointer BS outta here
			return NULL;
		}

		_final_task = _final_task->next;

		_final_task->task_id = _next_task_id;
		_next_task_id++;

		_final_task->next = NULL;
		_final_task->task_state = UV_TASK_NOT_STARTED;

		return _final_task;
	}

	return NULL;
}

uv_status addTaskToTaskRegister(){
	return 0;
}

/** @brief: This function can be used to retrieve one of our tasks, using it's ID
 *
 */
uv_task_info* uvGetTaskById(uv_task_id id){
	uv_task_info* curr = _task_register;

	if(_task_register == NULL){return NULL;}

	while(curr->next != NULL){
		if(curr->task_id == id){
			return curr;
		}

	}

	return NULL;
}


/** @brief: This is a function that starts tasks which are already registered in the system
 *
 */
enum uv_status_t uvStartTask(uv_task_info* t){
	const osThreadDef_t new_thread = {t->task_name,t->task_function,t->task_priority,t->instances,t->stack_size};
	t->task_handle = osThreadCreate(&new_thread,t);


	/** The function @c osThreadCreate returns null if it fails to create a thread.
	 * If that happens, we really do have a problem, so we will be returning an error value
	 *
	 */
	if(t->task_handle == NULL){ //WTF
		return UV_ERROR;
	}

	return UV_OK;
}


/** @brief: unga bunga no tunga something?
 *
 * Who even fucking knows at this point
 *
 */
enum uv_status_t uvInitStateEngine(){
	return UV_OK;
}


/** Actually starts up the state engine to do state engine things
 *
 */
enum uv_status_t uvStartStateMachine(){
	return UV_OK;
}

/**
 *
 */
enum uv_status_t killEmAll(){
	return UV_OK;
}

enum uv_status_t uvDeleteTask(){
	return UV_OK;
}

enum uv_status_t uvSuspendTask(){
	return UV_OK;
}

enum uv_status_t uvDeInitStateEngine(){
	return UV_OK;
}

enum uv_status_t updateRunningTasks(){
	return UV_OK;
}

typedef struct state_change_daemon_args{
	osThreadId meta_task_handle;
}state_change_daemon_args;

state_change_daemon_args scd_args = {NULL};


/** @brief Function for changing the state of the vehicle, as well as the list of active + inactive tasks.
 *
 * This function also changes out the tasks that are executing.
 *
 */
enum uv_status_t changeVehicleState(uint16_t state){
	if(state == vehicle_state){
		return UV_ABORTED;
	}

	previous_state = vehicle_state;
	vehicle_state = state;

//	int state_diff = previous_state-vehicle_state;
//
//	switch(state_diff){
//
//	}

	return UV_OK;
}

void uvPanic(char* msg, uint8_t msg_len){
	//ruh roh, something has gone a little bit fucky wucky


}


/** @brief: This collects all the data changing from different tasks, and makes sure that everything works properly
 * 	@attention: DO NOT EVER JUST CALL THIS FUNCTION. THIS SHOULD ONLY BE CALLED FROM changeVehicleState
 *
 */
void _stateChangeDaemon(void * args){

	//for(int i = 0, i < _LONGEST_SC_TIME/_SC_DAEMON_PERIOD){

	//}

}

