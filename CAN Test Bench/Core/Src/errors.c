#include "main.h"
#include "shutdown.h"
#include "errors.h"
#include "errorLUT.h"

extern error_struct _error_LUT[];


void _throw_error(enum error_id id){
	error_struct error = _error_LUT[id];
	if(error.type_info & ERR_SHUTDOWN_MASK ){

	}

}
void _throw_error_can(enum error_id id, char* can_msg){

}


