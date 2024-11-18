/*
 * uvfr_state_engine.c
 *
 *  Created on: Oct 15, 2024
 *      Author: byo10
 */

#define UVFR_STATE_MACHINE_IMPLIMENTATION

#include "uvfr_utils.h"
//#include "assert.h"

#define MAX_NUM_MANAGED_TASKS 16
#define MAX_NUM_SVC_TASKS 8


//Stores the actual task info
static uv_task_id _next_task_id = 0;
static uv_task_info* _task_register = NULL;

static uv_task_id _next_svc_task_id = 0;
static uv_task_info* _svc_task_register = NULL;


static volatile bool SCD_active = false;
static QueueHandle_t state_change_queue = NULL;

enum uv_vehicle_state_t vehicle_state = UV_BOOT;
enum uv_vehicle_state_t previous_state = UV_BOOT;

//Function prototypes
uv_status killEmAll();


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
	 * return the pointer anyways, and it cleans up the syntax a little.
	 *
	 */

	uv_task_info* _newtask = &(_task_register[_next_task_id]);
	_newtask->task_id = _next_task_id;
	++_next_task_id;

	_newtask->task_name = NULL;

	_newtask->task_function = NULL;
	_newtask->task_priority = osPriorityNormal;

	_newtask->max_instances = _UV_DEFAULT_TASK_INSTANCES;
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
		return UV_ERROR;
	}

	if(current_task->task_function == NULL){
				//Invalid, since no task assigned
		return UV_ERROR;
	}

	if(current_task->task_name == NULL){
				//Invalid, task needs a name
		return UV_ERROR;
	}

	if(current_task->max_instances == 0){
		//FreeRTOS literally will not be able to run this if that is the case
		return UV_ERROR;
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

	return is_chill;
}




/** @brief: This is a function that starts tasks which are already registered in the system
 *
 *	This bad boi gets called from the stateChangeDaemon because it's a special little snowflake.
 */
uv_status uvStartTask(uint32_t* tracker,uv_task_info* t){
	/** The first thing we will do is check if the task is running, since this could theoretically get called from literally anywhere.
	 * If the task is running, then we check to see if @c t->task_handle is set to @c NULL . If it is null, that is a physically impossible_state.
	 * Neither very mindful or very demure.
	 *
	 * That being said, if the task appears legit, then just update the corresponding bits in the tracker, and return that the task has aborted.
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
	 * memory leak. We want to call @c vTaskResume instead, and just boot the task back into existence.
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

	//we may need to explicitely start a task. Ha, JK
	t->task_state = UV_TASK_RUNNING;
	return UV_OK;
}


/** @brief Function that prepares the state engine to do its thing
 *
 * Who even fucking knows at this point
 *
 */
uv_status uvInitStateEngine(){
	//create all the managed tasks :)
	_task_register = uvMalloc(sizeof(uv_task_info)*MAX_NUM_MANAGED_TASKS);

	if(_task_register == NULL){
		__uvInitPanic();
	}

	initDrivingLoop(NULL); //create the main driving loop task
	initTempMonitor(NULL); //create the temperature monitoring task
	initDaqTask(NULL);
	initOdometer(NULL);

	return UV_OK;
}

/** @brief Stops and frees all resources used by uvfr_state_engine
 *
 * If we need to initialize the state engine, gotta de-initialize as well
 *
 */
uv_status uvDeInitStateEngine(){
	return killEmAll();
}




/** Actually starts up the state engine to do state engine things
 *
 */
uv_status uvStartStateMachine(){
	if(uvValidateManagedTasks() != UV_OK){
		return UV_ERROR;
	}

	previous_state = vehicle_state;
	vehicle_state = UV_INIT;

	return UV_OK;
}

/** @brief The name should be pretty self explanatory
 *
 */
uv_status killEmAll(){
	uint32_t tracker = 0x00000000;
	//QueueHandle_t incoming_scd_msg = xQueueCreate(8,sizeof(uv_init_task_response));
	for(int i = 0; i<_next_task_id;i++){
		tracker |= _BV_32(i);
		uvDeleteTask(&tracker,&_task_register[i]);
	}
	if(tracker){
		return UV_ERROR; //When the function of last resort fails
	}

	return UV_OK;
}


/** @brief if a task refuses to comply with the SCD, then it has no choice but to be deleted. There
 * is nothing that can be done.
 *
 * You will not win against the operating system.
 *
 */
static uv_status uvKillTaskViolently(){
	/** The first thing that needs to happen, is we will tell the kernel to
	 * release any resources owned by the task.
	 *
	 */
	return UV_OK;
}


/**@brief deletes a managed task via the system
 *
 * This function is the lowtier god of the program. It pulls up and is like "YOU SHOULD KILL YOURSELF, NOW!!"
 * It sends a message to the task which tells it to kill itself.
 *
 */
uv_status uvDeleteTask(uint32_t* tracker,uv_task_info* t){
	if(t == NULL){
		uvPanic("Null Task Info Struct",0);
	}

	if(t->task_state == UV_TASK_DELETED || t->task_state == UV_TASK_NOT_STARTED){
		if(t->task_handle == NULL){
			*tracker &= ~(0x01<<(t->task_id));
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
		*tracker &= ~(0x01<<(t->task_id));
		return UV_OK;

	}

	/** This checks with the RTOS kernel to see that the task
	 * as stated by the scheduler matches the state known by uvfr_utils
	 *
	 */
	eTaskState task_state = eTaskGetState(t->task_handle);

	if((task_state == eSuspended)&&(task_state != eBlocked)){
		vTaskDelete(t->task_handle);
		t->task_handle = NULL;
		t->task_state = UV_TASK_DELETED;
		*tracker &= ~(0x01<<(t->task_id));
		return UV_OK;

	}else if(task_state == eDeleted){ //Should this be ok? I dont really know
		t->task_handle = NULL;
		t->task_state = UV_TASK_DELETED;
		*tracker &= ~(0x01<<(t->task_id));
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
uv_status uvSuspendTask(uint32_t* tracker,uv_task_info* t){

	if(t == NULL){
		uvPanic("Null Task Info Struct",0);
	}

	eTaskState rtos_task_state = eInvalid;

	if(t->task_handle != NULL){
		rtos_task_state = eTaskGetState(t->task_handle);
	}else{
		if((t->task_state == UV_TASK_DELETED)||(t->task_state == UV_TASK_NOT_STARTED)){
			*tracker = *tracker & ~(0x01<<(t->task_id));
			return UV_OK;
		}else{
			return UV_ERROR; //WHY IS IT NULL
		}
	}

	if(t->task_state == UV_TASK_SUSPENDED){
		if(rtos_task_state != eSuspended){ //SCD thinks this is suspended
			uvPanic("TState Not Match RTOS in UVsuspend",0);
			return UV_ERROR;
		}else{
			*tracker &= ~(0x01<<(t->task_id));
			return UV_OK;
		}
	}

	if(t->task_state == UV_TASK_DELETED){
		if(rtos_task_state != eDeleted){
			uvPanic("State Engine Claim deleted but RTOS exist",0);
			return UV_ERROR;
		}
		*tracker &= ~(0x01<<t->task_id);
	}

	t->cmd_data = UV_SUSPEND_CMD;

	return UV_OK;
}




typedef struct state_change_daemon_args{
	TaskHandle_t meta_task_handle;
}state_change_daemon_args;

//state_change_daemon_args* scd_args = NULL;


/** @brief Function for changing the state of the vehicle, as well as the list of active + inactive tasks.
 *
 * This function also changes out the tasks that are executing, by invoking the legendary
 * state_change_daemon
 *
 */
uv_status changeVehicleState(uint16_t state){
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

	state_change_daemon_args* scd_args = uvMalloc(sizeof(state_change_daemon_args));
	scd_args->meta_task_handle = NULL;
	BaseType_t retval;

	retval = xTaskCreate(_stateChangeDaemon,"scd",256,scd_args,osPriorityAboveNormal,&(scd_args->meta_task_handle));

	if(retval != pdPASS || scd_args->meta_task_handle == NULL){
		uvFree(scd_args);
		uvPanic("State Transition Failed",0);
	}
	//scd_args->meta_task_handle = osThreadCreate(&SCD_thread,scd_args);


	return UV_OK;
}


/** @brief Something bad has occurred here now we in trouble
 *
 */
void __uvPanic(char* msg, uint8_t msg_len, const char* file, const int line, const char* func){
	//ruh roh, something has gone a little bit fucky wucky
	vTaskSuspendAll();
	while(1){

	}

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

	if(t == NULL){
		uvPanic("Null Task Info Struct",0);
	}



	//QueueHandle_t status_queue = t->manager;
	uv_scd_response* response = uvMalloc(sizeof(uv_scd_response));



	if(response == NULL){
		uvPanic("uvMalloc Failed",0);
	}




	t->task_state = UV_TASK_DELETED;
	response->meta_id = t->task_id;
	response->response_val = UV_SUCCESSFUL_DELETION;


	if(state_change_queue != NULL){
		if(xQueueSend(state_change_queue, &response, 0) != pdPASS){
			uvFree(response); //no memory leaks here sir :)
			uvPanic("Failed to send scd queue msg",0);
		}
	}else{ //bro why tf is the queue null
		uvPanic("Task Has invalid queue while SCD active",0);
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

	if(t == NULL){
		uvPanic("Null Task Info Struct",0);
	}
	//QueueHandle_t status_queue = t->manager;
	uv_scd_response* response = uvMalloc(sizeof(uv_scd_response));


	if(response == NULL){
		uvPanic("uvMalloc Failed",0);
	}

	t->task_state = UV_TASK_SUSPENDED;
	response->meta_id = t->task_id;
	response->response_val = UV_SUCCESSFUL_SUSPENSION;


	if(state_change_queue != NULL){
		if(xQueueSend(state_change_queue, &response, 0) != pdPASS){
			uvFree(response);
			uvPanic("Task couldn't send msg to scd queue",0);
		}

	}else{
		uvPanic("Task Has invalid queue while SCD active",0);
	}

	t->cmd_data = UV_NO_CMD;

	vTaskSuspend(t->task_handle);
}


/** @brief Helper function for the SCD, that proccesses a message, and double checks to make sure the
 * task that sent the message isn't straight up lying to us
 *
 * This function is responsible for the following functionality:
 * - Make sure that the message claims that the deletion or suspension of a task is successful
 * - If a task claims that it is deleted, or suspended, then we must verify that this is the case
 *
 *
 */
static uv_status proccessSCDMsg(uv_scd_response* msg){
	if(msg == NULL){
		return UV_ERROR;
	}

	/** Get the id of the message, then use that to index the _task_register
	 *
	 */
	uv_task_id id = msg->meta_id;
	enum uv_scd_response_e replystatus = msg->response_val;
	uv_task_info* t = &(_task_register[id]);
	eTaskState rtos_task_state =eInvalid;

	if(t->task_handle != NULL){
		rtos_task_state = eTaskGetState(t->task_handle);
	}

	if(replystatus == UV_SUCCESSFUL_DELETION){
		if(t->task_state != UV_TASK_DELETED){
			return UV_ERROR;
		}

		//rtos_task_state = eTaskGetState(t->task_handle); //HOL UP: MAYBE CHECK TO MAKE SURE THE TASK EXISTS LOL, otherwise null pointer dereference time

		if(rtos_task_state != eDeleted){

		}

	}else if(replystatus == UV_SUCCESSFUL_SUSPENSION){
		if(t->task_handle == NULL){
			return UV_ERROR; //that sure aint good, imma go complain now.
		}

		//rtos_task_state = eTaskGetState(t->task_handle);
		if((rtos_task_state != eSuspended)&&(rtos_task_state != eBlocked)){
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


/** @brief This collects all the data changing from different tasks, and makes sure that everything works properly
 *
 * 	@attention DO NOT EVER JUST CALL THIS FUNCTION. THIS SHOULD ONLY BE CALLED FROM changeVehicleState
 *
 * @param This accepts a @c void* pointer to avoid compile errors with freeRTOS, since freeRTOS expects
 * a pointer to the function that accepts a void pointer
 *
 * This is a one-shot RTOS task that spawns in when we want to change the state of the vehicle state.
 * It performs this in the following way
 */
void _stateChangeDaemon(void * args){
	SCD_active = true;


	uint32_t task_tracker = 0x00000000;
	uv_task_info* tmp_task;

	//BaseType_t rtos_status;

	state_change_queue = xQueueCreate(8,sizeof(uv_scd_response*));


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
		/**Acquires pointer to task definition struct, then sets the queue in the struct to the SCD queue,
		 * so that the task actually does task things. Love when that happens. Next it sets
		 * the bit in the task_tracker corresponding to the task id, therefore marking that some action must be taken to either
		 * - confirm that no action is neccessary
		 * - bring the task state into the correct state
		 *  @code */
		tmp_task = &(_task_register[i]);
		//tmp_task->manager = incoming_scd_msg; //No longer using this var, instead using the state_change_queue static global var
		if(((tmp_task->task_flags)&(UV_TASK_MANAGER_MASK | UV_TASK_SCD_IGNORE)) == UV_TASK_VEHICLE_APPLICATION){
			task_tracker |= 0x01<<i;
		}else{
			//If we get here that means that either:
			//This task has the SCD ignore flag active
			//This is a SVC task
			continue; //This is not the task you want
		}
		//redundant_tracker |= 0x01<<i;
		/**@endcode*/
		if(tmp_task->active_states & vehicle_state){
			//should be active
			if(tmp_task->task_state != UV_TASK_RUNNING){
				//start dah task
				if(uvStartTask(&task_tracker,tmp_task) == UV_OK){
					task_tracker &= ~(0x01<<i);
				}else{
					uvPanic("Failed to start task",0);
				}
			}else{
				task_tracker &= ~(0x01<<i); //didnt need to do anything
			}
		}else if(tmp_task->deletion_states & vehicle_state){
			//this task should not exist right now
			if((tmp_task->task_state != UV_TASK_NOT_STARTED)&&(tmp_task->task_state != UV_TASK_DELETED)){
				//gotta delete that task buddy
				uvDeleteTask(&task_tracker,tmp_task);
				//task_tracker &= ~(0x01<<i);
			}else{
				task_tracker &= ~(0x01<<i);
			}
		}else if(tmp_task->suspension_states & vehicle_state){
			/** Now we suspend the task because it has been misbehaving in school
			 *
			 */
			if(tmp_task->task_state != UV_TASK_SUSPENDED){
				//Suspend that thang
				uvSuspendTask(&task_tracker,tmp_task);
				//task_tracker &= ~(0x01<<i);

			}else{
				task_tracker &= ~(0x01<<i);
			}
		}

	}//end of for loop

	/**Wait for all the tasks that had changes made to respond.\code */

	uv_scd_response* response = NULL;

	for(int i = 0; i < _LONGEST_SC_TIME/_SC_DAEMON_PERIOD; i++){
		vTaskDelay(_SC_DAEMON_PERIOD);
		for(int j = 0; j<10;j++){

			/** Attempt to read from the message queue. If successful, then
			 * @c xQueueReceive() will return @c pdPASS and we will then be able to
			 * interpret the message.
			 *
			 * @c xQueueReceive() copies a pointer to the response function to the address specified by &response
			 *
			 */
			if(xQueueReceive(state_change_queue,&response,1) == pdPASS){

				if(response == NULL){//definately not supposed to happen
					uvPanic("null scd response",0);
				}

				/** Message interpretation happens in the @function proccessSCDMsg(), which is passed a pointer to
				 * a response message struct. See the documentation for this function to learn about it's internals, and what
				 * it actually looks like.
				 *
				 */
				if(proccessSCDMsg(response)==UV_OK){
//					if(task_tracker & 0xFFFFFFF0){
//									//gotcha
//						uvPanic("gotcha bitch proccessSCDMsg!!",0);
//					}
					//has_task_succeeded |= (0x01<<response->meta_id);
					task_tracker &= ~(0x01<<response->meta_id);
					if (_task_register[response->meta_id].task_state == UV_TASK_DELETED){
						_task_register[response->meta_id].task_handle = NULL;
					}


				}else{
					//Not ok, this means that process SCD has returned something weird. More detailed error_handling can be added later.
					uvPanic("Task giving Sass to SCD",0);
				}

				/** Gotta free the response
				 *
				 */
				if(uvFree(response)!=UV_OK){
					uvPanic("failed to free memory", 0);
				}
				response = NULL;

			}else{
				break;
			}


		}
		/** @endcode */
		/** If this is 0, then that means that all the tasks have successfully had their states validated.
		 *
		 * This means we can just exit and we're chillin.
		 *
		*/
		if(task_tracker == 0){
			goto END_OF_STATE_CHANGE_DAEMON;
		}


	}
	//You timed out didnt you... Naughty naughty...

	uvPanic("SCD Timeout",0);

	END_OF_STATE_CHANGE_DAEMON:




	TaskHandle_t scd_handle = ((state_change_daemon_args*)args)->meta_task_handle;
	uvFree(args);

	/**This line deletes the queue to free up the memory @code */
	vQueueDelete(state_change_queue);
	state_change_queue = NULL;
	/**@endcode*/


	/**The final act of the SCD, is to delete itself @code */
	vTaskDelete(scd_handle);
	/** @endcode */

}
//end of SCD


/** @brief The big papa task that deals with handling all of the others
 *
 */
void uvTaskManager(void* args){

	for(;;){

	}

}

uv_task_info* uvCreateServiceTask(){
	return NULL;
}

/** @brief Function to start a service task specifically
 *
 */
uv_status uvStartSVCTask(){

	return UV_OK;

}

/** @brief Function that suspends a service task
 *
 */
uv_status uvSuspendSVCTask(){
	return UV_OK;
}

/** @brief For when you need to delete a service task... for some reason...
 *
 */
uv_status uvDeleteSVCTask(){

	return UV_OK;
}

/** @brief Function that takes a service part that may be messed up and tries to reboot it to recover
 *
 * This may be neccessary if a SVC task is not responding. Be careful though, since this has the potential to delay more important tasks :o
 * Therefore, this technique should be used sparingly, and each task gets a limited number of attempts within a certain time period.
 */
uv_status uvRestartSVCTask(){
	//force task to relinquish resources

	//ask nicely for task to be deleted

	//if that fails, forcibly delete task
	uvKillTaskViolently();
	//try to start the task again
	uvStartSVCTask();
	return UV_OK;

}

/** @brief oversees all of the service tasks, and makes sure that theyre alright
 *
 *
 */
void uvSVCTaskManager(void* args){



	/** Start all of the service tasks. This involves allocating neccessary memory,
	 * setting the appropriate task parameters, and saying "fuck it we ball" and adding the tasks to the
	 * central task tracking data structure.
	 *
	 *
	 */

	//TODO: Determine whether or not service tasks should actually use the same struct as vehicle application tasks, since they behave completely differently
	_svc_task_register = uvMalloc(sizeof(uv_task_info)); //allocate mem for the svc task register


	if(_svc_task_register == NULL){
		__uvInitPanic(); //Double Plus Ungood
	}


	//super basic for now, just need something working



	//iterate through the list

	for(;;){
		//Hold for a command

		//Do different things depending on what the value of the command is

		//update various parameters that need updated
	}

	/** Now we deinitialize the svcTaskManager.
	 * This is done by doing the following:
	 * - actually shut down the svc tasks
	 * - double check that the tasks have acually shut down
	 * - if any svc tasks are resisting nature's call, they will be shut down forcibly
	 * - deallocate data structures specific to @c uvSVCTaskManager
	 *
	 * Lovely times for all
	 *
	 */

	uvFree(_svc_task_register); //Free this to avoid mem leaks
	//vTaskDelete();
}

/** Sometimes you just gottta deal with it lol
 *
 */
uv_task_id getSVCTaskID(char* tsk_name){
	return 0xFF;
}

