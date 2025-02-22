/** @file driving_loop.c
 *  @brief File containing the meat and potatoes driving loop thread, and all supporting functions
 *
 */



#include "main.h"
#include "uvfr_utils.h"
#include "can.h"
#include "motor_controller.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "driving_loop.h"

//External Variables:
extern uint16_t adc1_APPS1; //These are the locations for the sensor inputs for APPS and BPS
extern uint16_t adc1_APPS2;
extern uint16_t adc1_BPS1;
extern uint16_t adc1_BPS2;



 // RACHAN

// Constants
// #define RPM_MIN # // set appropriate min RPM
// #define RPM_MAX # // set appropriate max RPM
// #define T_MAX 200      // Maximum Torque (Example Value) (Nm)
// #define FILTER_K 0.1   // Filtering factor (Adjust based on response needs)


// FUNCTION PROTOTYPES
//static float volt_to_percent(float voltage);
//static float calcualteThrottlePercentage(uint16_t apps1, unit16_t app2);
//static float mapThrottleToTorque(float throttle_percent);
//static float applyTorqueFilter(float T_req, float T_prev);
//static void sendTorqueToMotorController(float T_filtered);




enum uv_status_t initDrivingLoop(void *argument){
	uv_task_info* dl_task = uvCreateTask(); // allocate memory for the task

	if(dl_task == NULL){
		//Oh dear lawd if allocation fails return error
		return UV_ERROR;
	}


	//DO NOT TOUCH ANY OF THE FIELDS WE HAVENT ALREADY MENTIONED HERE. FOR THE LOVE OF GOD.

	//dl_task->task_name = malloc(16*sizeof(char));
	//if(dl_task->task_name == NULL){
		//return UV_ERROR; //failed to malloc the name of the thing
	//}
	dl_task->task_name = "Driving_Loop"; // assign name


	dl_task->task_function = StartDrivingLoop; // defining the function the task will run
	dl_task->task_priority = osPriorityHigh; // assigns a high priority in FreeRTOS

	dl_task->stack_size = 256; // memory allocation for task execution

	dl_task->active_states = UV_DRIVING; // Specifies when the task should be active
	dl_task->suspension_states = 0x00;
	dl_task->deletion_states = UV_READY | PROGRAMMING | UV_LAUNCH_CONTROL | UV_ERROR_STATE; // defines conditions where the task should be deleted.

	dl_task->task_period = 100;//runs every 100ms or 0.1 seconds

	//

	dl_task->task_args = NULL; //TODO: Add actual settings dipshit

	return UV_OK; //

}


/**@brief  Function implementing the ledTask thread.
 * @param  argument: Not used for now. Will have configuration settings later.
 * @retval None
 *
 * This function is made to be the meat and potatoes of the entire vehicle.
 *
 */
void StartDrivingLoop(void * argument){
	//Initialize driving loop now

	/** The first thing we do here is create some local variables here, to cache whatever variables need cached.
	 *	We will be caching variables that are used very frequently in every single loop iteration, and are not
	 */


	//extracting task arguments, and gets parameters like min/max allowed values for the APPS and BPS
	uv_task_info* params = (uv_task_info*) argument;
	/*
	uint16_t min_apps_value;
	uint16_t max_apps_value;

	uint16_t min_apps_offset;
	uint16_t max_apps_offset;
	*/
	enum DL_internal_state dl_status = Plausible;

	/** This line extracts the specific driving loop parameters as specified in the
	 * vehicle settings
	 @code*/
	driving_loop_args* dl_params = (driving_loop_args*) params->task_args;
	/**@endcode*/

	/*
	min_apps_value = dl_params->min_apps_value;
	max_apps_value = dl_params->max_apps_value;

	min_apps_offset = dl_params->min_apps_offset; //minimum APPS offset
	max_apps_offset = dl_params->max_apps_offset;
	*/

	/**These here lines set the delay. This task executes exactly at the period specified, regardless of how long the task
		 * execution actually takes
		 * rachan: ensures the function runs exactly 100ms, regardless of execution time.
		 *
		 @code*/
	TickType_t tick_period = pdMS_TO_TICKS(params->task_period); //Convert ms of period to the RTOS ticks
	TickType_t last_time = xTaskGetTickCount();
	/**@endcode */


	// Rachan ATTEMPT:
	// Local variables to store computed values
	//float throttle_percent = 0.0f; // Throttle percentage
	//float T_req = 0.0f; // requested torque
	//float T_filtered = 0.0f; //Filtered Torque (smoothed output)

	// Define task timing (executes every 100ms);
	//TickType_t tick_period = pdMS_TO_TICKS(100);
	//TickType_t last_time = xTaskGetTickCount();

	for(;;){ // enters infinite loop for the driving loop task


		if(params->cmd_data == UV_KILL_CMD){ // to perform task control (suspend/kill)
			killSelf(params);
		}else if(params->cmd_data == UV_SUSPEND_CMD){
			suspendSelf(params);
		}
		vTaskDelayUntil( &last_time, tick_period); //Me and the boys on our way to wait for a set period

		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14); //Blink and LED (for debugging)
		//Read stuff from the addresses

		//Copy the values over into new local variables, in order to avoid messing up the APPS
		uint16_t apps1_value = adc1_APPS1; // reading sensor values
		uint16_t apps2_value = adc1_APPS2;

		uint16_t bps1_value = adc1_BPS1;
		uint16_t bps2_value = adc1_BPS2;


		// SAFETY CHECKS

		// APPS1/APPS2 CHECK (Throttle Position Sensors)

		// RACHAN SAFETY CHECK APPLICATION:
		// if (adc1_APPS1 < RPM_MIN || adc1_APPS1 > RPM_MAX) continue;
        // if (adc1_APPS2 < RPM_MIN || adc1_APPS2 > RPM_MAX) continue;

		// i would put these into funciton calls : rachan


		//Perform input validation and ensures values are within the expected range
		if(apps1_value < dl_params->min_apps_value){
			//APPS1 is probably unplugged. Act accordingly
		}else if(apps1_value > dl_params->max_apps_value){
			//indicative of APPS1 short (APPS1 short circuit detected)
			//goto DL_end;
		}

		if(apps2_value < dl_params->max_apps_value){
			//APPS2 is probably unplugged. Act accordingly.
		}else if(apps2_value > dl_params->max_apps_value){
			//indicative of APPS2 short
		}

		uint16_t apps_diff = apps1_value - apps2_value;

		if((apps_diff > dl_params->max_apps_offset)||(apps_diff < dl_params->min_apps_offset)){
			//APPS1 and APPS2 are no longer in sync with each other
		}



		// BPS1/BPS2 Check (Brake Pressure Sensors)
		// Ensures brake sensors are not faulty.

		if(bps1_value < dl_params->min_BPS_value){
			// Possible BPS1 disconnection
		}else if(bps1_value > dl_params->max_BPS_value){
			// Possible BPS1 short circuit
		}



		//Repeat these safety checks for BPS 2.
		if(bps2_value < dl_params->min_BPS_value){

		}else if(bps2_value > dl_params->max_BPS_value){

		}



		//Brake Plausibility Check

		/** Brake Plausibility Check
		 *
		 * The way that this works is that if the brake pressure is greater than some threshold,
		 * and the accelerator pedal position is also greater than some threshold, the thing will
		 * register that a brake implausibility has occurred. This is not very cash money.
		 *
		 * If this happens, we want to set the torque/speed output to zero. This will only reset itself once
		 * the brakes are set to less than a certain threshold. Honestly evil.
		 *
		 * Rachan: IF both accelerator and brake are pressed at the same time, the system registers a fault and cuts motor power.
		 */

		if((bps1_value > dl_params->bps_plausibility_check_threshold) && (apps1_value > dl_params->apps_plausibility_check_threshold)){
			//This means that the pedal and brake are checked simultaneously
			dl_status = Implausible; // set status to error
		}


		//possible return to plausibility
		if((dl_status == Implausible) && (1)){
			dl_status = Plausible;
		}

		//Compute torque/velocity or whatever
		if(dl_status == Plausible){
			// Compute motor output
		}


		//Command the motor controller to do the thing
		if(1){//if(1) reminds me that there should be a

		}

		//DL_end:
		//Set loose the ADC, and have it DMA the result into the variables


		// Rachan:
		// Step 1: Convert APPS sensor values to Throttle Percentage
		// throttle_percent = calculateThrottlePercentage(adc1_APPS1, adc2_APPS2);


		// Step 2: Convert Throttle % to Torque Request
		// T_req = mapThrottleToTorque(throttle_percent);

		// Step 3: Apply Filtering to Smooth Torque Output
		// T_filtered = applyTorqueFilter(T_req, T_filtered);

		// Step 4: Send Filtered Torque to Motor Controller
		// sendTorqueToMotorController(T_filtered);

		// Debugging: Print throttle and torque values (if needed)
		// printf("Throttle: %.2f%%, Torque: %.2f Nm\n", throttle_percent, T_filtered);


		// Wait for the next loop cycle (ensures a consistent 100ms execution interval)
		//vTaskDelayUntil(&last_time, tick_period);


		//Wait until next D.L. occurance
		//osDelay(DEFAULT_PERIOD);
	}

}




/**
 * @brief Converts APPS sensor readings into a throttle percentage
 *
 * @param apps1: Raw voltage value from APPS1 sensor
 * @param apps2: Raw voltage value from APPS2 sensor
 *
 *
 * @return Throttle percentage (0% to 100%).

static float calculateThrottlePercentage(uint16_t apps1, uint16_t apps2) {
    // Ensure both sensor values are within the valid range
    if (apps1 < RPM_MIN || apps1 > RPM_MAX || apps2 < RPM_MIN || apps2 > RPM_MAX) return 0.0f;

    // Compute throttle percentage using linear interpolation
    float throttle_percent = ((float)(apps1 - RPM_MIN) / (RPM_MAX - RPM_MIN)) * 100.0f;

    // SAFETY CHECK: Verify APPS1 and APPS2 values are within 10% of each other
    float apps_diff = ((float)apps1 - (float)apps2) / (float)apps1;
    if (apps_diff > 0.1f) {
        // Sensors are out of sync, return 0% to prevent errors
        return 0.0f;
    }

    return throttle_percent;
}
*/




/**
 * @brief  Maps Throttle Percentage to Torque Request.
 *
 * @param  throttle_percent: Throttle percentage (0% - 100%).
 *
 * @return Requested torque in Nm.

static float mapThrottleToTorque(float throttle_percent) {
    return (throttle_percent / 100.0f) * T_MAX;
}
*/





/**
 * @brief  Applies filtering to smooth torque transitions.
 *
 * @param  T_req: Requested torque before filtering.
 * @param  T_prev: Previous filtered torque value.
 *
 * @return Smoothed torque value.

static float applyTorqueFilter(float T_req, float T_prev) {
    // Filtering formula: T_filtered = T_prev + (T_req - T_prev) * k
    return T_prev + (T_req - T_prev) * FILTER_K;
}
*/






/**
 * @brief  Sends the filtered torque value to the motor controller.
 *
 * @param  T_filtered: Final torque value after filtering.

static void sendTorqueToMotorController(float T_filtered) {
    // Placeholder function to send torque command to motor controller
    motor_controller_set_torque(T_filtered);
}
*/





