// Code to make human readable CAN messages for the device

// This file will define message data as human readable stuff


#include "main.h"

// Can ID: 0x710

enum pdu_messages_5A{
	enable_speaker = 0x1C,
	disable_speaker = 0x0C,
	enable_brake_light = 0x1B,
	disable_brake_light = 0x0B,
	enable_motor_controller = 0x1E,
	disable_motor_controller = 0x0E,
	enable_shutedown_circuit = 0x1F,
	disable_shutedown_circuit = 0x0F
};

enum pdu_messages_20A{
	enable_cooling_fan1 = 0x33,
	disable_cooling_fan1 = 023,
	enable_cooling_fan2 = 0x34,
	disable_cooling_fan2 = 0x24,
	enable_coolant_pump = 0x31,
	disable_coolant_pump = 0x21

};
