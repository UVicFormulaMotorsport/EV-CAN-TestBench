/*
 * uvfr_settings.h
 *
 *  Created on: Oct 10, 2024
 *      Author: byo10
 */

#ifndef INC_UVFR_SETTINGS_H_
#define INC_UVFR_SETTINGS_H_

#include "driving_loop.h"
#include "uvfr_utils.h"
#include "main.h"
#include "daq.h"


#define ENABLE_FLASH_SETTINGS 0


typedef struct uvVehicleSettings{
	drivingLoopArgs* drivingLoopSettings;
	daqLoopArgs* daqSettings;

}uvVehicleSettings;


enum uv_status_t uvSettingsInit();

#endif /* INC_UVFR_SETTINGS_H_ */



#ifndef SRC_UVFR_SETTINGS_C_
//extern includes

extern uvVehicleSettings* uvCurrentSettings;

#endif
