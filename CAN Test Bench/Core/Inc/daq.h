/*
 * daq.h
 *
 *  Created on: Oct 15, 2024
 *      Author: byo10
 */

#ifndef INC_DAQ_H_
#define INC_DAQ_H_

#include "uvfr_utils.h"
#include "rb_tree.h"

#define _NUM_LOGGABLE_PARAMS



typedef enum{
	MOTOR_RPM,
	MOTOR_TEMP,
	MOTOR_CURRENT,
	MC_VOLTAGE,
	MC_CURRENT,
	MC_TEMP,
	MC_ERRORS,
	BMS_CURRENT,
	BMS_VOLTAGE,
	BMS_ERRORS,
	MAX_CELL_TEMP,
	MIN_CELL_TEMP,
	AVG_CELL_TEMP,
	ACC_POWER,
	ACC_POWER_LIMIT,
	APPS1_ADC_VAL,
	APPS2_ADC_VAL,
	BPS1_ADC_VAL,
	BPS2_ADC_VAL,
	ACCELERATOR_PEDAL_RATIO,
	BRAKE_PRESSURE_PA,
	POWER_DERATE_FACTOR,
	CURRENT_DRIVING_MODE,
	MAX_LOGGABLE_PARAMS
}loggable_params;




typedef struct daq_param_list_node{
	uint16_t param_idx;
	struct daq_param_list_node* next;
}daq_param_list_node;

/** @brief This struct holds info of what needs to be logged
 *
 */
typedef struct daq_datapoint{ //4 bytes, convenient, no?
	uint16_t can_id; /**< */
	uint8_t period; /**< */
	uint8_t type; /**< */
}daq_datapoint; /**< */

typedef struct daq_loop_args{
	uint8_t throttle_daq_to_preserve_performance; /**< */
	uint8_t minimum_daq_period; /**< */
	uint16_t padding; /**< */
	uint32_t padding2; /**< */
	daq_datapoint datapoints[MAX_LOGGABLE_PARAMS]; /**< */
}daq_loop_args;

typedef struct daq_child_task{
	struct rbnode treenode; //So this can just exist in a tree lol
	TaskHandle_t meta_task_handle;
	daq_param_list_node** param_list;
	uint32_t period;

}daq_child_task;

#ifndef _SRC_UVFR_DAQ
extern void* param_LUT[126];
#endif

enum uv_status_t initDaqTask(void * args);
void daqMasterTask(void* args);




#endif /* INC_DAQ_H_ */
