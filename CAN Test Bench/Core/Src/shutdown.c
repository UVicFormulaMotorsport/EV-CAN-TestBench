
#include "can.h"
#include "main.h"
#include "constants.h"

// We need to include pdu.h for the shutdown circuit
#include "pdu.h"
extern enum vehicle_state_t vehicle_state;

void _Voluntary_Power_Down(){ //this for if the driver voluntarily wants to stop
	PDU_disable_motor_controller();
	vehicle_state = ready;
	HAL_delay(200);
	PDU_disable_shutdown_circuit;

}

void _Shutdown(){//emergency hard stop
	PDU_disable_motor_controller(); //disable motor controller
	vehicle_state = error;
	PDU_disable_shutdown_circuit();
}

void _Suspend(uint8_t (*cond)()){
	if(vehicle_state == error){ //error gets precedence over suspend
		return;
	}

	/*
	 * Step 1: Disable the motor controller enable.
	 * Step 2: Set Vehicle State
	 * Step 3: add return condition to suspend subroutine
	 */

	PDU_disable_motor_controller();
	vehicle_state == suspended;
	_add_condition(cond);
}

// RFE (motor controller digital input) needs to be wired to shutdown circuit
void Trigger_Shutdown_Circuit(){
	PDU_disable_shutdown_circuit();
}


void Disable_Motor_Controller(){
	PDU_disable_motor_controller();
}


void Limp_Mode(){
	// turn on limp mode for motor controller
	// set max current output in BMS potentially - YOU CANNOT DO THAT
}
