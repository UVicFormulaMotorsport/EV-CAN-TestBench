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
uv_vehicle_settings* current_vehicle_settings = NULL;

extern struct uv_os_settings default_os_settings;

#if ENABLE_FLASH_SETTINGS
enum uv_status_t getSettingsFromFlash(){
	//if there exists settings in flash somewhere, then get them. Otherwise we use the defaults
	return UV_ABORTED;
}


bool uvAreCustomSettingsActive(){
	return false;
}

uv_status saveSettings(){
	return UV_ABORTED;
}


#endif


/** @brief Function that allocates the neccessary space for all the vehicle settings, and
 * handles sets all of the settings structs to defaults
 *
 */
void setupDefaultSettings(){
	if(current_vehicle_settings != NULL){
		uvFree(current_vehicle_settings);
	}
	//real trap shit
	current_vehicle_settings = uvMalloc(sizeof(uv_vehicle_settings));
	current_vehicle_settings->os_settings = &default_os_settings;
}

void nukeSettings(uv_vehicle_settings** settings_to_delete){


	uvFree(*settings_to_delete);
	*settings_to_delete = NULL;


}

/** @brief this function does one thing, and one thing only, it checks if we have custom settings, then it attempts to get them.
 * If it fails, then we revert to factory defaults.
 *
 */
enum uv_status_t uvSettingsInit(){
#if ENABLE_FLASH_SETTINGS
	uv_status setting_success = get_settings_from_flash();
	if(setting_success == UV_ABORTED){
		setting_success = setupDefaultSettings();
	} else if(setting_success == UV_ERROR){
		return UV_ERROR;
	}

	if(setting_success == UV_OK){
		return UV_OK;
	}else{
		//handle the resulting error, maybe soft reboot ngl
	}

#else
	setupDefaultSettings();
#endif


	return UV_OK;

}

void uvSettingsProgrammerTask(void* args){
	uv_task_info* params = (uv_task_info*) args;
	void* new_tmp_settings = NULL;
	for(;;){

	}
}




