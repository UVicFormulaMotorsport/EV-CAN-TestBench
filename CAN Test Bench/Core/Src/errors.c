#include "main.h"
#include "shutdown.h"
#include "errors.h"
#include "errorLUT.h"

extern error_struct _error_LUT[];

uint32_t error_bitfield[8] = {0,0,0,0,0,0,0,0};


void _throw_error(enum error_id id){
	error_struct error = _error_LUT[id];
	if(error.type_info & ERR_SHUTDOWN_MASK ){
		_SHUTDOWN;
	}else if(error.type_info & ERR_SUSPEND_MASK){
		if(error.resume_condition != NULL){
			_SUSPEND;
		} else {
			_SHUTDOWN;
		}
	}

#if _ERR_DISPLAY_ENABLE
	if((error.type_info & ERR_DISPLAY_MASK) && (error.msg != NULL)){
		//Display error message to dash

	}
#endif

	//logging
#if _ERR_LOGGING_ENABLE
	_log_error(e, NULL);
#endif


}

void _throw_error_int32(enum error_id id, int32_t arg)
{

}

void _throw_error_float(enum error_id id, float arg)
{

}

#if _ERR_LOGGING_ENABLE
	void _log_error(error_struct e, char* params){

	}
#endif


