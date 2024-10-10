/*
 * uvfr_utils.h
 *
 *  Created on: Oct 7, 2024
 *      Author: byo10
 */

#ifndef INC_UVFR_UTILS_H_
#define INC_UVFR_UTILS_H_

#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "can.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"

#include "FreeRTOS.h"
#include "task.h"


/* Some syntactic sugar for ya'll to use in your code
 * Defaults to 16 bit unsigned int
 */
#define _BV(x) _BV_16(x)
#define _BV_8(x) ((uint8_t)(0x01U >> x))
#define _BV_16(x) ((uint16_t)(0x01U >> x))
#define _BV_32(x) ((uint32_t)(0x01U >> x))

#define endianSwap(x) endianSwap16(x)
#define endianSwap8(x) x //if someone calls this, they are mentally retarded, but here ya go I guess
#define endianSwap16(x) (((x & 0x00FF)<<8) | ((x & 0xFF00)>>8))
#define endianSwap32(x) (((x & 0x000000FF)<<16)|((x & 0x0000FF00)<<8)|((x & 0x00FF0000)>>8)|((x & 0xFF000000)>>16))


/*	This is meant to be a return type from functions that indicates what is actually going on
 * Use this for things like initializing characters or
 *
 */
enum uv_status_t{
	UV_OK,
	UV_WARNING,
	UV_ERROR,
};

/* Type made to represent the state of the vehicle, and the location in the state machine
 *
 */
enum uv_vehicle_state_t{
	init,
	ready,
	programming,
	driving,
	suspended,
	launch_control,
	error
};

//not really sure how I want this implemented yet. Variable number of driving modes?
enum uv_driving_mode_t{
	normal,
	accel,
	econ,
	limp
};


//this works exactly the same way that malloc does
void * uvMalloc(size_t size);

//this works exactly the same way that free does
uv_status_t uvFree(void * mem);



#endif /* INC_UVFR_UTILS_H_ */
