/*
 * uvfr_utils.h
 *
 *  Created on: Oct 7, 2024
 *      Author: byo10
 */

#ifndef INC_UVFR_UTILS_H_
#define INC_UVFR_UTILS_H_

#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "can.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "uvfr_settings.h"
#include "uvfr_state_engine.h"

#include "bms.h"
#include "motor_controller.h"
#include "dash.h"
#include "imd.h"

#include "FreeRTOSConfig.h"

#include "stdlib.h"
#include "stdint.h"


/* Some syntactic sugar for ya'll to use in your code
 * Defaults to 16 bit unsigned int
 */
#define _BV(x) _BV_16(x)
#define _BV_8(x) ((uint8_t)(0x01U >> x))
#define _BV_16(x) ((uint16_t)(0x01U >> x))
#define _BV_32(x) ((uint32_t)(0x01U >> x))

#define endianSwap(x) endianSwap16(x)
#define endianSwap8(x) x //if someone calls this, they are mentally retarded, but here ya go I guess
#define endianSwap16(x) (((x & 0x00FF)<<8) | ((x & 0xFF00)>>8))
#define endianSwap32(x) (((x & 0x000000FF)<<16)|((x & 0x0000FF00)<<8)|((x & 0x00FF0000)>>8)|((x & 0xFF000000)>>16))

#define false 0
#define true !false

typedef uint8_t bool;
typedef uint8_t uv_task_id;
typedef uint8_t uv_status;
typedef uint8_t uv_ext_device_id;
typedef uint32_t uv_timespan_ms;

#define MAX_INIT_TIME 2500//if the car takes more than 2.5 seconds to boot, something seems a little fishy

#define INIT_CHECK_PERIOD 100

//TASK MANAGEMENT MACROS

/**	@brief This is meant to be a return type from functions that indicates what is actually going on
 *
 * Use this for things like initializing characters or
 *
 */
enum uv_status_t{
	UV_OK,
	UV_WARNING,
	UV_ERROR,
	UV_ABORTED
};



enum uv_task_state{
	UV_TASK_NOT_STARTED,
	UV_TASK_DELETED,
	UV_TASK_RUNNING,
	UV_TASK_SUSPENDED
};

/** Type made to represent the state of the vehicle, and the location in the state machine
 *	The states are powers of two to make it easier to discern tasks that need to happen in multiple states
 */
enum uv_vehicle_state_t{
	UV_INIT = 0x0001,
	UV_READY = 0x0002,
	PROGRAMMING = 0x0004,
	UV_DRIVING = 0x0008,
	UV_SUSPENDED = 0x0010,
	UV_LAUNCH_CONTROL = 0x0020,
	UV_ERROR_STATE = 0x0040,
	UV_BOOT = 0x0080,
	UV_HALT = 0x0100
};

//not really sure how I want this implemented yet. Variable number of driving modes?
enum uv_driving_mode_t{
	normal,
	accel,
	econ,
	limp
};

enum uv_external_device{
	MOTOR_CONTROLLER = 0,
	BMS = 1,
	IMD = 2,
	PDU = 3
};

typedef struct uv_init_struct{
	bool use_default_settings;

}uv_init_struct;




/** @brief This struct is designed to hold neccessary information about an RTOS task that
 * will be managed by uvfr_state_engine.
 *
 * @c task_id is an internal ID assigned to every task and is used by uvfr_utils.
 * @c task_name is the name of the task. This must be less than 15 characters, and ended with
 * a null terminator.
 *
 * @c task_function is a pointer to the function we want the task to run
 * @c task_priority is the priority of the task
 * @c instances is the number of legal instances
 *
 * @c task_state is an internal representation of what the task is up to. This is needed because the RTOS task may not actually exist.
 * @c active_states dictates when the states are active
 *
 * @c task_handle is an osThreadId created by FreeRTOS when it initializes the task.
 * @c next exists because this shit needs to exist in a linked list, and my ass is too lazy to create a generic struct with it
 *
 * @field cmd_data stores the command we really want this task to execute
 */
typedef struct uv_task_info{
	uv_task_id task_id;
	char* task_name;

	os_pthread task_function; //the thread function
	osPriority task_priority; //priority of the task

	uint32_t instances; //max number of task instances running at any moment
	uint32_t stack_size; //stack size allocated to task


	enum uv_task_state task_state; //tracks the internal state of the task
	uint16_t active_states; //corresponds to the vehicle states where the task should be active
	uint16_t deletion_states; //corresponds to the vehicle states where the task should be suspended
	uint16_t suspension_states; //when should the task be suspended? When it should exist, but shouldnt be active.

	osThreadId task_handle;
	struct uv_task_info* next;
	uv_task_cmd cmd_data; //every task gets it's own personal queue to have commands in it

	void* task_args;



}uv_task_info;


/** @brief Struct designed to act like the @c uv_task_info struct,
 * but for the initialisation tasks. As a result it takes fewer arguments.
 *
 *
 */
typedef struct uv_init_task_args{
	void* specific_args;
	QueueHandle_t init_info_queue;
	osThreadId meta_task_handle;
}uv_init_task_args;

typedef struct uv_internal_params{


}uv_internal_params;

typedef struct uv_init_task_response{
	uv_status status;
	uv_ext_device_id device;
	uint8_t nchar;
	char* errmsg;
}uv_init_task_response;

void uvInit(void * arguments);

void _uvInitPanic();

//this works exactly the same way that malloc does
void * uvMalloc(size_t size);

//this works exactly the same way that free does
//enum uv_status_t uvFree(void * mem);
//
//enum uv_status_t uvUtilsInit();



#endif /* INC_UVFR_UTILS_H_ */
