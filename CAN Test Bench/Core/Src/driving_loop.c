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



/**@brief  Function implementing the ledTask thread.
 * @param  argument: Not used for now. Will have configuration settings later.
 * @retval None
 *
 * This function is made to be the meat and potatoes of the entire vehicle.
 *
 */
void StartDrivingLoop(void const * argument){
	//Initialize driving loop now

	/** The first thing we do here is create some local variables here, to cache whatever variables need cached.
	 *	We will be caching variables that are used very frequently in every single loop iteration, and are not
	 */
	uint16_t min_apps_value;
	uint16_t max_apps_value;

	uint16_t min_apps_offset;
	uint16_t max_apps_offset;

	enum DL_internal_state dl_status = Plausible;


	drivingLoopArgs* dl_params = NULL;

	drivingLoopArgs** arg = (drivingLoopArgs**) argument;

	if(argument == NULL){
		//argument = &default_dl_params;
	}else{
		dl_params = (drivingLoopArgs*) *arg;
	}

	dl_params = (drivingLoopArgs*) *arg;

	min_apps_value = dl_params->min_apps_value;
	max_apps_value = dl_params->max_apps_value;

	min_apps_offset = dl_params->min_apps_offset; //minimum APPS offset
	max_apps_offset = dl_params->max_apps_offset;

	for(;;){
		//Read stuff from the addresses

		//Copy the values over into new local variables, in order to avoid messing up the APPS
		uint16_t apps1_value = adc1_APPS1;
		uint16_t apps2_value = adc1_APPS2;

		uint16_t bps1_value = adc1_BPS1;
		uint16_t bps2_value = adc1_BPS2;


		//Perform input validation
		if(apps1_value < dl_params->min_apps_value){
			//APPS1 is probably unplugged. Act accordingly
		}else if(apps1_value > dl_params->max_apps_value){
			//indicative of APPS1 short
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



		if(bps1_value < dl_params->min_BPS_value){

		}else if(bps1_value > dl_params->max_BPS_value){

		}


		//Repeat these safety checks for BPS 2.
		if(bps2_value < dl_params->min_BPS_value){

		}else if(bps2_value > dl_params->max_BPS_value){

		}

		//Brake Plausibility Check

		/** @heading Brake Plausibility Check
		 *
		 * The way that this works is that if the brake pressure is greater than some threshold,
		 * and the accelerator pedal position is also greater than some threshold, the thing will
		 * register that a brake implausibility has occurred. This is not very cash money.
		 *
		 * If this happens, we want to set the torque/speed output to zero. This will only reset itself once
		 * the brakes are set to less than a certain threshold. Honestly evil.
		 */

		if((bps1_value > dl_params->bps_plausibility_check_threshold) && (apps1_value > dl_params->apps_plausibility_check_threshold)){
			//This means that the pedal and brake are checked simultaneously
			dl_status = Implausible;
		}


		//possible return to plausibility
		if((dl_status == Implausible) && (1)){
			dl_status = Plausible;
		}

		//Compute torque/velocity or whatever
		if(dl_status == Plausible){

		}


		//Command the motor controller to do the thing
		if(1){

		}

		DL_end:
		//Set loose the ADC, and have it DMA the result into the variables




		//Wait until next D.L. occurance
		osDelay(DEFAULT_PERIOD);
	}

}


