// Code to make human readable CAN messages for the device

// This file will define message data as human readable stuff


#include "main.h"

// Can ID: 0x710

enum pdu_messages_5A{
	enable_speaker_msg = 0x1C,
	disable_speaker_msg = 0x0C,
	enable_brake_light_msg = 0x1B,
	disable_brake_light_msg = 0x0B,
	enable_motor_controller_msg = 0x1E,
	disable_motor_controller_msg = 0x0E,
	enable_shutdown_circuit_msg = 0x1F,
	disable_shutdown_circuit_msg = 0x0F
};

enum pdu_messages_20A{
	enable_left_cooling_fan_msg = 0x33,
	disable_left_cooling_fan_msg = 0x23,
	enable_right_cooling_fan_msg = 0x34,
	disable_right_cooling_fan_msg = 0x24,
	enable_coolant_pump_msg = 0x31,
	disable_coolant_pump_msg = 0x21
};


// PDU functions (5 Amp)
void PDU_speaker_chirp();
void PDU_enable_brake_light();
void PDU_disable_brake_light();
void PDU_enable_motor_controller();
void PDU_disable_motor_controller();
void PDU_enable_shutdown_circuit();
void PDU_disable_shutdown_circuit();

// PDU functions (20 Amp)
void PDU_enable_cooling_fans();
void PDU_disable_cooling_fans();
void PDU_enable_coolant_pump();
void PDU_disable_coolant_pump();

