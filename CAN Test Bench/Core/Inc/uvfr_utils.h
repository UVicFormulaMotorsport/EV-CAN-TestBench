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

#include "stdlib.h"


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


//TASK MANAGEMENT MACROS
#define uv_register_task(name, thread, priority, instances, stacksz, handle, args)\
osThreadDef(name, thread, priority, instances, stacksz);\
handle = osThreadCreate(osThread(name), args);\
/*	This is meant to be a return type from functions that indicates what is actually going on
 * Use this for things like initializing characters or
 *
 */
enum uv_status_t{
	UV_OK,
	UV_WARNING,
	UV_ERROR
};

typedef uint8_t uv_status;

enum uv_task_state{
	UV_DELETED,
	UV_RUNNING,
	UV_SUSPENDED
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

typedef struct uv_init_struct{
	bool use_default_settings;

}uv_init_struct;

typedef uint8_t uv_task_id;


/** @brief This struct is designed to hold neccessary information about an RTOS task that
 * will be managed by uvfr_state_engine.
 *
 * @field task_id is an internal ID assigned to every task and is used by uvfr_utils.
 * @field task_name is the name of the task. This must be less than 15 characters, and ended with
 * a null terminator.
 * @field task_handle is an osThreadId created by FreeRTOS when it initializes the task.
 *
 */
typedef struct uv_task_info{
	uv_task_id task_id;
	char task_name[16];


	enum uv_task_state task_state;
	uint16_t active_states;
	uint16_t deletion_states;
	uint16_t suspension_states;

	osThreadId task_handle;


} uv_task_info;

typedef struct uv_internal_params{


}uv_internal_params;


//this works exactly the same way that malloc does
void * uvMalloc(size_t size);

//this works exactly the same way that free does
//enum uv_status_t uvFree(void * mem);
//
//enum uv_status_t uvUtilsInit();



#endif /* INC_UVFR_UTILS_H_ */
