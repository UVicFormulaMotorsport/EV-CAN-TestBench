// Code to make human readable CAN messages for the device

// This file will define message data as human readable stuff


#include "main.h"

#ifndef __MOTOR_CONTROLLER_H__
#define __MOTOR_CONTROLLER_H__

// Need to add enums for human readable register addresses and such

enum motor_controller_speed_parameters{
	N_actual = 0x30, // actual motor speed rpm
	N_set = 0x31, // digital speed setpoint
	N_cmd = 0x32, // command speed after ramp
	N_error = 0x33, // speed error
};

enum motor_controller_current_parameters{
	todo1 = 0x69,
};

enum motor_controller_temperatures{
	igbt_temperature = 0x4A,

};

enum motor_controller_measurements{
	todo_mains_voltage = 0x696969,
};


enum motor_controller_status_information_errors_warnings{
	status_information_errors = 0x8F, // low byte, warnings are high byte
	//Eprom_read_error = bit 0 - not usre of endianness
};

enum motor_controller_warnings{
	todo123 = 123,
};

enum motor_controller_io{
	todo6969 = 6969,
};

enum motor_controller_PI_values{
	// There are two different ramps, one for speed and moment,
	// Each one corresponds to a high or low byte
	accelerate_ramp = 0x35,
	dismantling_ramp = 0xED,
	recuperation_ramp = 0xC7,
	proportional_gain = 0x1C,
	integral_time_constant = 0x1D,
	integral_memory_max = 0x2B,
	proportional_gain_2 = 0xC9, // This is for when the current is more than max setpoint
	current_feed_forward = 0xCB,
	ramp_set_current = 0x25,

};

// The motor controller can send data repeatedly if so desired
enum motor_controller_repeating_time{
	none = 0,
	one_hundred_ms = 0x64,
};

enum motor_controller_limp_mode{
	N_lim = 0x34,
	N_lim_plus = 0x3F,
	N_lim_minus = 0x3E,
};

enum motor_controller_startup{
	clear_errors = 0x8E //
};



// Function Declarations
void MC_Parse_Message(int DLC, uint8_t Data[]);
void MC_Request_Data(int RegID);
void MC_Send_Data(int RegID, uint8_t data, int size);







#endif
