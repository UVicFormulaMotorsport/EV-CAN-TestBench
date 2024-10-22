/*
 * uvfr_settings.c
 *
 *  Created on: Oct 10, 2024
 *      Author: byo10
 */
#define SRC_UVFR_SETTINGS_C_

#include "uvfr_utils.h"
#include "main.h"
#include "stdlib.h"


//global variables for all to enjoy
uvVehicleSettings* uvCurrentSettings = NULL;


/** @brief this function does one thing, and one thing only, it checks if we have custom settings, then it attempts to get them.
 * If it fails, then we revert to factory defaults.
 *
 */
enum uv_status_t uvSettingsInit(){
#if ENABLE_FLASH_SETTINGS
	if(get_settings_from_flash()!= UV_OK){
		setup_default_settings();
	}
#else
	setup_default_settings();
#endif


	return UV_OK;

}

enum uv_status_t get_settings_from_flash(){
	//if there exists settings in flash somewhere, then get them. Otherwise we use the defaults
	return UV_OK;
}

void setup_default_settings(){
	//real trap shit
	uvCurrentSettings = (uvVehicleSettings*) malloc(sizeof(uvVehicleSettings*));
}

void nuke_settings(uvVehicleSettings** settings_to_delete){

	//*uvVehicleSettings = NULL;


}
