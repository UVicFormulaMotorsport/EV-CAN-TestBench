/*
 * uvfr_state_engine.h
 *
 *  Created on: Oct 15, 2024
 *      Author: byo10
 */

/**
 * @defgroup state_engine State Engine
 * @brief Module containing all of the functions needed for the vehicle state machine to work
 *
 * The state-engine is mission critical code for doing the following:
 * - Providing a state machine for the vehicle
 * - Providing infrastructure neccessary for the vehicle to change state, and behaving as a parent to all the RTOS tasks
 * - Providing an API to hide the nitty-gritty of interfacing with the operating system, mitigating race conditions, etc...
 *
 */

/**
 * @defgroup state_engine_api State Engine API
 *
 * @brief Provides publically available API for controlling vehicle state and error handling.
 *
 * The functions defined in this group are publicly accessible and can be called from either application or service tasks.
 * These are not neccessarily interrupt safe, and therefore should not be called from them, unless they end with FromISR
 *
 *\ingroup state_engine
 */

/**
 * @defgroup state_engine_backend State Engine Internals
 *
 * @attention Do not edit these functions, or even contemplate calling one of them directly unless you 100% know what you are doing.
 * These are DANGEROUS
 *
 * This handles all the under the hood bullshit inherent to a system that dynamically starts and restarts RTOS tasks.
 * Due to this being a safety critical system, great care must be taken to prevent the vehicle from entering an unsafe state
 * as a result of anything happening in these functions.
 *
 * \ingroup state_engine
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

typedef uint32_t uv_timespan_ms;

/**@addtogroup state_engine_api
 * @{
 */

/** @brief Type representing the overall state and operating mode of the vehicle.
 *
 * Type made to represent the state of the vehicle, and the location in the state machine
 *	The states are powers of two to make it easier to discern tasks that need to happen in multiple states
 */
typedef enum uv_vehicle_state_t{
	UV_INIT = 0x0001, /**< Vehicle is in the process of initializing */
	UV_READY = 0x0002, /**< Vehicle has initialized and is ready to drive*/
	PROGRAMMING = 0x0004, /**< The settings of the vehicle are being edited now*/
	UV_DRIVING = 0x0008, /**< The vehicle is actively driving*/
	UV_SUSPENDED = 0x0010, /**< The vehicle is not allowed to produce any torque, but not full shutdown*/
	UV_LAUNCH_CONTROL = 0x0020, /**< The vehicle is presently in launch control mode*/
	UV_ERROR_STATE = 0x0040, /**< Some error has occurred here*/
	UV_BOOT = 0x0080, /**< Pre-init, when the boot loader is going*/
	UV_HALT = 0x0100 /**< Stop literally everything, except for what is needed to reset vehicle*/
}uv_vehicle_state;

/** @brief Special commands used to start and shutdown tasks.
 *
 */
typedef enum uv_task_cmd_e{
	UV_NO_CMD,/**<The SCD has issued no command, and therefore no action is required */
	UV_KILL_CMD, /**< The SCD has decreed that this task must be deleted */
	UV_SUSPEND_CMD, /**< The SCD has decreed that this task must be suspended*/
	UV_TASK_START_CMD /**< OK for task to begin execution*/
}uv_task_cmd;

/** @brief Response from a task confirming it has been either deleted or suspended
 *
 */
enum uv_scd_response_e{
	UV_SUCCESSFUL_DELETION, /**< Returned when a task was successfully deleted*/
	UV_SUCCESSFUL_SUSPENSION,/**< Returned when a task is successfully suspended*/
	UV_COULDNT_DELETE, /**< Task was not successfully deleted*/
	UV_COULDNT_SUSPEND, /**< Task was not successfully suspended*/
	UV_UNSAFE_STATE /**< Task has ended up in a fucked middle ground state*/
};

typedef struct uv_scd_response{
	enum uv_scd_response_e response_val; /**< */
	uv_task_id meta_id; /**< */
}uv_scd_response;

/** @brief Enum representing the state of a managed task.
 *
 * This is used as a flag to indicate whether or not the state_engine is aware of a task is running or not.
 *
 */
typedef enum uv_task_state_t{
	UV_TASK_NOT_STARTED,
	UV_TASK_DELETED,
	UV_TASK_RUNNING,
	UV_TASK_SUSPENDED
} uv_task_status;


/** @brief Priority of a managed task. Maps directly to OS priority
 *
 */
typedef enum task_priority{
	IDLE_TASK_PRIORITY,
	LOW_PRIORITY,
	BELOW_NORMAL,
	MEDIUM_PRIORITY,
	ABOVE_NORMAL,
	HIGH_PRIORITY,
	REALTIME_PRIORITY
}task_priority;



/** @brief Struct to contain data about a parent task
 *
 * This contains the information required for the child task to communicate with it's parent.
 *
 * This will be a queue, since one parent task can in theory have several child tasks
 *
 */
typedef struct task_management_info{
	TaskHandle_t task_handle; /**<Actual handle of parent */
	QueueHandle_t parent_msg_queue; /**< */
}task_management_info;

/** @brief Information about the task.
 *
 */
typedef struct task_status_block{
	uint32_t task_high_water_mark; /**< */
	TickType_t task_report_time; /**< */
}task_status_block;

enum os_flag{
	UV_OS_LOG_MEM = 0x01,
	UV_OS_LOG_TASK_END_TIME = 0x02,
	UV_OS_ATTEMPT_RESTART_NC_TASK = 0x04,
	UV_OS_ENABLE_NONCRIT_TASK_THROTTLE = 0x08
};

/** @brief Settings that dictate state engine behavior
 *
 */
typedef struct uv_os_settings{
	TickType_t svc_task_manager_period;
	TickType_t task_manager_period;
	TickType_t max_svc_task_period;
	TickType_t max_task_period;//fuckin lethal man
	TickType_t min_task_period;
	float task_overshoot_margin_noncrit;
	float task_overshoot_margin_crit;
	float task_throttle_increment;

	uint16_t os_flags;

}uv_os_settings;


#define UV_TASK_VEHICLE_APPLICATION    0x0001U<<(0)
#define UV_TASK_PERIODIC_SVC           0x0001U<<(1)
#define UV_TASK_DORMANT_SVC            0b0000000000000011
#define UV_TASK_GENERIC_SVC			   0x0001U<<(2)
#define UV_TASK_MANAGER_MASK           0b0000000000000011
#define UV_TASK_LOG_START_STOP_TIME    0x0001U<<(2)
#define UV_TASK_LOG_MEM_USAGE		   0x0001U<<(3)
#define UV_TASK_SCD_IGNORE			   0x0001U<<(4)
#define UV_TASK_IS_PARENT			   0x0001U<<(5)
#define UV_TASK_IS_CHILD			   0x0001U<<(6)
#define UV_TASK_IS_ORPHAN			   0x0001U<<(7)
#define UV_TASK_ERR_IN_CHILD		   0x0001U<<(8)
#define UV_TASK_AWAITING_DELETION	   0x0001U<<(9)
#define UV_TASK_DEFER_DELETION		   0x0001U<<(10)
#define UV_TASK_DEADLINE_NOT_ENFORCED  0x00
#define UV_TASK_PRIO_INCREMENTATION    0x0001U<<(11)
#define UV_TASK_DEADLINE_FIRM		   0x0001U<<(12)
#define UV_TASK_DEADLINE_HARD		   (0x0001U<<(11)|0x0001U<<(12))
#define UV_TASK_DEADLINE_MASK		   (0x0001U<<(11)|0x0001U<<(12))
#define UV_TASK_MISSION_CRITICAL	   0x0001U<<(13)
#define UV_TASK_DELAYING			   0x0001U<<(14)


/** @brief This struct is designed to hold neccessary information about an RTOS task that
 * will be managed by uvfr_state_engine.
 *
 * Pay close attention, because this is one of the most cursed structs in the project, as well as one of the most important
 */
typedef struct uv_task_info{
	uv_task_id task_id; /**< Detailed description after the member */
	char* task_name; /**< Detailed description after the member */

	uv_timespan_ms task_period; /**< Maximum period between task execution*/
	uv_timespan_ms deletion_delay; /**< If deferred deletion is enabled, how long to wait before we delete task? */

	TaskFunction_t task_function; /**< Pointer to function that implements the task */
	osPriority task_priority; /**< Priority of the task. Int between 0 and 7 */


	uint32_t stack_size; /**< Number of words allocated to the stack of the task */




	uv_task_status task_state; //tracks the internal state of the task


	TaskHandle_t task_handle; /**< Handle of freeRTOS task control block */

	uv_task_cmd cmd_data; /**< how we communicate with the task rn - THIS SUCKS SO BAD */

	void* task_args; /**< arguments for the specific task, this is where we will likely pass in task settings */

	struct uv_task_info_t* parent;/**< info about the parent of the task */

	task_management_info* tmi; /**< how we will be communicating in the future */
	MessageBufferHandle_t task_rx_mailbox; /**< Incoming messages for this task*/
	TickType_t last_execution_time;

	uint16_t active_states; //corresponds to the vehicle states where the task should be active
	uint16_t deletion_states; //corresponds to the vehicle states where the task should be suspended
	uint16_t suspension_states; //when should the task be suspended? When it should exist, but shouldnt be active.

	uint16_t task_flags; /**<
		- Bits 0:1 - | Task MGMT | Vehicle Application task - 01 | Periodic SVC Task - 10 | Dormant SVC Task - 11
		- Bit 2  - Log task start + stop time
		- Bit 3  - Log mem usage
		- Bit 4  - SCD ignore flag (only use if task is application layer
		- Bit 5  - is parent
		- Bit 6	 - is child
		- Bit 7  - is orphaned
		- Bit 8	 - error in child task
		- Bit 9	 - awaiting deferred deletion
		- Bit 10 - deferred deletion enabled
		- Bits 11:12 - Deadline firmness | No enforcement - 00 | Gradual Priority Incrimentation - 01 | Firm deadline 10 | Critical Deadline - 11
		- Bit 13 - mission critical, if this specific task crashes, the car will not continue to run
		- Bit 14 - Task currently delaying, either by vTaskDelay or vTaskDelayUntil
		 */

	uint8_t throttle_factor; /**< How much to throttle the task */
}uv_task_info;

#define uvTaskSetDeletionBit(t) (t->task_flags|=UV_TASK_AWAITING_DELETION)
#define uvTaskResetDeletionBit(t) (t->task_flags &=(~UV_TASK_AWAITING_DELETION))

#define uvTaskSetDelayBit(t) (t->task_flags|=UV_TASK_DELAYING)

#define uvTaskResetDelayBit(t) (t->task_flags&=(~UV_TASK_DELAYING))

#define uvTaskIsDelaying(t) ((t->task_flags&UV_TASK_DELAYING)==UV_TASK_DELAYING)

/** @brief State engine aware vTaskDelay wrapper
 *
 * @param x
 * @param t is how long to delay in ticks
 */
#define uvTaskDelay(x,t) uvTaskSetDelayBit(x);\
	vTaskDelay(t);\
	uvTaskResetDelayBit(x)

/** @ingroup state_engine_api
 * @brief Function called at the end of the task period.
 *
 * @
 *
 *
 */
void uvTaskPeriodEnd(uv_task_info* t);

/** @brief State engine aware vTaskDelayUntil wrapper
 *
 * @param x
 * @param lasttim is the variable storing the last delay time.
 * @param per is the period.
 *
 * This will cause the task to wait until the last time + the period.
 */
#define uvTaskDelayUntil(x,lasttim,per) uvTaskSetDelayBit(x);\
	vTaskDelayUntil(&lasttim,per);\
	uvTaskResetDelayBit(x)

/**
 * @}
 */

struct uv_task_info* uvCreateTask();

struct uv_task_info* uvCreateServiceTask();

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
/** @brief Called when things have gone heinously wrong, and we would like to get off the ride
 *
 * This function is called in the event of a non-recoverable error. It puts the vehicle into a safe state, logs the error, and changes over
 * vehicle state.
 *
 */
#define uvPanic(msg, errnum) __uvPanic(msg, errnum, __FILE__,__LINE__,__FUNCTION__)
#endif

void killSelf(struct uv_task_info * t);

void suspendSelf(struct uv_task_info * t);

#ifndef UVFR_STATE_MACHINE_IMPLIMENTATION

//EXTERNAL VARIABLES
extern enum uv_vehicle_state_t vehicle_state; //This is the one that ya'll are permitted to know
#else
//These shuold only be visible in the implimentation file

void _stateChangeDaemon(void * args);

void uvSVCTaskManager(void* args);

#endif



uv_task_id getSVCTaskID(char* tsk_name);
#endif /* INC_UVFR_STATE_ENGINE_H_ */


