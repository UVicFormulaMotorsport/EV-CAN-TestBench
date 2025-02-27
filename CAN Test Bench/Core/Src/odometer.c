/*
 * odometer.c
 *
 *  Created on: Nov 7, 2024
 *      Author: byo10
 */

#include "uvfr_utils.h"


uv_status initOdometer(void* args){

	uv_task_info* odom_task = uvCreateTask();

	if(odom_task == NULL){
				//Oh dear lawd
		return UV_ERROR;
	}


			//DO NOT TOUCH ANY OF THE FIELDS WE HAVENT ALREADY MENTIONED HERE. FOR THE LOVE OF GOD.
	odom_task->task_name = "Odometer";


	odom_task->task_function = odometerTask;
	odom_task->task_priority = osPriorityNormal;


	odom_task->stack_size = _UV_DEFAULT_TASK_STACK_SIZE;

	odom_task->active_states = UV_READY | UV_DRIVING;
	odom_task->suspension_states = UV_ERROR_STATE;
	odom_task->deletion_states = PROGRAMMING | UV_LAUNCH_CONTROL ;

	odom_task->task_period = 100;

	odom_task->task_args = NULL; //TODO: Add actual settings dipshit

	return UV_OK;
}


/** @brief, gotta know what the distance travelled is fam
 *
 */
void odometerTask(void* args){

	uv_task_info* params = (uv_task_info*) args; //Evil pointer typecast

		/**These here lines set the delay. This task executes exactly at the period specified, regardless of how long the task
		 * execution actually takes
		 *
		 @code*/
	TickType_t tick_period = pdMS_TO_TICKS(params->task_period); //Convert ms of period to the RTOS ticks
	TickType_t last_time = xTaskGetTickCount();
		/**@endcode */
	for(;;){
		if(params->cmd_data == UV_KILL_CMD){
			killSelf(params);
		}else if(params->cmd_data == UV_SUSPEND_CMD){
			suspendSelf(params);
		}
		vTaskDelayUntil( &last_time, tick_period);

		//whatever you want

		HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_13);

	}

}
