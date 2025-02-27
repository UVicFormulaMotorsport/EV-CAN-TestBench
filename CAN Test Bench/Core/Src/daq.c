#define _SRC_UVFR_DAQ

#include "uvfr_utils.h"
#include "daq.h"


void* param_LUT[126]; //horrible and evil. Absolutely abysmal.

void deleteParamList(){

}

void deleteDaqSubTask(){

}

uv_status startDaqSubTasks(){
	return UV_ERROR;
}

uv_status stopDaqSubTasks(){
	return UV_ERROR;
}

/** @brief initializes the master DAQ task, all that fun stuff. This task probably manages a while plethora of smaller tasks
 *
 * This is a fairly standard function
 *
 */
uv_status initDaqTask(void * args){
	uv_task_info* daq_task = uvCreateTask();

	if(daq_task == NULL){
					//Oh dear lawd
		return UV_ERROR;
	}


				//DO NOT TOUCH ANY OF THE FIELDS WE HAVENT ALREADY MENTIONED HERE. FOR THE LOVE OF GOD.
	daq_task->task_name = "daq";

	daq_task->task_function = daqMasterTask;
	daq_task->task_priority = osPriorityNormal;

	daq_task->stack_size = _UV_DEFAULT_TASK_STACK_SIZE;

	daq_task->active_states = UV_READY | UV_DRIVING | UV_ERROR_STATE;
	daq_task->suspension_states = 0x00;
	daq_task->deletion_states = PROGRAMMING | UV_LAUNCH_CONTROL ;

	daq_task->task_period = 20; //measured in ms

	daq_task->task_args = NULL; //TODO: Add actual settings dipshit


	return UV_OK;
}

/**
 *
 */
void daqMasterTask(void* args){
	uv_task_info* params = (uv_task_info*) args; //Evil pointer typecast



		/**These here lines set the delay. This task executes exactly at the period specified, regardless of how long the task
		 * execution actually takes
		 *
		 @code*/
	TickType_t tick_period = pdMS_TO_TICKS(params->task_period); //Convert ms of period to the RTOS ticks
	//TickType_t last_time = xTaskGetTickCount();		/**@endcode */
	for(;;){
		if(params->cmd_data == UV_KILL_CMD){
			killSelf(params);
		}else if(params->cmd_data == UV_SUSPEND_CMD){
			suspendSelf(params);
		}
		vTaskDelay(tick_period);

		//HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_15);
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)){
			vTaskDelay(25);
			while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)){

			}


			if(vehicle_state == UV_READY){
				changeVehicleState(UV_DRIVING);
			}else if(vehicle_state == UV_DRIVING){
				changeVehicleState(UV_ERROR_STATE);
			}else if(vehicle_state == UV_ERROR_STATE){
				changeVehicleState(UV_READY);
			}


		}

	}
}

void daqSubTask(void* args){

	for(;;){

	}
}


