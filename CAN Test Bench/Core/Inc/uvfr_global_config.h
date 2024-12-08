/*
 * uvfr_global_config.h
 *
 *  Created on: Nov 27, 2024
 *      Author: byo10
 */

#ifndef INC_UVFR_GLOBAL_CONFIG_H_
#define INC_UVFR_GLOBAL_CONFIG_H_

//VCU Port Configuration
#define UV_EXTERNAL_PDU 1
#define STM32_F407 1



//memory management configuration
#ifndef UV_MALLOC_LIMIT
#define UV_MALLOC_LIMIT ((size_t)1024)
#endif

#ifndef USE_OS_MEM_MGMT
#define USE_OS_MEM_MGMT 0
#endif


#endif /* INC_UVFR_GLOBAL_CONFIG_H_ */
