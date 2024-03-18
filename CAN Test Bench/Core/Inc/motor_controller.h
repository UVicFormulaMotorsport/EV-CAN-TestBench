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

enum motor_controller_motor_constants{
	nominal_motor_frequency = 0x05,
	nominal_motor_voltage = 0x06,
	power_factor = 0x0e,
	motor_max_current = 0x4D,
	motor_continuous_current = 0x4E,
	motor_pole_number = 0x4F,
	motor_kt_constant = 0x87, //low,
	motor_ke_constant = 0x87, //high, back emf
	rated_motor_speed = 0x59,
	motor_temperature_switch_off_point = 0xA3, // not sure if this is best in temp or not
	stator_leakage_inductance = 0xB1,
	nominal_magnitizing_current = 0xB2,
	motor_magnetising_inductance = 0xB3,
	rotor_resistance = 0xB4,
	minimum_magnetising_current = 0xB5,
	time_constant_rotor = 0xB6,
	leakage_inductance_ph_ph = 0xBB,
	stator_resistance_ph_ph = 0xBC,
	time_constant_stator = 0xBD,

};

enum motor_controller_temperatures{
	igbt_temperature = 0x4A,
	motor_temperature = 0x49,
	air_temperature = 0x4B,
	current_derate_temperature = 0x4C,
	temp_sensor_pt1 = 0x9C,
	temp_sensor_pt2 = 0x9D,
	temp_sensor_pt3 = 0x9E,
	temp_sensor_pt4 = 0x9F,
};

enum motor_controller_measurements{
	DC_bus_voltage = 0xEB,
};


enum motor_controller_status_information_errors_warnings{
	motor_controller_errors_warnings = 0x8F, // low bytes, warnings are high bytes
	// errors - the bit stuff is quite annoying. Look at can message excel sheet
	eprom_read_error = 1<<8, // This is bit 0
	hardware_fault = 1<<9, // bit 1
	rotate_field_enable_not_present_run = 1<<10, // bit 2 // with run active
	CAN_timeout_error = 1<<11, // bit 3
	feedback_signal_error = 1<<12, // bit 4
	mains_voltage_min_limit = 1<<13, // bit 5
	motor_temp_max_limit = 1<<14, // bit 6
	IGBT_temp_max_limit = 1<<15, // bit 7
	mains_voltage_max_limit = 1, // bit 8
	critical_AC_current = 1<<1, // bit 9
	race_away_detected = 1<<2, // bit 10
	ecode_timeout_error = 1<<3, // bit 11
	watchdog_reset = 1<<4, // bit 12
	AC_current_offset_fault = 1<<5, // bit 13
	internal_hardware_voltage_problem = 1<<6, // bit 14
	bleed_resistor_overload = 1<<7, // bit 15
	// The high and low bytes will be split up and we'll check errors and warnings separately
	// warnins - again, see can messages excel sheet / manuals
	parameter_conflict_detected = 1<<8, // bit 16
	special_CPU_fault = 1<<9, // bit 17
	rotate_field_enable_not_present_norun = 1<<10, // bit 18 // without run active
	auxiliary_voltage_min_limit = 1<<11, // bit 19
	feedback_signal_problem = 1<<12, // bit 20
	warning_5 = 1<<13, // bit 21 no idea what this warning is
	motor_temperature_warning = 1<<14, // bit 22, >87%
	IGBT_temperature_warning = 1<<15, // bit 23, >87%
	Vout_saturation_max_limit = 1, // bit 24
	warning_9 = 1<<1, // bit 25, no idea what this warning is either
	speed_actual_resolution_limit = 1<<2, // bit 26
	check_ecode_ID = 1<<3, // bit 27
	tripzone_glitch_detected = 1<<4, // bit 28
	ADC_sequencer_problem = 1<<5, // bit 29
	ADC_measurement_problem = 1<<6, // bit 30
	bleeder_resistor_warning = 1<<7, // bit 31
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
	clear_errors = 0x8E,
	firmware_version = 0x1B,
};



// Function Declarations
void MC_Parse_Message(int DLC, uint8_t Data[]);
void MC_Request_Data(uint8_t RegID);
void MC_Send_Data(uint8_t RegID, uint8_t data_to_send[], uint8_t size);
void MC_Torque_Control(int todo);
void MC_Speed_Control(int todo);
void MC_Check_Error_Warning(uint8_t Data[]);
void MC_Check_Serial_Number(uint8_t Data[]);
void MC_Check_Firmware(uint8_t Data[]);

void MC_Startup();





#endif
