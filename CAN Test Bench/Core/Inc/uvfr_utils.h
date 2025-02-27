/**
 * @file uvfr_utils.h
 * @author Byron Oser
 *
 */

/**
 * @defgroup uvfr_utils UVFR Utilities
 * @brief Module containing useful functions and abstractions that are used throughout the vehicle software system
 *
 * This contains several abstractions such as useful macros, global typedefs, memory allocation, etc...
 */

/**
 * @defgroup utility_macros Utility Macros
 * @brief handy macros that perform very common functionality
 *
 * \ingroup uvfr_utils
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
#include "message_buffer.h"

#include "uvfr_settings.h"
#include "uvfr_state_engine.h"
#include "rb_tree.h"

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


/** @addtogroup utility_macros
 * @{
 *
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

/**@brief macro to set bits of an int without touching the ones we dont want to edit

 Usage:
 Will set the values of certain bits of an int. This depends on the following however:
 @param x represents the value you want to edit. Can be any signed or unsigned integer type.
 @param msk Bits of X will only be altered if the matching bit of msk is a 1
 @param data Bits of data will map to bits of x, provided that the corresponding bit of msk is a one

In practice this looks like the following:
@code
uint8_t num = 0xF0;  // int is 0b11110000
uint8_t mask = 0x22; // msk is 0b00100010
uint8_t data = 0x0F; // val is 0b00001111

//now we deploy the macro

setBits(num,mask,data);

//now, num = 0b11010010

@endcode

 */
#define setBits(x,msk,data) x=(x&(~msk)|data)

/** @brief Returns a truthy value if "x" is a power of two
 *
 */
#define isPowerOfTwo(x) (x&&(!(x&(x-1))))

/** @brief lil treat to help us avoid the dreaded null pointer dereference
 *
 */
#define safePtrRead(x) (*((x)?x:uvPanic("nullptr_deref",0)))
#define safePtrWrite(p,x) (*((p)?p:&x))


/** Wish.com Boolean */
#define false 0
#define true !false

/**@} */

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


/** @brief Represents the data type of some variable
 *
 */
typedef enum  {
	UV_UINT8,
	UV_INT8,
	UV_UINT16,
	UV_INT16,
	UV_UINT32,
	UV_INT32,
	UV_FLOAT,
	UV_DOUBLE,
	UV_INT64,
	UV_UINT64,
	UV_STRING

}data_type;


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

/** @brief Enum dictating the meaning of a generic message
 *
 */
typedef enum uv_msg_type_t{
	UV_TASK_START_COMMAND,
	UV_TASK_DELETE_COMMAND,
	UV_TASK_SUSPEND_COMMAND,
	UV_COMMAND_ACKNOWLEDGEMENT,
	UV_TASK_STATUS_REPORT,
	UV_ERROR_REPORT,
	UV_WAKEUP,
	UV_PARAM_REQUEST,
	UV_PARAM_READY,
	UV_RAW_DATA_TRANSFER,
	UV_SC_COMMAND,
	UV_INVALID_MSG,
	UV_ASSIGN_TASK

}uv_msg_type;

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
#define UV_CAN_DYNAMIC_MEM  0b00001000


/** @brief Representative of a CAN message
 *
 */
typedef struct uv_CAN_msg{
	uint8_t flags; /**< Bitfield that contains some basic information about the message:
	-Bit 0: Is the message an extended ID message, or a standard ID message? 1 For extended.
	-Bits 1:2 Which CANbus is being used to send the message? 01 -> CAN1 10 -> CAN2 11-> CAN3 (doesnt exist yet). Will default to CAN1 if all zeros*/

	uint8_t dlc; /**<Data Length Code, representing how many bytes of data are present*/
	uint32_t msg_id; /**<The ID of a message*/
	uint8_t data[8]; /**<The actual data packet contained within the CAN message */
}uv_CAN_msg;


/** contains info relevant to initializing the vehicle
 *
 */
typedef struct uv_init_struct{
	bool use_default_settings;//Flag for using default settings

}uv_init_struct;







/** @brief Struct containing a message between two tasks
 *
 * This is a generic type that is best used in situations where the message could mean a variety of different things.
 * For niche applications or where efficiency is paramount, we recommend creating a bespoke protocol.
 *
 */
typedef struct uv_task_msg_t{
	uint32_t message_type;
	uv_task_info* sender;
	uv_task_info* intended_recipient;
	TickType_t time_sent;
	size_t message_size;
	void* msg_contents;

}uv_task_msg;







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
	p_status peripheral_status[8];
	uint16_t e_code[8];
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
