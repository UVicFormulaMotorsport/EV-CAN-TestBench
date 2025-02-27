/*
 * uvfr_utils.c
 *
 *  Created on: Oct 7, 2024
 *      Author: byo10
 */


#define UV_UTILS_SRC_IMPLIMENTATION
#include "uvfr_utils.h"

extern TaskHandle_t init_task_handle;
extern uint8_t               TxData[8];

TaskHandle_t reset_handle = NULL;


//#define CAN_TRANSMIT_TEST_IN_INIT



/** @brief: Function that initializes all of the car's stuff.
 *
 * This is an RTOS task, and it serves to setup all of the car's different functions.
 * at this point in our execution, we have already initialized all of our favorite hardware
 * peripherals using HAL. Now we get to configure our convoluted system of OS-level
 * settings and state machines.
 *
 * It executes the following functions, in order:
 * - Load Vehicle Settings
 * - Initialize and Start State Machine
 * - Start Service Tasks, such as CAN, ADC, etc...
 * - Initialize External Devices such as BMS, IMD, Motor Controller
 * - Validate that these devices have actually booted up
 * - Set vehicle state to @c UV_READY
 *
 *	Pretty important shit if you ask me.
 */
void uvInit(void * arguments){
	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_15); //For debugging purposes, I wanna see if we actually end up here at some point

	char* error_msg = NULL;
	uint8_t msg_length = 0;

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

	if(uvSettingsInit() != UV_OK){
		__uvInitPanic();

		/** Next up we will attempt to initialize the state engine. If this fails, then we are in another case where we are genuinely unsafe to drive.
		 * This will create the prototypes for a bajillion tasks that will be started and stopped. Which tasks are currently running,
		 * depends on the whims of the state engine. Since the state engine is critical to our ability to handle errors and implausibilitys,
		 * we cannot proceed without a fully operational state engine.
		 */
	}else if(uvInitStateEngine() != UV_OK){
		__uvInitPanic();

		/** Once the state machine is initialized we get to actually start the thing.
		 *
		 */
	}else if(uvStartStateMachine() != UV_OK){
		__uvInitPanic();
	}

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
	QueueHandle_t init_validation_queue = xQueueCreate(8,sizeof(uv_init_task_response));
	if(init_validation_queue == NULL){
		__uvInitPanic();
	}

	/** @endcode
	 * The next big thing on our plate is checking the status of all external devices we need, and initializing them with appropriate parameters.
	 * These are split into tasks because it takes a bit of time, especially for devices that need to be configured via CANBus such as the motor controller.
	 * That is why it is split the way it is, to allow these to run somewhat concurrently
	 * @code */

	BaseType_t retval;
	//osThreadDef_t MC_init_thread = {"MC_init",MC_Startup,osPriorityNormal,128,0};
	uv_init_task_args* MC_init_args = uvMalloc(sizeof(uv_init_task_args));
	MC_init_args->init_info_queue = init_validation_queue;
	MC_init_args->specific_args = &(current_vehicle_settings->mc_settings);
	//MC_init_args->meta_task_handle = osThreadCreate(&MC_init_thread,MC_init_args);
	//vTaskResume( MC_init_args->meta_task_handle );
	retval = xTaskCreate(MC_Startup,"MC_init",128,MC_init_args,osPriorityAboveNormal,&(MC_init_args->meta_task_handle));
	if(retval != pdPASS){
		//FUCK
		error_msg = "bruh";
	}
	/** @endcode
	 * This thread is for initializing the BMS
	 * @code */


	//osThreadDef_t BMS_init_thread = {"BMS_init",BMS_Init,osPriorityNormal,128,0};
	uv_init_task_args* BMS_init_args = uvMalloc(sizeof(uv_init_task_args));
	BMS_init_args->init_info_queue = init_validation_queue;
	BMS_init_args->specific_args = &(current_vehicle_settings->bms_settings);
	//BMS_init_args->meta_task_handle = osThreadCreate(&BMS_init_thread,BMS_init_args);
	retval = xTaskCreate(BMS_Init,"BMS_init",128,BMS_init_args,osPriorityAboveNormal,&(BMS_init_args->meta_task_handle));
	if(retval != pdPASS){
		//FUCK
		error_msg = "bruh";
	}
	/** @endcode
	* This variable is a tracker that tracks which devices have successfully initialized
	* @code*/

	uv_init_task_args* IMD_init_args = uvMalloc(sizeof(uv_init_task_args));
	IMD_init_args->init_info_queue = init_validation_queue;
	IMD_init_args->specific_args = &(current_vehicle_settings->imd_settings);
	retval = xTaskCreate(initIMD,"BMS_init",128,IMD_init_args,osPriorityAboveNormal,&(IMD_init_args->meta_task_handle));
	if(retval != pdPASS){
			//FUCK
		error_msg = "bruh";
	}

	uv_init_task_args* PDU_init_args = uvMalloc(sizeof(uv_init_task_args));
	PDU_init_args->init_info_queue = init_validation_queue;
	PDU_init_args->specific_args = &(current_vehicle_settings->imd_settings);
	retval = xTaskCreate(initPDU,"PDU_init",128,PDU_init_args,osPriorityAboveNormal,&(PDU_init_args->meta_task_handle)); //pass in the right settings, dumdum
	if(retval != pdPASS){
			//FUCK
		error_msg = "bruh";
	}


	uint16_t ext_devices_status = 0x000F; //Tracks which devices are currently setup




	/** @endcode
	 *
	 * Wait for all the spawned in tasks to do their thing. This should not take that long, but we wanna be sure that everything is chill
	 * If we are say, missing a BMS, then it will not allow you to proceed past the initialisation step
	 * This is handled by a message buffer, that takes inputs from all of the tasks
	 *
	 */

	if(error_msg != NULL){
		__uvInitPanic();
	}

	if(BMS_init_args->meta_task_handle == NULL || MC_init_args->meta_task_handle == NULL){
		__uvInitPanic();
	}
	/** We allocate space for a response from the initialization.
	 *
	 */
	uv_init_task_response rx_response;
	TickType_t last_time = xTaskGetTickCount();
	for(int i = 0; i< MAX_INIT_TIME/INIT_CHECK_PERIOD; i++){
		vTaskDelayUntil(&last_time,pdMS_TO_TICKS(INIT_CHECK_PERIOD));

		while(xQueueReceive(init_validation_queue,&rx_response,0) == pdPASS){
			if(rx_response.status == UV_OK){
				ext_devices_status &= ~(0x01<<rx_response.device);
			}else{
				error_msg = rx_response.errmsg;
				msg_length = rx_response.nchar;

			}

		}



		if(ext_devices_status == 0){
			//SUCCESS
			//Set vehicle state to "idle"
			if(changeVehicleState(UV_READY) == UV_OK){
				break;
			}else{
				uvPanic("Unable To Change State to Ready",0);
			}

			//GTFO
			break;
		}else{

		}
	}
	//If we get here, then we have timed out

	initialisation_failure:


	/** Clean up, clean up, everybody clean up, clean up, clean up, everybody do your share!
	 *	The following code cleans up all the threads that were running, and free up used memory
	 *
	 */

	vTaskDelete(MC_init_args->meta_task_handle);
	uvFree(MC_init_args);

	vTaskDelete(BMS_init_args->meta_task_handle);
	uvFree(BMS_init_args);

	vTaskDelete(IMD_init_args->meta_task_handle);
	uvFree(IMD_init_args);

	vTaskDelete(PDU_init_args->meta_task_handle);
	uvFree(PDU_init_args);



	//vQueueDelete(init_validation_queue);
	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_15);
	vTaskDelete(init_task_handle);
	//return;

}

void uvSysResetDaemon(void* args){


}

/**@brief This function is a soft-reboot of the uv_utils_backend and OS abstraction.
 *
 * The idea here is to basically start from a blank slate and boot up everything. So therefore we must:
 * - Halt state machine.
 * - Nuke vehicle operation related tasks.
 * - Nuke the state machine
 * - Nuke old settings
 *
 * reinitialize uv_utils
 *
 *
 */
enum uv_status_t uvUtilsReset(){
	xTaskCreate(uvSysResetDaemon,"reset",128,NULL,5,&reset_handle);
	return UV_OK;
}

/** @deprecated I really dunno why this still exists, but this gets called somewhere so Im leaving it.
 * I think we just pass it NULL.
 *
 */
void setup_extern_devices(void * argument){


}


/** @brief Low Level Panic, that does not require the full UVFR utils functionality to be operational.
 *
 * @attention Calling _uvInitPanic() is irreversable and will cause the vehicle to hang itself. This is only to be used as a last resort to stop
 * the vehicle from entering an invalid state.
 *
 */
void __uvInitPanic(){ //Shit is so fucked up the thing couldn't properly initialize itself even.
	//Ensure vehicle defaults to safe state

	//Kill all tasks

	//Kill the scheduler

	vTaskEndScheduler(); //No more computer lol
	//Stop it, get some help


	while(1){//We want the program to straight up hang itself

	}
}

/** @brief Wrapper function for @c malloc() that makes it thread safe
 *
 * This typically appears in a macro expansion from @c uvMalloc(x)
 *
 */
void * __uvMallocCritSection(size_t memrequest){
	void* ptr = NULL;
	uint8_t oopsie_detected = 0;

	if(memrequest == 0){
		return NULL;
	}

	vTaskSuspendAll();

	ptr = malloc(memrequest);

	if(ptr == NULL){
		oopsie_detected = 1;
	}


	if( xTaskResumeAll() == pdTRUE){


	}else{

	}

	if(oopsie_detected){
		return NULL;
	}

	return ptr;
}

/** @brief Thread-safe wrapper for @c free
 *
 * This is typically called from the macro expansion of @c uvFree(x)
 *
 */
uv_status __uvFreeCritSection(void* ptr){
	if(ptr == NULL){
		return UV_ERROR;//Cant free something that doesnt exist
	}

	if(uvIsPTRValid(ptr)!= UV_OK){
		return UV_ERROR;
	}

	vTaskSuspendAll();

	free(ptr);

	if(xTaskResumeAll() != pdTRUE){

	}
	return UV_OK;
}

/** @brief @c malloc() wrapper that calls pvPortMalloc() rather than malloc()
 *
 * The reason we might wanto to be using pvPortMalloc() rather than regular stdlib malloc()
 * is to consolodate the heap between RTOS and non-RTOS functions.
 *
 */
void* __uvMallocOS(size_t memrequest){
	if(memrequest == 0){
		return NULL;
	}else if(memrequest > UV_MALLOC_LIMIT){
		return NULL;
	}

	//Does the scheduler actually need to be running for this?
	//Should maybe double check

	void* retval = NULL;

	retval = pvPortMalloc(memrequest);

	if(retval == NULL){
		uvPanic("OS Malloc Failed",0);
	}else if(uvIsPTRValid(retval) != UV_OK){
		return NULL;
	}

	return retval;
}

/** @brief OS-based free wrapper that calls pvPortFree
 *
 */
uv_status __uvFreeOS(void* ptr){
	if(ptr == NULL){
		return UV_ERROR;
	}

	if(uvIsPTRValid(ptr) != UV_OK){
		return UV_ERROR;
	}

	vPortFree(ptr);

	return UV_OK;
}

/** @brief function that checks to make sure a pointer points to a place it is allowed to point to
 *
 * The primary motivation for this is to avoid trying to dereference a pointer that doesnt exist, and
 * triggering the @c HardFaultHandler(). That is never a fun time.
 * This allows us to exit gracefully instead of getting stuck in an IRQ handler
 *
 * Exiting gracefully can be pretty neat sometimes.
 */
uv_status uvIsPTRValid(void* ptr){
	if(ptr == NULL){
		return UV_WARNING;
	}
	uint32_t pval = (uint32_t)ptr;

	//bool is_valid = false;

	if(pval < 0x000FFFFF){ //Aliased to FLASH, systmem or SRAM
		return UV_OK;
	}

	if((pval > 0x08000000)&& (pval < 0x080FFFFF)){ //Flash be like
		return UV_OK;
	}

	if((pval > 0x10000000)&&(pval < 0x1000FFFF)){ //CCM Data RAM
		return UV_OK;
	}

	if((pval > 0x1FFF0000)&&(pval < 0x1FFF7A0F)){ //System memory + OTP
		return UV_OK;
	}

	if((pval > 0x1FFFC000)&&(pval < 0x1FFFC007)){ //option bytes (should these be user accessable under any circumstances?)
		return UV_WARNING;
	}

	if((pval > 0x20000000)&&(pval < 0x2001FFFF)){ //SRAM :)
		return UV_OK;
	}

	if((pval > 0x40000000)&&(pval < 0x40007FFF)){ //APB1
		return UV_OK;
	}

	if((pval > 0x40010000)&&(pval < 0x400157FF)){ //APB2
		return UV_OK;
	}

	if((pval > 0x40020000)&&(pval < 0x4007FFFF)){ //AHB1
		return UV_OK;
	}

	if((pval > 0x50000000)&&(pval < 0x50060BFF)){//AHB2
		return UV_OK;
	}

	if((pval > 0x60000000)&&(pval < 0xA0000FFF)){//AHB3
		return UV_OK;
	}

	if((pval > 0xE0000000)&&(pval < 0xE00FFFFF)){//
		return UV_OK;
	}

	return UV_ERROR;
}



