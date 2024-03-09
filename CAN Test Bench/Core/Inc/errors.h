/*
 * errors.h
 *
 *  Created on: Mar 2, 2024
 *      Author: byo10
 */

#ifndef INC_ERRORS_H_
#define INC_ERRORS_H_
#include "main.h"

//How this thing works, for byron's future memory
#define ERR_SHUTDOWN_MASK 0b00000001
#define ERR_SUSPEND_MASK 0b00000010
//#define ERR_
#define ERR_DISPLAY_MASK 0b00001000

typedef enum error_id{
	shutdown_cct_tripped = 1, //shutdown circuit errors
	bms_shutdown_tripped,
	bspd_shutdown_tripped,
	imd_shutdown_cct_tripped,
	coolant_temp_too_high,
	coolant_pressure_too_high,
}error_id;





typedef struct error_struct{
	char* msg;
	uint8_t type_info;
	void* err_handlers; //function pointer to a dedicated error handler
	uint8_t* resume_condition; //function pointer for some function that decides whether to resume after suspended
}error_struct;

typedef struct error_log_entry_struct{
	uint32_t timestamp;
	error_id id;
	char msg[];
}error_log_entry_struct;





void emergency_shutdown();



#endif /* INC_ERRORS_H_ */


