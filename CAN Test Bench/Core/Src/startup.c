/*
 * startup.c
 *
 *  Created on: Mar 6, 2024
 *      Author: Peter
 *      Blame me for bad code
 */
#include "imd.h"

// startup happens when GLV master switch gets turned on
// As soon as there is GLV power, the control board will be performing these checks
// The IMD will get power from GLV, so it will be turning on, and then outputting high to shutdown
// We want to poll the IMD to make sure it is active and sends out the right serial number and such

// Then, we want to check the BMS operation

// Most of this is for motor controller startup
// We need to monitor the ready to drive state for tractive system active stuff
// There is an enable for the motor controller that should clear the errors
// However, we can also send the CAN message to clear the error
// Then, we want to check if there are any persistent errors
// If there are any errors, then we want to abort startup

// motor controller startup is going to check that the device is turned on properly
// we want to read the serial number, but can check other things like unit type, voltages...

// The other devices on the CAN bus are the dash and PDU. We should display something on the dash
// like device status or something, and if the PDU fails then the car won't drive so oh well

void Check_Devices(){
	// TODO
	// - Need a timer for timeout of devices
	// - Need a way to indicate whether a device has responded properly
	// - Need to incorporate digital inputs of shutdown circuit to make sure devices are outputting
	// - Need to worry about when tractive system has voltage and such
	IMD_Startup();

	// BMS_Startup();
	// Motor_controller_Startup();


}

