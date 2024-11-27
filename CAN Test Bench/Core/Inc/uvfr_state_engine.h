/*
 * uvfr_state_engine.h
 *
 *  Created on: Oct 15, 2024
 *      Author: byo10
 */

#ifndef INC_UVFR_STATE_ENGINE_H_
#define INC_UVFR_STATE_ENGINE_H_


#include "uvfr_utils.h"


//#include "uvfr_utils.h"
typedef enum uv_status_t uv_status;
typedef uint8_t uv_task_id; //WHY DO I NEED TO DO THIS STUPID REDEFINITION HERE

#define _UV_DEFAULT_TASK_INSTANCES 128
//STACK size 0, means that it uses operating system defaults
#define _UV_DEFAULT_TASK_STACK_SIZE 128
//period of 100ms, aka 10Hz
#define _UV_DEFAULT_TASK_PERIOD 100
#define _UV_MIN_TASK_PERIOD 5

#define _LONGEST_SC_TIME 300
#define _SC_DAEMON_PERIOD 10

#define SVC_TASK_MAX_CHECKIN_PERIOD 500

//typedef uint8_t uv_task_cmd;

typedef enum uv_task_cmd_e{
	UV_NO_CMD,
	UV_KILL_CMD,
	UV_SUSPEND_CMD,
	UV_TASK_START_CMD
}uv_task_cmd;

enum uv_scd_response_e{
	UV_SUCCESSFUL_DELETION,
	UV_SUCCESSFUL_SUSPENSION,
	UV_COULDNT_DELETE,
	UV_COULDNT_SUSPEND,
	UV_UNSAFE_STATE
};

typedef struct uv_scd_response{
	enum uv_scd_response_e response_val;
	uv_task_id meta_id;
}uv_scd_response;

//typedef struct uv_svc_task_tcb{
//	struct uv_task_info; // editable by user
//	//State engine private fields below
//}b;
//
//typedef union uv_svc_task{
//	struct uv_task_info; // user task info
//	struct uv_svc_task_tcb;
//}a;


typedef struct task_management_info{
	TaskHandle_t task_handle;
	QueueHandle_t task_management_queue;
}task_management_info;


struct uv_task_info* uvCreateTask();

struct uv_task_info* uvGetTaskById(uint8_t id);

uv_status _uvValidateSpecificTask(uint8_t id);

uv_status uvValidateManagedTasks();

uv_status uvStartTask(uint32_t* tracker,struct uv_task_info * t);

uv_status uvRegisterTask();

uv_status uvInitStateEngine();

uv_status uvStartStateMachine();

//static enum uv_status_t killEmAll();

uv_status uvDeleteTask(uint32_t* tracker,struct uv_task_info * t);

uv_status uvSuspendTask(uint32_t* tracker,struct uv_task_info * t);

uv_status uvDeInitStateEngine();

uv_status updateRunningTasks();

uv_status changeVehicleState(uint16_t state);

//void uvPanic(char* msg, uint8_t msg_len); //ruh roh scoobs, something has gone a little bit fucky wucky
void __uvPanic(char* msg, uint8_t msg_len, const char* file, const int line, const char* func);

#ifndef uvPanic
#define uvPanic(x, y) __uvPanic(x, y, __FILE__,__LINE__,__FUNCTION__)
#endif

void killSelf(struct uv_task_info * t);

void suspendSelf(struct uv_task_info * t);

#ifndef UVFR_STATE_MACHINE_IMPLIMENTATION

//EXTERNAL VARIABLES
extern enum uv_vehicle_state_t vehicle_state; //This is the one that ya'll are permitted to know
#else
//These shuold only be visible in the implimentation file

#endif

void _stateChangeDaemon(void * args);

void uvSVCTaskManager(void* args);

uv_task_id getSVCTaskID(char* tsk_name);
#endif /* INC_UVFR_STATE_ENGINE_H_ */


