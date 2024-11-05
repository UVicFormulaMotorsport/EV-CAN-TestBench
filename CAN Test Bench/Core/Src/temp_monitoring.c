/*
 * temp_monitoring.c
 *
 *  Created on: Oct 31, 2024
 *      Author: byo10
 */


#include "uvfr_utils.h"
#include "gpio.h"

uv_status initTempMonitor(void * arguments){
	uv_task_info* tm_task = uvCreateTask();

		if(tm_task == NULL){
			//Oh dear lawd
			return UV_ERROR;
		}


		//DO NOT TOUCH ANY OF THE FIELDS WE HAVENT ALREADY MENTIONED HERE. FOR THE LOVE OF GOD.

		//dl_task->task_name = malloc(16*sizeof(char));
		//if(dl_task->task_name == NULL){
			//return UV_ERROR; //failed to malloc the name of the thing
		//}
		tm_task->task_name = "Driving_Loop";


		tm_task->task_function = StartDrivingLoop;
		tm_task->task_priority = osPriorityHigh;

		tm_task->instances = 1;
		tm_task->stack_size = _UV_DEFAULT_TASK_STACK_SIZE;

		tm_task->active_states = UV_DRIVING;
		tm_task->suspension_states = 0x00;
		tm_task->deletion_states = UV_READY | PROGRAMMING | UV_LAUNCH_CONTROL | UV_ERROR_STATE;

		tm_task->task_period = 200;

		tm_task->task_args = NULL; //TODO: Add actual settings dipshit

	return UV_OK;
}


/** @brief Monitors the temperatures of various points in the tractive system, and activates various cooling systems and such accordingly
 *
 * Atm, this is mostly serving as an example of a task
 *
 */
void tempMonitorTask(void* args){
	uv_task_info* params = (uv_task_info*) args; //Evil pointer typecast

	/**These here lines set the delay. This task executes exactly at the period
	 *
	 */
	TickType_t tick_period = pdMS_TO_TICKS(params->task_period); //Convert ms of period to the RTOS ticks
	TickType_t last_time = xTaskGetTickCount();
	for(;;){
		if(params->cmd_data == UV_KILL_CMD){

		}else if(params->cmd_data == UV_SUSPEND_CMD){

		}
		vTaskDelayUntil( &last_time, tick_period);

		HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_15);

	}
}
