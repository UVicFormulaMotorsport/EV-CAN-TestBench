
#include "can.h"
#include "main.h"
#include "constants.h"

// We need to include pdu.h for the shutdown circuit
#include "pdu.h"


void _Shutdown(){

}

void _Suspend(){

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
	// set max current output in BMS potentially
}
