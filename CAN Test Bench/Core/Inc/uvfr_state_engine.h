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
typedef uint8_t uv_status;
typedef uint8_t uv_task_id; //WHY DO I NEED TO DO THIS STUPID REDEFINITION HERE

#define _UV_DEFAULT_TASK_INSTANCES 128
//STACK size 0, means that it uses operating system defaults
#define _UV_DEFAULT_TASK_STACK_SIZE 128
//period of 100ms, aka 10Hz
#define _UV_DEFAULT_TASK_PERIOD 100
#define _UV_MIN_TASK_PERIOD 5

#define _LONGEST_SC_TIME 300
#define _SC_DAEMON_PERIOD 10

typedef uint8_t uv_task_cmd;

enum uv_task_cmd_e{
	UV_NO_CMD,
	UV_KILL_CMD,
	UV_SUSPEND_CMD,
	UV_TASK_START_CMD
};

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

struct uv_task_info* uvCreateTask();

struct uv_task_info* uvGetTaskById(uint8_t id);

uv_status _uvValidateSpecificTask(uint8_t id);

uv_status uvValidateManagedTasks();

enum uv_status_t uvStartTask(uint32_t* tracker,struct uv_task_info * t);

enum uv_status_t uvRegisterTask();

enum uv_status_t uvInitStateEngine();

enum uv_status_t uvStartStateMachine();

//static enum uv_status_t killEmAll();

enum uv_status_t uvDeleteTask(uint32_t* tracker,struct uv_task_info * t);

enum uv_status_t uvSuspendTask(uint32_t* tracker,struct uv_task_info * t);

enum uv_status_t uvDeInitStateEngine();

enum uv_status_t updateRunningTasks();

enum uv_status_t changeVehicleState(uint16_t state);

//void uvPanic(char* msg, uint8_t msg_len); //ruh roh scoobs, something has gone a little bit fucky wucky
void __uvPanic(char* msg, uint8_t msg_len, const char* file, const int line, const char* func);

#define uvPanic(x, y) __uvPanic(x, y, __FILE__,__LINE__,__FUNCTION__)


void killSelf(struct uv_task_info * t);

void suspendSelf(struct uv_task_info * t);

#ifndef UVFR_STATE_MACHINE_IMPLIMENTATION

//EXTERNAL VARIABLES
extern enum uv_vehicle_state_t vehicle_state;
#else
//These shuold only be visible in the implimentation file
#endif
void _stateChangeDaemon(void * args);
#endif /* INC_UVFR_STATE_ENGINE_H_ */


