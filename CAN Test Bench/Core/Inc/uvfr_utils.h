/*
 * uvfr_utils.h
 *
 *  Created on: Oct 7, 2024
 *      Author: byo10
 */

#ifndef INC_UVFR_UTILS_H_
#define INC_UVFR_UTILS_H_

#include "uvfr_global_config.h"

#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "can.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "spi.h"

#include "FreeRTOS.h"
#include "task.h"

#include "uvfr_settings.h"
#include "uvfr_state_engine.h"

#include "bms.h"
#include "motor_controller.h"
#include "dash.h"
#include "imd.h"
#include "pdu.h"
#include "daq.h"

//Only used for debugging
#include "oled.h"

//mainstay meat and potatoes tasks
#include "driving_loop.h"
#include "temp_monitoring.h"
#include "odometer.h"

#include "FreeRTOSConfig.h"

//#include "stdlib.h"
#include "stdint.h"
#include <stdlib.h>


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

#define deserializeSmallE16(x,i) ((x[i])|(x[i+1]<<8))
#define deserializeSmallE32(x,i) ((x[i])|(x[i+1]<<8)|(x[i+2]<<16)|(x[i+3]<<24))
#define deserializeBigE16(x,i) ((x[i]<<8)|(x[i+1]))
#define deserializeBigE32(x,i) ((x[i]<<24)|(x[i+1]<<16)|(x[i+2]<<8)|(x[i+3]))

#define serializeSmallE16(x,d,i) x[i]=d&0x00FF; x[i+1]=(d&0xFF00)>>8
#define serializeSmallE32(x,d,i) x[i]=d&0x000000FF; x[i+1]=(d&0x0000FF00)>>8; x[i+2]=(d&0x00FF0000)>>16; x[i+3]=(d&0xFF000000)>>24
#define serializeBigE16(x,d,i) x[i+1]=d&0x00FF; x[i]=(d&0xFF00)>>8
#define serializeBigE32(x,d,i)x[i+3]=d&0x000000FF; x[i+2]=(d&0x0000FF00)>>8; x[i+1]=(d&0x00FF0000)>>16; x[i]=(d&0xFF000000)>>24


#define false 0
#define true !false

//Typedefs used throughout vehicle

typedef uint8_t bool;
typedef uint8_t uv_task_id;
typedef enum uv_task_cmd_e uv_task_cmd;
//typedef enum
typedef uint8_t uv_ext_device_id;
typedef uint32_t uv_timespan_ms;



//Time limits for the initialization. Car has 2.5 seconds to initialize all peripherals, before it decides that something has gone horrifically wrong
#define MAX_INIT_TIME 2500//if the car takes more than 2.5 seconds to boot, something seems a little fishy
#define INIT_CHECK_PERIOD 100

//Memory management macros



//Some fun CAN macros
#define UV_CAN1
#define UV_CAN2

//Feature flags
#define USE_OLED_DEBUG 1

/**	@brief This is meant to be a return type from functions that indicates what is actually going on
 *
 * Use this as a return value for functions you want to know the success of. In general,
 * any function you write must return something, as well as account for any possible errors
 * that may have occurred.
 *
 */
typedef enum uv_status_t{
	UV_OK,
	UV_WARNING,
	UV_ERROR,
	UV_ABORTED //nothing wrong per say,
}uv_status;



typedef enum uv_task_state_t{
	UV_TASK_NOT_STARTED,
	UV_TASK_DELETED,
	UV_TASK_RUNNING,
	UV_TASK_SUSPENDED
} uv_task_status;

typedef enum task_priority{
	IDLE_TASK_PRIORITY,
	LOW_PRIORITY,
	BELOW_NORMAL,
	MEDIUM_PRIORITY,
	ABOVE_NORMAL,
	HIGH_PRIORITY,
	REALTIME_PRIORITY
}task_priority;

/** Type made to represent the state of the vehicle, and the location in the state machine
 *	The states are powers of two to make it easier to discern tasks that need to happen in multiple states
 */
typedef enum uv_vehicle_state_t{
	UV_INIT = 0x0001, //Vehicle is Initialising
	UV_READY = 0x0002, //Vehicle is ready to drive, but not actually driving
	PROGRAMMING = 0x0004, //Vehicle is in proccess programming
	UV_DRIVING = 0x0008, //actively driving
	UV_SUSPENDED = 0x0010,
	UV_LAUNCH_CONTROL = 0x0020, //Launch control doing it's thing
	UV_ERROR_STATE = 0x0040,
	UV_BOOT = 0x0080,
	UV_HALT = 0x0100
}vehicle_state;

//not really sure how I want this implemented yet. Variable number of driving modes?
enum uv_driving_mode_t{
	normal,
	accel,
	econ,
	limp
};

/** @brief ID for external devices, which allows us to know what's good with them.
 *
 */
enum uv_external_device{
	MOTOR_CONTROLLER = 0,
	BMS = 1,
	IMD = 2,
	PDU = 3
};

typedef enum access_control_t{
	UV_NONE,
	UV_DUMB_FLAG,
	UV_MUTEX,
	UV_BINARY_SEMAPHORE,
	UV_SEMAPHORE
}access_control_type;

struct uv_mutex_info{
	SemaphoreHandle_t handle;

};

struct uv_binary_semaphore_info{
	SemaphoreHandle_t handle;

};

struct uv_semaphore_info{
	SemaphoreHandle_t handle;

};

typedef union access_control_info{
	struct uv_mutex_info mutex;
	struct uv_binary_semaphore_info bin_semaphore;
	struct uv_semaphore_info semaphore;

}access_control_info;


#define UV_CAN_EXTENDED_ID 0x01
#define UV_CAN_CHANNEL_MASK 0b00000110
typedef struct uv_CAN_msg{
	uint8_t flags;
	//Bit 0: extended id?
	//Bit [1:2]: which actual CANbus do I use?
	//Bit 3: dynamically allocated
	uint8_t dlc;
	uint32_t msg_id;
	uint8_t data[8];
}uv_CAN_msg;


/** contains info relevant to initializing the vehicle
 *
 */
typedef struct uv_init_struct{
	bool use_default_settings;//Flag for using default settings

}uv_init_struct;


#define UV_TASK_VEHICLE_APPLICATION 0b00000001
#define UV_TASK_PERIODIC_SVC        0b00000010
#define UV_TASK_DORMANT_SVC         0b00000011
#define UV_TASK_MANAGER_MASK        0b00000011
#define UV_TASK_LOG_START_STOP_TIME 0b00000100
#define UV_TASK_LOG_MEM_USAGE		0b00001000
#define UV_TASK_SCD_IGNORE			0b00010000


/** @brief This struct is designed to hold neccessary information about an RTOS task that
 * will be managed by uvfr_state_engine.
 *
 * @c task_id is an internal ID assigned to every task and is used by uvfr_utils.
 * @c task_name is the name of the task. This must be less than 15 characters, and ended with
 * a null terminator.
 *
 * @c task_function is a pointer to the function we want the task to run
 * @c task_priority is the priority of the task
 * @c max_instances is the number of legal instances
 *
 * @c task_state is an internal representation of what the task is up to. This is needed because the RTOS task may not actually exist.
 * @c active_states dictates when the states are active
 *
 * @c task_handle is an osThreadId created by FreeRTOS when it initializes the task.
 * @c next exists because this shit needs to exist in a linked list, and my ass is too lazy to create a generic struct with it
 *
 * @c cmd_data stores the command we really want this task to execute
 */
typedef struct uv_task_info{
	uv_task_id task_id;
	char* task_name;

	uv_timespan_ms task_period;

	TaskFunction_t task_function; //the thread function
	osPriority task_priority; //priority of the task

	uint8_t max_instances; //max number of task instances running at any moment
	uint32_t stack_size; //stack size allocated to task

	uint8_t num_instances; //number of instances running

	uint8_t task_flags; //Some task flags for ya
	//Bits 0:1 - | Task MGMT | Vehicle Application task - 01 | Periodic SVC Task - 10 | Dormant SVC Task - 11
	//Bit 2 - Log task start + stop time
	//Bit 3 - Log mem usage
	//Bit 4 - SCD ignore flag
	//Bit 5
	//Bit 6
	//Bit 7

	uv_task_status task_state; //tracks the internal state of the task
	uint16_t active_states; //corresponds to the vehicle states where the task should be active
	uint16_t deletion_states; //corresponds to the vehicle states where the task should be suspended
	uint16_t suspension_states; //when should the task be suspended? When it should exist, but shouldnt be active.

	TaskHandle_t task_handle;
	struct uv_task_info* next;
	uv_task_cmd cmd_data; //every task gets it's own personal queue to have commands in it

	void* task_args; //specific arguments for that task. Probably comes from the vehicle settings.

	QueueHandle_t manager;//TODO: Fix type/impliment a better manager system than the SCD


}uv_task_info;


typedef struct tm_data{
	//

}task_management_data;




typedef struct p_status{
	uv_status peripheral_status;
	TickType_t activation_time;

}p_status;


/** @brief Struct designed to act like the @c uv_task_info struct,
 * but for the initialisation tasks. As a result it takes fewer arguments.
 *
 *
 */
typedef struct uv_init_task_args{
	void* specific_args; //anything specific here? This probably comes from the device settings.
	QueueHandle_t init_info_queue; //reference to the initialisation queue
	TaskHandle_t meta_task_handle; //Handle to itself, which it can use to delete itself
}uv_init_task_args;


/** @brief Data used by the uvfr_utils library to do what it needs to do :)
 *
 * This is a global variable that is initialized at some point at launch
 *
 */
typedef struct uv_internal_params{
	uv_init_struct* init_params;
	uv_vehicle_settings* vehicle_settings;
	p_status peripheral_status[];



}uv_internal_params;


/** @brief Struct representing the response of one of the initialization tasks.
 *
 * Is returned in the initialization queue, and is read by @c uvInit() to determine
 * whether the initialization of the internal device has failed or succeeded.
 *
 */
typedef struct uv_init_task_response{
	uv_status status; //Did it succeed? This gets to be a UV_OK if success
	uv_ext_device_id device; //Which device was it
	uint8_t nchar;
	char* errmsg; //if we didn't succeed, then what went wrong?
}uv_init_task_response;



#ifndef UV_UTILS_SRC_IMPLIMENTATION
	extern uv_internal_params global_context;
#endif

void uvInit(void * arguments);

void __uvInitPanic();

#ifndef uvMalloc
#if USE_OS_MEM_MGMT
	void* __uvMallocOS(size_t memrequest);
	#define uvMalloc(x) __uvMallocOS(x)
#else //default to STDlib
	void * __uvMallocCritSection(size_t memrequest);
	#define uvMalloc(x) __uvMallocCritSection(x)

#endif //OS mem mgmt?
#endif //allow macro override

#ifndef uvFree
#if USE_OS_MEM_MGMT
	uv_status __uvFreeOS(void* ptr);
	#define uvFree(x) __uvFreeOS(x)

#else
	uv_status __uvFreeCritSection(void* ptr);
	#define uvFree(x) __uvFreeCritSection(x)

#endif //OS mem mgmt
#endif //Allows macro overriding

uv_status uvIsPTRValid(void* ptr);





#endif /* INC_UVFR_UTILS_H_ */
