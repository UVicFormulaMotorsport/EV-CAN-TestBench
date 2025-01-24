/*
 * driving_loop.h
 *
 *  Created on: Oct 14, 2024
 *      Author: byo10
 */

#ifndef INC_DRIVING_LOOP_H_
#define INC_DRIVING_LOOP_H_

#include "motor_controller.h"
#include "uvfr_utils.h"

typedef uint16_t MC_Torque;
typedef uint16_t MC_RPM;
typedef uint16_t MC_POWER;

/**
 * @brief DL_PERIOD is meant to represent how often the driving loop executes, in ms
 *
 * This is a define since I would eventually like this to be configurable via a global variable,
 * or possible be dynamic in the future.
 *
 * Just replace the number with the name of the variable, and you're all set.
 */
//#define DEFAULT_PERIOD 50

/**@brief enum meant to represent the different types of pedal map
 *
 * This enum is meant to represent different functions that map the torque to speed.
 *
 */
enum map_mode{
	linear_speed_map,
	s_curve_speed_map,
	exp_speed_map,
	linear_torque_map,
	s_curve_torque_map,
	exp_torque_map
};

enum DL_internal_state{
	Plausible = 0x01,
	Implausible = 0x02,
	Erroneous = 0x04
};

typedef struct linear_torque_map_args{ //y = mx + b bitch
	int32_t offset; /**< */
	float slope; /**< */
}linear_torque_map_args;

/** @brief struct to hold parameters used in an exponential torque map
 *
 */
typedef struct exp_torque_map_args{
	int32_t offset;
	float gamma;


}exp_torque_map_args;

/** @brief struct for s-curve parameters for torque
 *
 */
typedef struct s_curve_torque_map_args{
	int32_t a;
	int32_t b;
	int32_t c[16];
}s_curve_torque_map_args;

/** @brief this struct is designed to hold information about each drivingmode's map params
 *
 */
typedef union drivingModeParams{
	struct linear_torque_map_args; /**< */
	struct exp_torque_map_args; /**< */
	struct s_curve_torque_map_args; /**< */
}drivingModeParams;


/** @brief This is where the driving mode and the drivingModeParams are at
 *
 */
typedef struct drivingMode{
	char dm_name[16]; /**< Name of mode, 15 chars + /0*/
	uint32_t max_acc_pwr;
	uint32_t max_motor_torque;
	uint32_t max_current;


	uint16_t flags;

	drivingModeParams map_fn_params; /**< */
	uint8_t control_map_fn; /**< */
}drivingMode;




/**@struct drivingLoopArgs
 * @brief Arguments for the driving loop. The reason this is a struct
 * passed in as an argument, rather than a bunch of global variables or constants is to allow the code to
 * take settings from flash memory, therefore allowing the team to meet it's goal of having an actual
 * GUI to change vehicle settings.
 *
 */
typedef struct driving_loop_args{
	uint32_t absolute_max_acc_pwr; /**< Maximum possible accum power*/
	uint32_t absolute_max_motor_torque; /**< Max power output*/
	uint32_t absolute_max_accum_current;/**< Max current (ADC reading)*/
	uint32_t max_accum_current_5s; /**< Current maximum for 10s */


	uint16_t absolute_max_motor_rpm;    /**< Max limit of RPM*/
	uint16_t regen_rpm_cutoff;			/**< No regen below this rpm */


	//uint8_t current_driving_mode; //what is the driving mode?
	uint16_t min_apps_offset; /**<minimum APPS offset */
	uint16_t max_apps_offset; /**< maximum APPS offset */
	uint16_t min_apps_value; /**< for detecting disconnects and short circuits*/
	uint16_t max_apps_value; /**< for detecting disconnects and short circuits*/
	uint16_t min_BPS_value; /**< are the brakes valid?*/
	uint16_t max_BPS_value; /**< are the brakes valid?*/

	uint16_t apps_top; /**< Max APPS input value, representing 100% throttle*/
	uint16_t apps_bottom; /**< Min APPS input value, representing 0% throttle*/

	uint16_t apps_plausibility_check_threshold; /**< Threshold for accelerator position with  */
	uint16_t bps_plausibility_check_threshold; /**< Brake pressure threshold for APPS */

	uint16_t bps_implausibility_recovery_threshold; /**< Threshold for accellerator pedal position to recover fron APPS check*/
	uint16_t apps_implausibility_recovery_threshold; /**< Threshold for brake position */

	uint8_t num_driving_modes;/**< How many modes are actually populated */
	uint8_t period; /**< how often does the driving loop execute */
	uint8_t accum_regen_soc_threshold; /**< Vehicle will not regen if above this SOC*/


	drivingMode dmodes[8]; /**< These are various driving modes */

}driving_loop_args;

enum uv_status_t initDrivingLoop(void *argument);

void StartDrivingLoop(void *argument);

#endif /* INC_DRIVING_LOOP_H_ */
