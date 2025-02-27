/**
 * @file uvfr_state_engine.c
 * @author Byron Oser
 *
 * @brief File containing the implementation of the vehicle's state engine and error handling infrastructure
 */



#define UVFR_STATE_MACHINE_IMPLIMENTATION

#include "uvfr_utils.h"
//#include "assert.h"

/**
 * @addtogroup state_engine
 * @{
 */

#define MAX_NUM_MANAGED_TASKS 16



//Stores the actual task info
static uv_task_id _next_task_id = 0;
static uv_task_info* _task_register = NULL;

static TickType_t* last_task_start_times = NULL;
static TickType_t* last_task_end_times = NULL;
uint8_t* task_tardiness;

static uv_task_id _next_svc_task_id = 0;
//static uv_task_info* _svc_task_register = NULL;

TaskHandle_t* scd_handle_ptr;


static volatile bool SCD_active = false;
static volatile bool throttle_daq = false;
static volatile bool nc_throttling = 1;
static QueueHandle_t state_change_queue = NULL;

rbtree* task_name_lut = NULL;

enum uv_vehicle_state_t vehicle_state = UV_BOOT;
enum uv_vehicle_state_t previous_state = UV_BOOT;

uv_task_info* task_manager = NULL;
uv_task_info* svc_task_manager = NULL;

rbtree* task_name_tree;

uv_os_settings* os_settings = NULL;

uv_os_settings default_os_settings ={
		.svc_task_manager_period = 50,
		.task_manager_period = 50,
		.max_svc_task_period = 250,
		.max_task_period = 500,
		.min_task_period = 3,
		.task_overshoot_margin_noncrit = 1.5F,
		.task_overshoot_margin_crit = 1.1F
};

//Function prototypes
uv_status killEmAll();
void uvSVCTaskManager(void* args);
void uvTaskManager(void* args);
int compareTaskByName(uv_task_info* t1, uv_task_info* t2);

typedef struct state_change_daemon_args{
	TaskHandle_t meta_task_handle;
}state_change_daemon_args;

/** @addtogroup state_engine_api
 * @{
 */

/** @brief Function for changing the state of the vehicle, as well as the list of active + inactive tasks.
 *
 * This function also changes out the tasks that are executing, by invoking the legendary
 * _state_change_daemon
 *
 * @param state is a member of \ref uv_status, and therefore a power of two
 * @retval returns a memeber of \ref uv_status depending on whether execution is successful
 *
 * Example usage:
 *
 @code

if((brakepedal_pressed == true) && (start_button_pressed == true)){
	changeVehicleState(UV_DRIVING);
}

 @endcode

 As you can see, all you need to do is specify the new state. Naturally, the task should be ready to get deleted by the state_change_daemon, but that is neither here nor there.
 */
uv_status changeVehicleState(uint16_t state){

	if(!(isPowerOfTwo(state))){
		return UV_ERROR; //literally not a possible state, since all vehicle states are powers of two
	}

	/** If the state we wish to change to is the same as the state we're in, then
	 * no need to be executing any of this fancy code
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


		default:
			//invalid transitions that should not exist
			//uvPanic("Invalid State Transition",0);
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

//	if(*scd_handle_ptr != NULL){
//		xTaskNotifyGive(*scd_handle_ptr); //basically we just wanna wake up the SCD so it rectifies the state
//	}else{
//		return UV_ERROR;
//	}


	return UV_OK;
}


/** @brief Function that prepares the state engine to do its thing
 *
 * This is called when the system is first starting up.
 *
 */
uv_status uvInitStateEngine(){
	//create all the managed tasks :)
	_task_register = uvMalloc(sizeof(uv_task_info)*MAX_NUM_MANAGED_TASKS);

	if(_task_register == NULL){
		__uvInitPanic();
	}

	svc_task_manager = uvCreateServiceTask();
	task_manager = uvCreateServiceTask();

	if((svc_task_manager == NULL)||(task_manager == NULL)){
		__uvInitPanic();
	}

	initDrivingLoop(NULL); //create the main driving loop task
	initTempMonitor(NULL); //create the temperature monitoring task
	initDaqTask(NULL);
	initOdometer(NULL);

	return UV_OK;
}

/** @brief Actually starts up the state engine to do state engine things
 *
 * This function ensures that all of the managed tasks are setup in a legal way, and then it allocates resources for, and starts
 * the state engine and the background tasks. This unlocks the ability for the vehicle to do basically anything.
 */
uv_status uvStartStateMachine(){

	os_settings = current_vehicle_settings->os_settings;


	previous_state = vehicle_state;
	vehicle_state = UV_INIT;



	svc_task_manager->task_name = "svcTaskManager"; //Task info for the svc task manager struct
	svc_task_manager->task_flags |= UV_TASK_MISSION_CRITICAL | UV_TASK_SCD_IGNORE;
	svc_task_manager->task_function = uvSVCTaskManager;
	svc_task_manager->stack_size = 256;
	svc_task_manager->task_period = os_settings->svc_task_manager_period;

	task_manager->task_name = "taskManager"; //Task info for regular uvTaskManager struct
	task_manager->task_flags |= UV_TASK_MISSION_CRITICAL | UV_TASK_SCD_IGNORE;
	task_manager->task_function = uvTaskManager;
	task_manager->stack_size = 256;
	task_manager->task_period = 25;

	if(uvValidateManagedTasks() != UV_OK){
		return UV_ERROR;
	}

	BaseType_t retval;

	//starting up the terrifying tasks
	retval = xTaskCreate(svc_task_manager->task_function,svc_task_manager->task_name,svc_task_manager->stack_size,svc_task_manager,4,&(svc_task_manager->task_handle));

	if(retval != pdPASS){
		return UV_ERROR; //if for whatever god forsaken reason neither of these tasks actually activate
	}

	retval = xTaskCreate(task_manager->task_function,task_manager->task_name,task_manager->stack_size,task_manager,4,&(task_manager->task_handle));

	if(retval != pdPASS){
		return UV_ERROR;//very much ++ ungoods
	}

//	state_change_daemon_args* scd_args = uvMalloc(sizeof(state_change_daemon_args));
//	scd_args->meta_task_handle = NULL;
//	scd_handle_ptr = &(scd_args->meta_task_handle);
//
//
//	retval = xTaskCreate(_stateChangeDaemon,"scd",256,scd_args,5,&(scd_args->meta_task_handle));
//
//	if(retval != pdPASS || scd_args->meta_task_handle == NULL){
//		uvFree(scd_args);
//		uvPanic("SCD Not starting :(",0);
//	}

	return UV_OK;
}

/** @brief Stops and frees all resources used by uvfr_state_engine
 *
 * If we need to initialize the state engine, gotta de-initialize as well. This is the opposite of
 * \ref uvInitStateEngine
 *
 */
uv_status uvDeInitStateEngine(){
	return killEmAll();
}


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

	_newtask->stack_size = _UV_DEFAULT_TASK_STACK_SIZE;

	_newtask->task_state = UV_TASK_NOT_STARTED;
	_newtask->active_states = 0x00;
	_newtask->deletion_states = 0x00;
	_newtask->suspension_states = 0x00;

	_newtask->parent = NULL;

	_newtask->task_handle = NULL;

	_newtask->task_flags |= UV_TASK_VEHICLE_APPLICATION;

	return _newtask;
}

/** @} */ //end of public API

/** @addtogroup state_engine_backend
 * @{
 */

uv_status addTaskToTaskRegister(uv_task_id id, uint8_t assign_to_whom){
	uv_status retval = _uvValidateSpecificTask(id);

	if(retval != UV_OK){
		return retval;
	}

	return 0;
}

/**@brief make sure the parameters of a task_info struct is valid
 *
 */
uv_status _uvValidateSpecificTask(uv_task_id id){
	uv_task_info* current_task = &(_task_register[id]);
	if((current_task->active_states&current_task->deletion_states)|
			(current_task->active_states&current_task->suspension_states)|
			(current_task->deletion_states&current_task->suspension_states)){
				//Undefined behavior time, this should be 0
		return UV_ERROR;
	}

//	if((current_task->active_states | current_task->deletion_states | current_task->suspension_states) != 0x01FF){
//		if((current_task->task_flags & UV_TASK_MANAGER_MASK) == UV_TASK_VEHICLE_APPLICATION)
//		return UV_ERROR; //This avoids undefined states where the task state is not specified for a given vehicle state
//	}

	if(current_task->task_function == NULL){
				//Invalid, since no task assigned
		return UV_ERROR;
	}

	if(current_task->task_name == NULL){
				//Invalid, task needs a name
		return UV_ERROR;
	}

	//Now we check for invalid combinations of flags




	return UV_OK;
}

/** @brief ensure that all the tasks people have created actually make sense, and are valid
 *
 */
uv_status uvValidateManagedTasks(){
	uv_status is_chill;
	for(int i = 0; i < _next_task_id; i++){

		is_chill = _uvValidateSpecificTask(i); //probably a slightly more elegant way to do this than have it immediately hang itself

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
	if(t == NULL){
		return UV_ERROR;
	}


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
		t->last_execution_time = xTaskGetTickCount(); // so we can continue tracking task period

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
		return UV_ERROR;
	}

	if(t->task_handle == NULL){ //WTF, how has this occurred
		return UV_ERROR;
	}

	//we may need to explicitely start a task. Ha, JK
	t->task_state = UV_TASK_RUNNING;
	t->last_execution_time = xTaskGetTickCount();
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
static uv_status uvKillTaskViolently(uv_task_info* t){
	if(t==NULL){
		uvPanic("null task info block",0); //sub-optimal situation here
	}
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
 * The task complies. It does not have a choice.
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

	t->cmd_data = UV_KILL_CMD;
	if(uvTaskIsDelaying(t)){
		xTaskAbortDelay(t->task_handle);
	}

	return UV_OK;
}

/** @brief If a task is scheduled for deletion, we want to be able to resurrect it
 *
 * Calling this will find the task deletion timer, and remove the task from the grave.
 *
 */
uv_status uvAbortTaskDeletion(uv_task_info* t){
	if(t == NULL){
		return UV_ERROR;
	}

	return UV_OK;
}

/** @brief Schedule a task to be deleted in the future
 * double plus ungood imho
 *
 */
uv_status uvScheduleTaskDeletion(uint32_t* tracker, uv_task_info* t){
	if(t == NULL){
		return UV_ERROR;
	}

	if(t->task_state == UV_TASK_DELETED){ //no need to delete an already deleted task.
		*tracker &= ~(0x0001U<<(t->task_id));
		return UV_ABORTED;
	}

	if(t->task_flags & UV_TASK_AWAITING_DELETION){//previos calls of this function get precedence over subsequent ones
		*tracker &= ~(0x0001U<<(t->task_id));
	}

	//Now we get to create some timers to actually delete tasks because that is simply what we need to do.


	return UV_OK;
}

/** @brief function to suspend one of the managed tasks.
 *
 * @param tracker is a pointer to an int. If the task actually suspends, we update the tracker, since no further action is needed.
 *
 * @param t is a pointer to a \ref uv_task_info struct.
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
	if(uvTaskIsDelaying(t)){
		xTaskAbortDelay(t->task_handle); //Speed up the process
	}

	return UV_OK;
}

/** @brief Called when a task has crashed and we need to figure out what to do with it
 *
 * Effectively, there are a couple variables we care about here:
 * 1) Can the vehicle continue operation without that task active?
 * 2) Do we really care?
 *
 * If the task is critical, then this needs to 100% result in a panic.
 * If it isn't then we can try to restart the task, noting that this may result in strange undefined behavior down the line.
 * Thankfully if a task is not safety critical, we don't really care whether it misbehaves. Appropriate countermeasures are in place
 * to prevent one task from overflowing into another task, as well as to mitigate against possible memory leaks.
 *
 */
uv_status uvTaskCrashHandler(uv_task_info* t){
	//this is not a good place to be ngl
	if(t->task_flags & UV_TASK_MISSION_CRITICAL){
		uvPanic("fuck",0);
		return UV_ERROR;
	}else{

	}
	return UV_OK;
}






/** @brief Something bad has occurred here now we in trouble
 *
 * General idea here: Something bad has happened that is severe enough that it requires the shutdown of
 * the vehicle. This can mean several things, such as being on fire, etc... that need to be appropriately handled
 *
 * This should also log whatever the fuck happened.
 *
 * The following should happen, in order:
 * - Forcibly put vehicle into a safe state
 * - Change vehicle state to error, and invoke the SCD
 * - Log the error in our lil running journal
 *
 * Should change vehicle state itself be the source of the error, we just need the software
 * to completely fucking hang itself. If things are so fucked that we genuinely cannot even transition
 * to the error state, then get that shit the fuck outta here, we shuttin down fr fr.
 *
 */
void __uvPanic(char* msg, uint8_t msg_len, const char* file, const int line, const char* func){


	uvSecureVehicle(); // ensure safe state of vehicle.

	changeVehicleState(UV_ERROR_STATE);

	//TODO: We should probably keep a log of this or something

	//ruh roh, something has gone a little bit fucky wucky
	//vTaskSuspendAll();
	//while(1){

	//}

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
	 * Mission critical stuff that stops ev from driving into a wall
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
			//TODO Handle this error, because this is BAD
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

void uvSendTaskStatusReport(uv_task_info* t){

}

//TODO Give SCD Object permanence, make a function that is called by the task manager. Allocation of SCD resources costs valuable time in the event of a fault

/** @brief This collects all the data changing from different tasks, and makes sure that everything works properly
 *
 * 	@attention DO NOT EVER JUST CALL THIS FUNCTION. THIS SHOULD ONLY BE CALLED FROM changeVehicleState
 *
 * @param args This accepts a @c void* pointer to avoid compile errors with freeRTOS, since freeRTOS expects
 * a pointer to the function that accepts a void pointer
 *
 * This is a one-shot RTOS task that spawns in when we want to change the state of the vehicle state.
 * It performs this in the following way
 */
void _stateChangeDaemon(void * args) PRIVILEGED_FUNCTION{
	SCD_active = true;


	uint32_t task_tracker = 0x00000000;
	uv_task_info* tmp_task;

	//BaseType_t rtos_status;

	state_change_queue = xQueueCreate(8,sizeof(uv_scd_response*));

	if(state_change_queue == NULL){
		uvPanic("didntcreatequeue",0);
	}

//	uint32_t notification_value;
//	BaseType_t notification_received;
//

	//for(;;){

//		notification_value = ulTaskNotifyTake(pdTRUE,500);


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
//		if(notification_value == 0){
//			continue;
//		}
		for(int i = 0; i<_next_task_id; i++){
		/**Acquires pointer to task definition struct, then sets the queue in the struct to the SCD queue,
		 * so that the task actually does task things. Love when that happens. Next it sets
		 * the bit in the task_tracker corresponding to the task id, therefore marking that some action must be taken to either
		 * - confirm that no action is neccessary
		 * - bring the task state into the correct state
		 */
			tmp_task = &(_task_register[i]);

			//task_tracker |= 0x01<<i;

		//tmp_task->manager = incoming_scd_msg; //No longer using this var, instead using the state_change_queue static global var

		//redundant_tracker |= 0x01<<i;

			if(tmp_task->active_states & vehicle_state){
			//should be active
				if(tmp_task->task_state != UV_TASK_RUNNING){
				//start dah task
					if(uvStartTask(&task_tracker,tmp_task) == UV_OK){
						task_tracker &= ~(0x01<<i);
					}else{
						uvPanic("Failed to start task",0);
					}
				}else if(tmp_task->task_flags & UV_TASK_AWAITING_DELETION){
				//FUCK, GO BACK!!
				}else{
					task_tracker &= ~(0x01<<i); //didnt need to do anything
				}
			}else if(tmp_task->deletion_states & vehicle_state){
			//this task should not exist right now
				if((tmp_task->task_state != UV_TASK_NOT_STARTED)&&(tmp_task->task_state != UV_TASK_DELETED)){
				//gotta delete that task buddy
					if(tmp_task->task_flags & UV_TASK_DEFER_DELETION){
						uvScheduleTaskDeletion(&task_tracker, tmp_task);
					}else{
						uvDeleteTask(&task_tracker,tmp_task);
					}
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

		}//end of first iteration loop where thas reconciliation occurs

		/**Wait for all the tasks that had changes made to respond.\code */

		uv_scd_response* response = NULL;

		for(int i = 0; i < _LONGEST_SC_TIME/_SC_DAEMON_PERIOD; i++){ //This loop verifies to make sure things are actually chillin
			vTaskDelay(_SC_DAEMON_PERIOD);
			for(int j = 0;j<10;j++){ //What kinda magic number is this? Why 10?

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

		}
		//You timed out didnt you... Naughty naughty...
		if(task_tracker != 0){
			uvPanic("SCD Timeout",0);
		}
		//TODO: Forcibly reconcile vehicle state, and nuke whatever tasks require nuking, suspend whatever needs suspended

		//END_OF_STATE_CHANGE_DAEMON:


	//}

	TaskHandle_t scd_handle = ((state_change_daemon_args*)args)->meta_task_handle;
	uvFree(args);

	/**This line deletes the queue to free up the memory @code */
	vQueueDelete(state_change_queue);
	state_change_queue = NULL;
	/**@endcode*/

	SCD_active = false;
	/**The final act of the SCD, is to delete itself @code */
	vTaskDelete(scd_handle);
	/** @endcode */

}
//end of SCD

uv_status uvThrottleNonCritTasks(){

}


void uvLateTaskHandler(uv_task_info* t ,TickType_t tdiff , uint8_t task_tardiness){

	if(t == NULL){
		//bruh
	}

	if(tdiff == portMAX_DELAY){
		//only base task lateness on the tardiness level, ignore tdiff
	}

	if(t->task_flags & UV_TASK_MISSION_CRITICAL){
		//SHIT FUCK SHIT WE GOTTA STOP THE CAR
		if(task_tardiness >= 2 || tdiff > (t->task_period * 2)){
			uvPanic("Critical Task Excessively Delayed",0);//This means task delayed AF
		}else{

		}

	}

}

/** @brief The big papa task that deals with handling all of the others.
 *
 * The responsibilities of this task are as follows:
 * - Monitor tasks to ensure they are on schedule
 * - Setup inter-task communication channels?
 * - Invoke SCD if necessary
 * - Track mem usage if needed
 *
 * This task is one of the most important ones in the system. Lovely times for all.
 * Therefore it us of utmost importance that this one DOES NOT CRASH. EVER.
 *
 */
void uvTaskManager(void* args) PRIVILEGED_FUNCTION{
	uv_task_info* params = (uv_task_info*) args;
	task_management_info* tmi = params->tmi; //Task Manager interface;
	tmi->task_handle = params->task_handle;
	tmi->parent_msg_queue = xQueueCreate(8,sizeof(uv_task_msg));//our good ol friend the message queue
	//Init the variables we need
	uv_task_msg incoming_msg;
	uv_msg_type incoming_msg_type = UV_INVALID_MSG;



	last_task_start_times = uvMalloc(MAX_NUM_MANAGED_TASKS*sizeof(TickType_t));
	last_task_end_times = uvMalloc(MAX_NUM_MANAGED_TASKS*sizeof(TickType_t));
	task_tardiness = uvMalloc(MAX_NUM_MANAGED_TASKS*sizeof(uint8_t));

	uint32_t late_tasks = 0;


	BaseType_t queue_status;

	for(;;){ //Loop checking for late tasks


		//vTaskDelayUntil( &last_time, tick_period);
		if(params->cmd_data == UV_KILL_CMD){

			//TASK DESTRUCTOR: CLEAN UP ANY RESOURCES USED BY THE TASK HERE

			uvFree(last_task_start_times);
			uvFree(last_task_end_times);
			uvFree(task_tardiness);

			last_task_start_times = NULL;
			last_task_end_times = NULL;
			task_tardiness = NULL;


			killSelf(params);
		}else if(params->cmd_data == UV_SUSPEND_CMD){

			//TASK SUSPENSION DESTRUCTOR: RELEASE THINGS LIKE MUTICES OR SEMAPHORES, BUT NO NEED TO DEALLOCATE ANY MEMORY

			suspendSelf(params);
		}


		uv_task_info* tmp = NULL;
		TickType_t time = xTaskGetTickCount();
		TickType_t threshold = 0xFF;
		TickType_t tdiff = 0;
		for(int i = 0; i<_next_task_id; i++){//iterate through the tasks
			tmp = &(_task_register[i]);

			if((tmp->task_flags & UV_TASK_MANAGER_MASK)!= UV_TASK_VEHICLE_APPLICATION){
				continue;
			}

			if(tmp->task_state != UV_TASK_RUNNING){
				task_tardiness[i] = 0;
				continue;
			}

			if(task_tardiness[i] > 2){
				uvLateTaskHandler(tmp, portMAX_DELAY, task_tardiness[i]);
			}

			if(tmp->task_flags & UV_TASK_MISSION_CRITICAL){
				threshold = (tmp->task_period)*nc_throttling*(os_settings->task_overshoot_margin_crit);
			}else{
				threshold = (tmp->task_period)*nc_throttling*(os_settings->task_overshoot_margin_noncrit);
			}

			tdiff = time - tmp->last_execution_time;

			if(tdiff > threshold){
				//YOU're LATE
				late_tasks |= (0x01U<<i);
				task_tardiness[i] ++;
				uvLateTaskHandler(tmp, tdiff, task_tardiness[i]);
			}else{
				//perhaps I judged you too harshly
				if(task_tardiness[i] > 0){
					task_tardiness[i]--;
				}
				late_tasks &= (0x01U<<i);
			}

		}











		//HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_15); //BLUE

		if(params->cmd_data == UV_KILL_CMD || params->cmd_data == UV_SUSPEND_CMD){
			continue; // The idea here is to skip the delay
		}

		uvTaskPeriodEnd(params);
	}


}

/** @brief Create a new service task, because fuck you, thats why
 *
 *
 */
uv_task_info* uvCreateServiceTask(){

	if(_next_task_id >= MAX_NUM_MANAGED_TASKS){
		return NULL;
	}

	/** Acquire the pointer to the spot in the array, we are doing this since we need to
	 * return the pointer anyways, and it cleans up the syntax a little.
	 *
	 */

	uv_task_info* _newtask = &(_task_register[_next_task_id]);
	_newtask->task_id = _next_svc_task_id;
	++_next_task_id;

	_newtask->task_name = NULL;

	_newtask->task_function = NULL;
	_newtask->task_priority = osPriorityRealtime; //DEPRECATED PRIORITY SYSTEM, GET THAT SHIT OUTTA HERE


	_newtask->stack_size = _UV_DEFAULT_TASK_STACK_SIZE;

	_newtask->task_state = UV_TASK_NOT_STARTED;
	_newtask->active_states = 0x00;
	_newtask->deletion_states = 0x00;
	_newtask->suspension_states = 0x00;

	_newtask->parent = NULL;

	_newtask->task_handle = NULL;

	_newtask->task_flags = UV_TASK_GENERIC_SVC | UV_TASK_SCD_IGNORE;

	return _newtask;
}

/** @brief Function to start a service task specifically
 *
 */
uv_status uvStartSVCTask(uv_task_info* t){
	if(t == NULL){
		return UV_ERROR;
	}

	if(!(t->task_flags & UV_TASK_GENERIC_SVC)){ //not a svc task
		return UV_ERROR;
	}

	if(t->task_state == UV_TASK_RUNNING){
		if(t->task_handle == NULL){
			return UV_ERROR;
		}
		return UV_ABORTED; //only one instance of this particular task is permitted
	}

	if(t->task_state == UV_TASK_SUSPENDED){
		if(t->task_handle == NULL){
			return UV_ERROR;
		}
		vTaskResume(t->task_handle);
	}

	BaseType_t retval;
	retval = xTaskCreate(t->task_function,t->task_name,t->stack_size,t->task_args,t->task_priority,&(t->task_handle));
	//This creates a task cause the necessary conditions have been met

	if(retval != pdPASS){
		//Something has gone wrong with the task creation.
		return UV_ERROR;
	}

	return UV_OK;

}

/** @brief Function that suspends a service task
 *
 */
uv_status uvSuspendSVCTask(uv_task_info* t){
	if(t == NULL){
		return UV_ERROR;
	}

	if(t->task_state == UV_TASK_SUSPENDED){
		return UV_ABORTED;
	}

	return UV_OK;
}

/** @brief For when you need to delete a service task... for some reason...
 *
 */
uv_status uvDeleteSVCTask(uv_task_info* t){
	if(t == NULL){
		return UV_ERROR;
	}

	if((t->task_state == UV_TASK_NOT_STARTED) || (t->task_state == UV_TASK_DELETED)){
		return UV_ABORTED;
	}

	if((t->task_state) == UV_TASK_SUSPENDED){
		//Somebody has suspended the task. Get it's ass
		vTaskDelete(t->task_handle);
		return UV_OK;
	}

	if(t->task_state == UV_TASK_RUNNING){
		//kindly tell it to fucking stop
		t->cmd_data = UV_KILL_CMD;
	}

	return UV_OK;
}

/** @brief Function that takes a service part that may be messed up and tries to reboot it to recover
 *
 * This may be neccessary if a SVC task is not responding. Be careful though, since this has the potential to delay more important tasks :o
 * Therefore, this technique should be used sparingly, and each task gets a limited number of attempts within a certain time period.
 */
uv_status uvRestartSVCTask(uv_task_info* t){
	//force task to relinquish resources

	//ask nicely for task to be deleted

	if(uvDeleteSVCTask(t) == UV_ERROR){
		//if that fails, forcibly delete task
		uvKillTaskViolently(t);
	}

	//try to start the task again
	if(uvStartSVCTask(t) == UV_OK){
		return UV_OK;
	}

	return UV_ERROR;


}

/** @brief oversees all of the service tasks, and makes sure that theyre alright
 *
 *
 */
void uvSVCTaskManager(void* args){

	task_management_info* params =(task_management_info*) args;

	/** Start all of the service tasks. This involves allocating neccessary memory,
	 * setting the appropriate task parameters, and saying "fuck it we ball" and adding the tasks to the
	 * central task tracking data structure.
	 *
	 *
	 */


	//_svc_task_register = uvMalloc(sizeof(uv_task_info)*MAX_NUM_SVC_TASKS); //allocate mem for the svc task register


	if(_task_register == NULL){
		__uvInitPanic(); //Double Plus Ungood
	}

	uv_task_info* canTxtask = uvCreateServiceTask();
	canTxtask->task_function = CANbusTxSvcDaemon;
	canTxtask->active_states = 0xFFFF;
	canTxtask->task_name = CAN_TX_DAEMON_NAME;

	uv_task_info* canRxtask = uvCreateServiceTask();
	canRxtask->task_function = CANbusRxSvcDaemon;
	canRxtask->active_states = 0xFFFF;
	canRxtask->task_name = CAN_RX_DAEMON_NAME;
	//super basic for now, just need something working
	uint32_t var = 0; //retarded dummy var
	uvStartTask(&var,canTxtask);
	uvStartTask(&var,canRxtask);

	//vTaskSuspend(params->task_handle);
	//iterate through the list

	for(;;){
		//Hold for a command
		vTaskDelay(100);
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

	//uvFree(_svc_task_register); //Free this to avoid mem leaks
	vTaskDelete(params->task_handle);
}

int compareTaskByName(uv_task_info* t1, uv_task_info* t2){

	if(t1 == NULL || t2 == NULL){
		return 0;
	}

	char* name1 = t1->task_name;
	char* name2 = t2->task_name;

	for(int i = 0; i<16; i++){
		if(name1[i] == '\0'){
			if(name2[i] == '\0'){
				return 0;
			}else{
				return -1; //name 2 has not ended but name 1 has
			}
		}else if(name2[i] == '\0'){
			return 1;
		}


		if(name1[i] > name2[i]){
			return 1;
		}else if (name1[i] < name2[i]){
			return -1;
		}
	}

	return 0;


}

/** Sometimes you just gottta deal with it lol
 *
 */
uv_task_info* uvGetTaskFromName(char* tsk_name){
	return NULL;
}

/** @brief Returns the pointer to the task info structure
 *
 * @param t_handle A freeRTOS task handle.
 *
 * @retval A pointer to a uv_task_info data structure. This is mostly useful for cases where you know the
 * RTOS handle, but not the task info struct
 *
 */
uv_task_info* uvGetTaskFromRTOSHandle(TaskHandle_t t_handle){
	if(t_handle == NULL){
		return NULL;
	}
	return NULL;
}

/** @} */ //end of backend

void uvTaskPeriodEnd(uv_task_info* t){
	//BaseType_t successful_delay;
	bool is_late = 0;

	TickType_t end_of_loop_time = xTaskGetTickCount;

	if((end_of_loop_time - t->last_execution_time)>t->task_period){
		is_late = 1;
		if(task_tardiness != NULL){
			task_tardiness[t->task_id]++; //missed a period
		}
	}else{
		if(task_tardiness != NULL){
			if(task_tardiness[t->task_id] > 0){
				task_tardiness[t->task_id]--; //missed a period
			}
		}
	}




	if(!is_late){// if the task identifies that it is late, it will try to catch up by not bothering with the delay
		uvTaskSetDelayBit(t);
		vTaskDelayUntil(&t->last_execution_time,pdMS_TO_TICKS(t->task_period * nc_throttling));
		uvTaskResetDelayBit(t);
	}

	t->last_execution_time = xTaskGetTickCount();




}

/** @}
 *
 */
