
#include "can.h"
#include "main.h"
#include "constants.h"

// We need to include pdu.h for the shutdown circuit
#include "pdu.h"



void Trigger_Shutdown_Circuit(){
	PDU_disable_shutdown_circuit();
}


void Disable_Motor_Controller(){
	PDU_disable_motor_controller();
}



