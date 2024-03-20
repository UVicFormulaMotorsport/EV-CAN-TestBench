#include "main.h"
#include "shutdown.h"
#include "errors.h"
#include "errorLUT.h"

extern error_struct* _error_LUT[];//Lookup table for what to do with certain errors.

//stores info about what errors are currently active. Better than scraping the logs.
uint16_t error_bitfield[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void _throw_error(enum error_id id){
	error_struct* error = _error_LUT[id];
	if(error->type_info & ERR_SHUTDOWN_MASK ){ //Shutdown takes precedence over suspend
		_SHUTDOWN;
	}else if(error->type_info & ERR_SUSPEND_MASK){ //suspend takes precedence over simple warnings, or log events

		if(error->resume_condition != NULL){ //if there is no way to recover from the suspended state, might as well shut down.

			if(!(error_bitfield[id>>4] & 0x01<<(id & 0x000F))){//check if error is already active, no need for duplicates
				_Suspend(error->resume_condition);
			}
		} else {
			_Shutdown();
		}

	}

	if(error->type_info & ERR_LIMP_MASK){
		//TODO: Implement Limp Mode
	}


	error_bitfield[id>>4] |= 0x01<<(id & 0x000F); //final 4 bits of id represent the individual bit, remaining bits represent index

	if(error->err_handler != NULL){ //ya'll better have defined the error handler, bitch
		error->err_handler(); //executes code specified by user if available. Will do it's thing
	}

//update the bitfield



#if _ERR_DISPLAY_ENABLE
	if((error->type_info & ERR_DISPLAY_MASK) && (error.msg != NULL)){
		//Display error message to dash

	}
#endif

	//logging
#if _ERR_LOGGING_ENABLE
	_log_error(error, NULL);
#endif


}

//may god help us all
#if _ERR_ARGS_ENABLE
//different types of args... bruh...

void _throw_error_int32(enum error_id id, int32_t arg)
{

}

void _throw_error_float(enum error_id id, float arg)
{

}
#endif

#if _ERR_LOGGING_ENABLE
	void _log_error(error_struct e, char* params){
		//step 1:
	}
#endif

void error_loop(){
	//Do some final shutdown things here
	while(1){ //there is no escape from the error loop. Most unfortunate. Get fucked. Only a hard reboot can save you now.

	}
}

