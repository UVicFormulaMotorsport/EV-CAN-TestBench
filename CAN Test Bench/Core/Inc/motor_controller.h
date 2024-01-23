// Code to make human readable CAN messages for the device

// This file will define message data as human readable stuff


#include "main.h"


// Need to add enums for human readable register addresses and such

enum motor_controller_speed_parameters{
	N_actual = 0x30, // actual motor speed rpm
	N_set = 0x31, // digital speed setpoint
	N_cmd = 0x32, // command speed after ramp
	N_error = 0x33, // speed error
	N_lim = 0x34, // speed limit
	accel = 0x35, // speed/torque accel ramp
};

enum motor_controller_current_parameters{
	todo1 = 0x69,
};

enum motor_controller_temperatures{
	todo = 69,

};

enum motor_controller_errors{
	todo69 = 69,
};

enum motor_controller_warnings{
	todo123 = 123,
};

enum motor_controller_io{
	todo6969 = 6969,
};

// The motor controller can send data repeatedly if so desired
enum motor_controller_repeating_time{
	none = 0,
	one_hundred_ms = 0x64,
};






// Function Declarations
void MC_Parse_Message(int DLC, int Data[]);
void MC_Request_Data(int RegID);
void MC_Send_Data(int RegID, int data, int size);








