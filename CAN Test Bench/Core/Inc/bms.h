// Code to make human readable CAN messages for the device



#ifndef _BMS_H_
#define _BMS_H_

#include "main.h"
#include "uvfr_utils.h"

#define DEFAULT_BMS_CAN_TIMEOUT ((uv_timespan_ms)200)

typedef struct bms_settings_t{
	uint32_t mc_CAN_timeout;
}bms_settings_t;

void BMS_Init(void* args);

#endif



