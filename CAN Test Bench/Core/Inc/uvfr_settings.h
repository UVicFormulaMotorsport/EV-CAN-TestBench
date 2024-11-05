/*
 * uvfr_settings.h
 *
 *  Created on: Oct 10, 2024
 *      Author: byo10
 */

#ifndef INC_UVFR_SETTINGS_H_
#define INC_UVFR_SETTINGS_H_


#include "motor_controller.h"
#include "driving_loop.h"
#include "uvfr_utils.h"
#include "main.h"
#include "daq.h"
#include "bms.h"



#define ENABLE_FLASH_SETTINGS 0

typedef struct motor_controller_settings_t{
	int param;
}motor_controller_settings_t;


typedef struct uv_vehicle_settings{
	//struct motor_controller_settings mc_settings;
	bms_settings_t bms_settings;
	driving_loop_args driving_loop_settings;
	daq_loop_args daq_settings;
	struct motor_controller_settings_t motor_controller_settings;


}uv_vehicle_settings;


enum uv_status_t uvSettingsInit();

#ifndef SRC_UVFR_SETTINGS_C_
//extern includes

extern uv_vehicle_settings* current_vehicle_settings;

#endif


#endif /* INC_UVFR_SETTINGS_H_ */



