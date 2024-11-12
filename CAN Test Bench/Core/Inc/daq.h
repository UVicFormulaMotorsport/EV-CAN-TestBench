/*
 * daq.h
 *
 *  Created on: Oct 15, 2024
 *      Author: byo10
 */

#ifndef INC_DAQ_H_
#define INC_DAQ_H_

#include "uvfr_utils.h"

#define _NUM_LOGGABLE_PARAMS

typedef enum  {
	UV_UINT8,
	UV_INT8,
	UV_UINT16,
	UV_INT16,
	UV_UINT32,
	UV_INT32,
	UV_FLOAT,
	UV_DOUBLE

}data_type;

typedef uint16_t daq_log_entry;

typedef union daq_type{
	data_type type;
	uint16_t length;

}daq_type;


/** @brief This struct holds info of what needs to be logged
 *
 */
typedef struct daq_datapoint{
	uint8_t is_active;
	uint8_t period;
	daq_log_entry daq_log1;//What data
	daq_type daq_type1; //what type
	daq_log_entry daq_log2; //empty if the first one is a strung
	daq_type daq_type2; //length of log1 is string

}daq_datapoint;

typedef struct daq_loop_args{

}daq_loop_args;

enum uv_status_t initDaqTask(void * args);
void daqMasterTask(void* args);


#endif /* INC_DAQ_H_ */
