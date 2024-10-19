/*
 * uvfr_state_engine.h
 *
 *  Created on: Oct 15, 2024
 *      Author: byo10
 */

#ifndef INC_UVFR_STATE_ENGINE_H_
#define INC_UVFR_STATE_ENGINE_H_

#ifndef UVFR_STATE_MACHINE_IMPLIMENTATION

//EXTERNAL VARIABLES
extern enum uv_vehicle_state_t;

#endif

enum uv_status_t uvRegisterTask();

enum uv_status_t uvInitStateEngine();

enum uv_status_t uvStartStateMachine();



#endif /* INC_UVFR_STATE_ENGINE_H_ */


