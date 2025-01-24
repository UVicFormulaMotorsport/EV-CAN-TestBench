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

#ifndef SIZEOF_SETTINGS_BLOCK
#define SIZEOF_SETTINGS_BLOCK sizeof(uv_vehicle_settings);
#endif

//AAA
typedef struct veh_gen_info{

}veh_gen_info;

typedef struct uv_vehicle_settings{

	struct uv_os_settings* os_settings;
	struct motor_controller_settings* mc_settings;

	driving_loop_args* driving_loop_settings;

	void* imd_settings;
	bms_settings_t* bms_settings;

	daq_loop_args* daq_settings;

	void* pdu_settings;
	//struct motor_controller_settings motor_controller_settings;

	uint16_t is_default; /**< Bitfield containing info on whether each settings instance is factory default. 0 default, 1 altered*/


}uv_vehicle_settings;


void nukeSettings(uv_vehicle_settings** settings_to_delete);

enum uv_status_t uvSettingsInit();

#ifndef SRC_UVFR_SETTINGS_C_
//extern includes

extern uv_vehicle_settings* current_vehicle_settings;

#endif


#endif /* INC_UVFR_SETTINGS_H_ */



