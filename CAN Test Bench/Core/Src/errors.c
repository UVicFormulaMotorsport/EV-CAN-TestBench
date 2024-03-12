#include "main.h"
#include "shutdown.h"
#include "errors.h"
#include "errorLUT.h"

extern error_struct* _error_LUT[];

uint32_t error_bitfield[8] = {0,0,0,0,0,0,0,0};


void _throw_error(enum error_id id){
	error_struct* error = _error_LUT[id];
	if(error->type_info & ERR_SHUTDOWN_MASK ){ //Shutdown takes precedence over suspend
		_SHUTDOWN;
	}else if(error->type_info & ERR_SUSPEND_MASK){ //suspend takes precedence over limit
		if(error->resume_condition != NULL){
			_SUSPEND;
		} else {
			_SHUTDOWN;
		}
	}

if(error->err_handler != NULL){ //ya'll better have defined the error handler, bitch
	error->err_handler();
}

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

	}
#endif


