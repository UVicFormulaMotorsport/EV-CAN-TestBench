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
#define DEFAULT_PERIOD 50

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

/** @brief this struct is designed to hold information about each drivingmode's map params
 *
 */
typedef struct drivingModeParams{
	uint8_t control_map_fn;//function used to do the mapping. this must match the function of the one on the
	uint8_t params[48];
}drivingModeParams;


/** @brief This is where the driving mode and the deivingModeParams are at
 *
 */
typedef struct drivingMode{
	char dm_name[16];
	uint8_t is_speed_control;
	uint8_t control_map_fn; //is a map_mode
	drivingModeParams* map_fn_params;
}drivingMode;




/**@struct drivingLoopArgs
 * @brief Arguments for the driving loop. The reason this is a struct
 * passed in as an argument, rather than a bunch of global variables or constants is to allow the code to
 * take settings from flash memory, therefore allowing the team to meet it's goal of having an actual
 * GUI to change vehicle settings.
 *
 */
typedef struct driving_loop_args{
	uint8_t period; // how often does the driving loop execute
	uint8_t current_driving_mode; //what is the driving mode?
	uint16_t min_apps_offset; //minimum APPS offset
	uint16_t max_apps_offset; // maximum APPS offset, between this and the minimum offset we get a range of permissible values
	uint16_t min_apps_value; //for detecting disconnects and short circuits
	uint16_t max_apps_value; //for detecting disconnects and short circuits
	uint16_t min_BPS_value; //are the brakes valid?
	uint16_t max_BPS_value; //are the brakes valid?

	uint16_t apps_top;
	uint16_t apps_bottom;

	uint16_t apps_plausibility_check_threshold;
	uint16_t bps_plausibility_check_threshold;

	uint16_t bps_implausibility_recovery_threshold;
	uint16_t apps_implausibility_recovery_threshold;

	uint8_t num_driving_modes;
	drivingMode dmodes[8]; //This has the different driving modes. We have up to 8.

}driving_loop_args;

enum uv_status_t initDrivingLoop(void *argument);

void StartDrivingLoop(void *argument);

#endif /* INC_DRIVING_LOOP_H_ */
