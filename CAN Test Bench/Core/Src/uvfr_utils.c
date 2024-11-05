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

extern TaskHandle_t init_task_handle;


/** @brief: Function that initializes all of the car's stuff.
 *
 * This is an RTOS task, and it serves to setup all of the car's different functions.
 * at this point in our execution, we have already initialized all of our favorite hardware
 * peripherals using HAL. Now we get to configure our convoluted system of OS-level
 * settings and state machines.
 *
 *
 */
void uvInit(void * arguments){
	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_15);
	//GPIOA->ODR = 0xFFFFFFFF;
	//osDelay(500);

	char* error_msg = NULL;
	//uint8_t msg_length = 0;

	/** First on the block is our settings. The uv_settings are a bit strange, in the following way.
	 * We will check if we have saved custom settings, or if these settings are the default or not.
	 * It will then perform a checksum on the settings, and validate them to ensure they are safe
	 * If it fails to validate the settings, it will attempt to return to factory default.
	 *
	 * If it is unable to return even to factory default settings, then we are in HUGE trouble, and
	 * some catastrophic bug has occurred. If it fails to even start this, it will not be safe to drive
	 * We must therefore panic.
	 *
	 */

//	if(uvSettingsInit() != UV_OK){
//		_uvInitPanic();
//
//		/** Next up we will attempt to initialize the state engine. If this fails, then we are in another case where we are genuinely unsafe to drive.
//		 * This will create the prototypes for a bajillion tasks that will be started and stopped. Which tasks are currently running,
//		 * depends on the whims of the state engine. Since the state engine is critical to our ability to handle errors and implausibilitys,
//		 * we cannot proceed without a fully operational state engine.
//		 */
//	}else if(uvInitStateEngine() != UV_OK){
//		_uvInitPanic();
//	}

	/** Once we have initialized the state engine, what we want to do is create the prototypes of all the
	 * tasks that will be running.
	 *
	 */

	/** Now we are going to create a bunch of tasks that will initialize our car's external devices.
	 * The reason that these are RTOS tasks, is that it takes a buncha time to verify the existance of some devices.
	 * As a direct result, we can sorta just wait around and check that each task sends a message confirming that it has successfully executed. :)
	 * However, first we need to actually create a Queue for these tasks to use
	 * @code
	 */
//	QueueHandle_t init_validation_queue = xQueueCreate(8,sizeof(uv_init_task_response));
//	if(init_validation_queue == NULL){
//
//	}

	/** @endcode
	 * The next big thing on our plate is checking the status of all external devices we need, and initializing them with appropriate parameters.
	 * These are split into tasks because it takes a bit of time, especially for devices that need to be configured via CANBus such as the motor controller.
	 * That is why it is split the way it is, to allow these to run somewhat concurrently
	 * @code */

//	BaseType_t retval;
//	//osThreadDef_t MC_init_thread = {"MC_init",MC_Startup,osPriorityNormal,128,0};
//	uv_init_task_args* MC_init_args = malloc(sizeof(uv_init_task_args));
//	MC_init_args->init_info_queue = init_validation_queue;
//	MC_init_args->specific_args = &(current_vehicle_settings->bms_settings);
//	//MC_init_args->meta_task_handle = osThreadCreate(&MC_init_thread,MC_init_args);
//	//vTaskResume( MC_init_args->meta_task_handle );
//	retval = xTaskCreate(MC_Startup,"MC_init",1024,MC_init_args,osPriorityAboveNormal,&(MC_init_args->meta_task_handle));
//	if(retval != pdPASS){
//		//FUCK
//		error_msg = "bruh";
//	}
	/** @endcode
	 * This thread is for initializing the BMS
	 * @code */


	//osThreadDef_t BMS_init_thread = {"BMS_init",BMS_Init,osPriorityNormal,128,0};
//	uv_init_task_args* BMS_init_args = malloc(sizeof(uv_init_task_args));
//	BMS_init_args->init_info_queue = init_validation_queue;
//	BMS_init_args->specific_args = &(current_vehicle_settings->bms_settings);
	//BMS_init_args->meta_task_handle = osThreadCreate(&BMS_init_thread,BMS_init_args);
//	retval = xTaskCreate(MC_Startup,"BMS_init",1024,BMS_init_args,osPriorityAboveNormal,&(BMS_init_args->meta_task_handle));
//	if(retval != pdPASS){
//		//FUCK
//		error_msg = "bruh";
//	}
	/** @endcode
	* This variable is a tracker that tracks which devices have successfully initialized
	* @code*/


	uint16_t ext_devices_status = 0x000F; //Tracks which devices are currently setup


	/** @endcode
	 *
	 * Wait for all the spawned in tasks to do their thing. This should not take that long, but we wanna be sure that everything is chill
	 * If we are say, missing a BMS, then it will not allow you to proceed past the initialisation step
	 * This is handled by a message buffer, that takes inputs from all of the tasks
	 *
	 */

	if(error_msg != NULL){
		while(1){

		}
	}

//	if(BMS_init_args->meta_task_handle == NULL || MC_init_args->meta_task_handle == NULL){
//		_uvInitPanic();
//	}

	//uv_init_task_response rx_response;
	TickType_t last_time = xTaskGetTickCount();
	for(int i = 0; i< MAX_INIT_TIME/INIT_CHECK_PERIOD; i++){
		vTaskDelayUntil(&last_time,pdMS_TO_TICKS(INIT_CHECK_PERIOD));

//		while(xQueueReceive(init_validation_queue,&rx_response,0) == pdPASS){
//			if(rx_response.status == UV_OK){
//				ext_devices_status &= ~(_BV_16(rx_response.device));
//			}else{
//				error_msg = rx_response.errmsg;
//				msg_length = rx_response.nchar;
//
//			}
//
//		}



//		if(ext_devices_status == 0){
//			//SUCCESS
//			//Set vehicle state to "idle"
//			if(changeVehicleState(UV_READY) == UV_OK){
//				break;
//			}else{
//				uvPanic("Unable To Change State to Ready",0);
//			}
//
//			//GTFO
//			break;
//		}else{
//
//		}
	}
	//If we get here, then we have timed out

	initialisation_failure:


	/** Clean up, clean up, everybody clean up, clean up, clean up, everybody do your share!
	 *	The following code cleans up all the threads that were running, and free up used memory
	 *
	 */

	//vTaskDelete(MC_init_args->meta_task_handle);
	//free(MC_init_args);

	//vTaskDelete(BMS_init_args->meta_task_handle);
	//free(BMS_init_args);

	//vQueueDelete(init_validation_queue);
	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_15);
	vTaskDelete(init_task_handle);
	return;

}

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
enum uv_status_t uvUtilsReset(){
	return UV_OK;
}


void setup_extern_devices(void * argument){


}


/** @brief Low Level Panic, that does not require the full UVFR utils functionality to be operational.
 *
 * @attention Calling _uvInitPanic() is irreversable and will cause the vehicle to hang itself. This is only to be used as a last resort to stop
 * the vehicle from entering an invalid state.
 *
 */
void _uvInitPanic(){ //Shit is so fucked up the thing couldn't properly initialize itself even.
	//Ensure vehicle defaults to safe state

	//Kill all tasks

	//Kill the scheduler

	vTaskEndScheduler(); //No more computer lol
	//Stop it, get some help


	while(1){//We want the program to straight up hang itself

	}
}



