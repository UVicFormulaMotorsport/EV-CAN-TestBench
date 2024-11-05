/*
 * uvfr_state_engine.c
 *
 *  Created on: Oct 15, 2024
 *      Author: byo10
 */

#define UVFR_STATE_MACHINE_IMPLIMENTATION

#include "uvfr_utils.h"

#define MAX_NUM_MANAGED_TASKS 16

uv_task_id _next_task_id = 0;
uv_task_info _task_register[MAX_NUM_MANAGED_TASKS] = {0};

QueueHandle_t state_change_queue = NULL;

enum uv_vehicle_state_t vehicle_state = UV_BOOT;
enum uv_vehicle_state_t previous_state = UV_BOOT;


/** @brief This function gets called when you want to create a task, and register it
 * with the task register. Theres some gnarlyness here, but not unacceptable levels.
 * Pray this thing doesn't hang itself.
 *
 */
uv_task_info *uvCreateTask(){

	/** Do not exceed the number of tasks available
	 *
	 */

	if(_next_task_id >= MAX_NUM_MANAGED_TASKS){
		return NULL;
	}

	/** Acquire the pointer to the spot in the array, we are doing this since we need to
	 * return the pointer anyways
	 *
	 */

	uv_task_info* _newtask = &(_task_register[_next_task_id]);
	_newtask->task_id = _next_task_id;
	++_next_task_id;

	_newtask->task_name = NULL;

	_newtask->task_function = NULL;
	_newtask->task_priority = osPriorityNormal;

	_newtask->instances = _UV_DEFAULT_TASK_INSTANCES;
	_newtask->stack_size = _UV_DEFAULT_TASK_STACK_SIZE;

	_newtask->task_state = UV_TASK_NOT_STARTED;
	_newtask->active_states = 0x00;
	_newtask->deletion_states = 0x00;
	_newtask->suspension_states = 0x00;

	_newtask->next = NULL;


	_newtask->task_handle = NULL;




	return _newtask;
}

uv_status addTaskToTaskRegister(){
	return 0;
}

uv_status _uvValidateSpecificTask(uv_task_id id){
	uv_task_info* current_task = &(_task_register[id]);
	if(current_task->active_states & current_task->deletion_states & current_task->suspension_states){
				//Undefined behavior time, this should be 0
	}

	if(current_task->task_function == NULL){
				//Invalid, since no task assigned
	}

	if(current_task->task_name == NULL){
				//Invalid, task needs a name
	}

	if(current_task->instances == 0){
				//FreeRTOS literally will not be able to run this if that is the case
	}

	return UV_OK;
}

/** @brief ensure that all the tasks people have created actually make sense, and are valid
 *
 */
uv_status uvValidateManagedTasks(){
	uv_status is_chill;
	for(int i = 0; i < _next_task_id; i++){

		is_chill = _uvValidateSpecificTask(i);

		if(is_chill != UV_OK){
			return is_chill;
		}

	}

	return UV_OK;
}




/** @brief: This is a function that starts tasks which are already registered in the system
 *
 *	This bad boi gets called from the stateChangeDaemon because it's a special little snowflake.
 */
enum uv_status_t uvStartTask(uint32_t* tracker,uv_task_info* t){
	/** The first thing we will do is check
	 *
	 */
	if(t->task_state == UV_TASK_RUNNING){
		if(t->task_handle == NULL){
			return UV_ERROR; //LITERALLY HOW HAS THIS HAPPENED
		}else{
			*tracker &= ~_BV_32(t->task_id);
			return UV_ABORTED;

		}
	}


	/** If a task has been suspended, we do not want to create a new instance
	 * of the task, becuase then the task will go out of scope, and changing the task
	 * handle to a new instance will result in the task never being de-initialized, therefore causing a
	 * memory leak. If a task is in this boat then that's what will happen.
	 *
	 */
	if(t->task_state == UV_TASK_SUSPENDED){
		if(t->task_handle == NULL){ //this should not be physically possible
			return UV_ERROR;
		}

		vTaskResume(t->task_handle);

		t->task_state = UV_TASK_RUNNING;
		*tracker &= ~_BV_32(t->task_id); //Set the bit in the tracker so that we won't have issues down the road
		return UV_OK;
	}

	/** If none of the previous if statements caught the task handle, then that means that either this is
	 * our first time attempting to activate this task, or the task has been deleted at some point prior to this one
	 *
	 */

	if(t->task_function == NULL){
		return UV_ERROR;
	}

	//const osThreadDef_t new_thread = {t->task_name,t->task_function,t->task_priority,t->instances,t->stack_size};
	//t->task_handle = osThreadCreate(&new_thread,(void*)t);
	BaseType_t x_return = xTaskCreate(t->task_function,t->task_name,t->stack_size,t,t->task_priority,&(t->task_handle));

	if(x_return != pdPASS){ //thats not very good, or very cash money of you

	}
	/** The function @c osThreadCreate returns null if it fails to create a thread.
	 * If that happens, we really do have a problem, so we will be returning an error value
	 *
	 */
	if(t->task_handle == NULL){ //WTF, how has this occurred
		return UV_ERROR;
	}

	//we may need to explicitely start a task

	return UV_OK;
}


/** @brief Function that prepares the state engine to do its thing
 *
 * Who even fucking knows at this point
 *
 */
enum uv_status_t uvInitStateEngine(){
	//create all the managed tasks :)

	initDrivingLoop(NULL); //create the main driving loop task
	initTempMonitor(NULL); //create the temperature monitoring task
	initDaqTask(NULL);
	return UV_OK;
}




/** Actually starts up the state engine to do state engine things
 *
 */
enum uv_status_t uvStartStateMachine(){
	return UV_OK;
}

/** @brief The name should be pretty self explanatory
 *
 */
enum uv_status_t killEmAll(){
	uint32_t tracker = 0x00000000;
	QueueHandle_t incoming_scd_msg = xQueueCreate(8,sizeof(uv_init_task_response));
	for(int i = 0; i<_next_task_id;i++){
		tracker |= _BV_32(i);
		uvDeleteTask(&tracker,&_task_register[i]);
	}
	if(tracker){
		return UV_ERROR; //When the function of last resort fails
	}

	return UV_OK;
}


/**@brief deletes a managed task via the system
 *
 * This function is the lowtier god of the program. It pulls up and is like "YOU SHOULD KILL YOURSELF, NOW!!"
 * It sends a message to the task which tells it to kill itself.
 *
 */
enum uv_status_t uvDeleteTask(uint32_t* tracker,uv_task_info* t){
	if(t->task_state == UV_TASK_DELETED || t->task_state == UV_TASK_NOT_STARTED){
		if(t->task_handle == NULL){
			return UV_ABORTED;
		}else{
			return UV_ERROR;
		}

	}

	//TaskStatus_t task_state = eTaskGetState( TaskHandle_t xTask );
	//vTaskGetInfo(t->task_handle,&task_status,pdFalse,eInvalid);

	if(t->task_state == UV_TASK_SUSPENDED){
		vTaskDelete(t->task_handle);
		t->task_handle = NULL;
		t->task_state = UV_TASK_DELETED;
		return UV_OK;

	}


	eTaskState task_state = eTaskGetState(t->task_handle);

	if((task_state == eSuspended)&&(task_state != eBlocked)){
		vTaskDelete(t->task_handle);
		t->task_handle = NULL;
		t->task_state = UV_TASK_DELETED;
		*tracker &= ~_BV_32(t->task_id);
		return UV_OK;

	}else if(task_state == eDeleted){ //Should this be ok? I dont really know
		t->task_handle = NULL;
		t->task_state = UV_TASK_DELETED;
		*tracker &= ~_BV_32(t->task_id);
		return UV_OK;
	}

	/** if we have reached this point in the function, then the task is active and ready to get nuked
	 *
	 */

	t->cmd_data = UV_KILL_CMD;

	return UV_OK;
}
/** @brief function to suspend one of the managed tasks
 *
 */
enum uv_status_t uvSuspendTask(uint32_t* tracker,uv_task_info* t){
	eTaskState rtos_task_state = eTaskGetState(t->task_handle);


	if(t->task_state == UV_TASK_SUSPENDED){
		if(rtos_task_state != eSuspended){ //SCD thinks this is suspended
			uvPanic(NULL,0);
		}
	}

	return UV_OK;
}

enum uv_status_t uvDeInitStateEngine(){
	killEmAll();


	return UV_OK;
}



typedef struct state_change_daemon_args{
	TaskHandle_t meta_task_handle;
}state_change_daemon_args;

//state_change_daemon_args* scd_args = NULL;


/** @brief Function for changing the state of the vehicle, as well as the list of active + inactive tasks.
 *
 * This function also changes out the tasks that are executing.
 *
 */
enum uv_status_t changeVehicleState(uint16_t state){
	/** If the state we wish to change to is the same as the state we're in, then
	 * no need to be executing any of this fancy code
	 *
	 *
	 */
	if(state == vehicle_state){
		return UV_ABORTED;
	}

	previous_state = vehicle_state;
	vehicle_state = state;

	int state_diff = previous_state-vehicle_state;

	switch(state_diff){
		case (UV_INIT-UV_READY):
			/** Transition from @c UV_INIT to @c UV_READY states
			 *
			 */
			break;
		case (UV_INIT-UV_ERROR_STATE):
			/** Transition from @c UV_INIT to @c UV_ERROR states
			*
			*/
			break;
		case (UV_INIT-UV_BOOT):
		case (UV_INIT-UV_DRIVING):
		case (UV_INIT-UV_LAUNCH_CONTROL):
			//invalid transitions
			uvPanic("Invalid State Transition",0);
			break;
		default:
			break;
	}

	//scd_args = malloc(sizeof(state_change_daemon_args));
	//TaskHandle_t SCD_handle = NULL;
	//TaskHandle_t SCD_thread = {"BMS_init",_stateChangeDaemon,osPriorityAboveNormal,128,0};
	state_change_daemon_args* scd_args = malloc(sizeof(state_change_daemon_args));
	scd_args->meta_task_handle = NULL;
	BaseType_t retval;

	retval = xTaskCreate(_stateChangeDaemon,"scd",_UV_DEFAULT_TASK_STACK_SIZE,scd_args,osPriorityAboveNormal,&(scd_args->meta_task_handle));

	if(retval != pdPASS || scd_args->meta_task_handle == NULL){
		uvPanic("State Transition Failed",0);
	}
	//scd_args->meta_task_handle = osThreadCreate(&SCD_thread,scd_args);


	return UV_OK;
}


/** @brief Something bad has occurred here now we in trouble
 *
 */
void uvPanic(char* msg, uint8_t msg_len){
	//ruh roh, something has gone a little bit fucky wucky


}


/** @brief This function is called by a task to nuke itself.
 * Is a wrapper function that is used to do all the different things.
 *
 */
void killSelf(uv_task_info* t){
	/** First lets load up the queue and the values in it.
	 * These come from the task we are doing.
	 *
	 */



	QueueHandle_t status_queue = t->manager;
	uv_scd_response response;
	BaseType_t retval;




	t->task_state = UV_TASK_DELETED;
	response.meta_id = t->task_id;
	response.response_val = UV_SUCCESSFUL_DELETION;


	if(status_queue != NULL){
		retval = xQueueSend(status_queue, &response, 10);

		if(retval != pdPASS){
		//ohdear
		}
	}

	t->cmd_data = UV_NO_CMD;

	vTaskDelete(t->task_handle);

}


/** @brief Called by a task that needs to suspend itself, once the task has determined it
 * is safe to do so.
 *
 *
 */
void suspendSelf(uv_task_info* t){
	QueueHandle_t status_queue = t->manager;
	uv_scd_response response;
	BaseType_t retval;



	t->task_state = UV_TASK_DELETED;
	response.meta_id = t->task_id;
	response.response_val = UV_SUCCESSFUL_DELETION;


	if(status_queue != NULL){
		retval = xQueueSend(status_queue, &response, 10);

		if(retval != pdPASS){
			//ohdear
		}
	}

	t->cmd_data = UV_NO_CMD;

	vTaskSuspend(t->task_handle);
}

uv_status proccessSCDMsg(uv_scd_response * msg){
	if(msg == NULL){
		return UV_ERROR;
	}


	uv_task_id id = msg->meta_id;
	enum uv_scd_response_e replystatus = msg->response_val;
	uv_task_info* t = &(_task_register[id]);
	eTaskState rtos_task_state;

	if(replystatus == UV_SUCCESSFUL_DELETION){
		if(t->task_state != UV_TASK_DELETED){
			return UV_ERROR;
		}

		rtos_task_state = eTaskGetState(t->task_handle);

	}else if(replystatus == UV_SUCCESSFUL_SUSPENSION){
		if(t->task_handle == NULL){
			return UV_ERROR; //that sure aint good, imma go complain now.
		}

		rtos_task_state = eTaskGetState(t->task_handle);
		if(rtos_task_state != eSuspended){
			//The task is not actually suspended!!!
		}

	}else if(replystatus == UV_COULDNT_DELETE){

		return UV_ERROR;
	}else if(replystatus == UV_COULDNT_SUSPEND){

		return UV_ERROR;
	}else if(replystatus == UV_UNSAFE_STATE){

		return UV_ERROR;
	}else{
		//unknown reply, wtf man??

		return UV_ERROR;
	}


	return UV_OK;
}


/** @brief: This collects all the data changing from different tasks, and makes sure that everything works properly
 * 	@attention: DO NOT EVER JUST CALL THIS FUNCTION. THIS SHOULD ONLY BE CALLED FROM changeVehicleState
 *
 */
void _stateChangeDaemon(void * args){
	uv_task_info* tmp_task;

	//BaseType_t rtos_status;

	QueueHandle_t incoming_scd_msg = xQueueCreate(8,sizeof(uv_init_task_response));

	uint32_t task_tracker;

	uv_scd_response response;

	/** We get to iterate through all of the managed tasks.
	 * Goes via IDs as well. We load up the array entry as a temp pointer to a task info struct.
	 * As we go through it determines what to do by comparing the @c uv_task_info.active_states as
	 * well as @c uv_task_info.deletion_states and @c uv_task_info.suspension_states
	 * with @c uv_vehicle_state
	 *
	 * This is done with the bitwise & operator, since the definition of the @c uv_vehicle_state_t enum
	 * facilitates this by only using factors of two.
	 *
	 */
	for(int i = 0; i<_next_task_id; i++){
		tmp_task = &(_task_register[i]);
		tmp_task->manager = incoming_scd_msg;
		if(tmp_task->active_states & vehicle_state){
			//should be active
			if(tmp_task->task_state != UV_TASK_RUNNING){
				//start dah task
				uvStartTask(&task_tracker,tmp_task);
			}
		}else if(tmp_task->deletion_states & vehicle_state){
			//this task should not exist right now
			if((tmp_task->task_state != UV_TASK_NOT_STARTED)&&(tmp_task->task_state != UV_TASK_DELETED)){
				//gotta delete that task buddy
				uvDeleteTask(&task_tracker,tmp_task);
			}
		}else if(tmp_task->suspension_states & vehicle_state){
			/**Now we suspend the task because it has been misbehaving in school
			 *
			 */
			if(tmp_task->task_state != UV_TASK_SUSPENDED){
				//Suspend that thang
				uvSuspendTask(&task_tracker,tmp_task);


			}
		}

	}//end of for loop

	/**Wait for all the tasks that had changes made to respond.\code */

	BaseType_t queue_read_status;
	for(int i = 0; i < _LONGEST_SC_TIME/_SC_DAEMON_PERIOD; i++){
		osDelay(_SC_DAEMON_PERIOD);
		for(int j = 0; j<10;j++){
			queue_read_status = xQueueReceive(incoming_scd_msg,&response,0);
			if(queue_read_status == pdPASS){
				if(proccessSCDMsg(&response)==UV_OK){
					task_tracker &= ~_BV_32(response.meta_id);
				}else{
					//Not ok, idek what to do here
				}

			}else{
				break;
			}
		}
		/** @endcode */

		/** If this is 0, then that means that all the tasks have successfully had their states validated.
		 * This means we can just exit and we're chillin.
		 *
		 */
		if(task_tracker == 0){
			goto END_OF_STATE_CHANGE_DAEMON;
		}

	}
	//You timed out didnt you... Naughty naughty...

	uvPanic(NULL,0);

	END_OF_STATE_CHANGE_DAEMON:

	/** We reset the queue that all the tasks have in their arguments, that way we are chilling
	 * That way if they for whatever reason try to call out of context, they just get hit with a NULL
	 *
	 */
	for(int i = 0; i<_next_task_id; i++){
		_task_register[i].manager = NULL;
	}

	TaskHandle_t scd_handle = ((state_change_daemon_args*)args)->meta_task_handle;
	free(args);

	/**This line deletes the queue to free up the memory @code*/

	vQueueDelete(incoming_scd_msg);

	/**@endcode*/

	vTaskDelete(scd_handle);

}
//end of SCD

