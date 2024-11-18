/*
 * temp_monitoring.h
 *
 *  Created on: Oct 31, 2024
 *      Author: byo10
 */

#ifndef INC_TEMP_MONITORING_H_
#define INC_TEMP_MONITORING_H_

#include "uvfr_utils.h"

uv_status initTempMonitor(void* args);

void tempMonitorTask(void* args);

#endif /* INC_TEMP_MONITORING_H_ */
