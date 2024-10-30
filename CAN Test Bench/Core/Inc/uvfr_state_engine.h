/*
 * uvfr_state_engine.h
 *
 *  Created on: Oct 15, 2024
 *      Author: byo10
 */

#ifndef INC_UVFR_STATE_ENGINE_H_
#define INC_UVFR_STATE_ENGINE_H_

#include "uvfr_utils.h"



#include "uvfr_utils.h"

#define _UV_DEFAULT_TASK_INSTANCES 128

//STACK size 0, means that it uses operating system defaults
#define _UV_DEFAULT_TASK_STACK_SIZE 0

#define _LONGEST_SC_TIME 100
#define _SC_DAEMON_PERIOD 10

typedef uint8_t uv_task_cmd;

enum uv_task_cmd_e{
	UV_KILL_CMD,
	UV_SUSPEND_CMD
};

struct uv_task_info* uvCreateTask();

struct uv_task_info* uvGetTaskById(uint8_t id);

enum uv_status_t uvRegisterTask();

enum uv_status_t uvInitStateEngine();

enum uv_status_t uvStartStateMachine();

enum uv_status_t killEmAll();

enum uv_status_t uvDeleteTask();

enum uv_status_t uvSuspendTask();

enum uv_status_t uvDeInitStateEngine();

enum uv_status_t updateRunningTasks();

enum uv_status_t changeVehicleState(uint16_t state);

void uvPanic(char* msg, uint8_t msg_len); //ruh roh scoobs, something has gone a little bit fucky wucky

void _stateChangeDaemon(void * args);

#ifndef UVFR_STATE_MACHINE_IMPLIMENTATION

//EXTERNAL VARIABLES
extern enum uv_vehicle_state_t vehicle_state;

#endif

#endif /* INC_UVFR_STATE_ENGINE_H_ */


