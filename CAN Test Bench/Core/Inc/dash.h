// Code to make human readable CAN messages for the device

// This file will define message data as human readable stuff

// The dash does not have a unique CAN ID
// Each function on the dash has a unique CAN ID

#include "main.h"

#ifndef __DASH_H__
#define __DASH_H__

// CAN IDs correspond to specific values on the dash
enum dash_can_ids{
	Dash_RPM = 0x80,
	Dash_Battery_Temperature = 0x82,
	Dash_Motor_Temperature = 0x88,
	Dash_State_of_Charge = 0x87,
};

// Battery Temperature 0x82

// Motor Temperature 0x87

// Inverter Temperature TODO

// RPM 0x80

// State of Charge 0x87

// Need to define these

void Update_RPM(int16_t value);
void Update_Batt_Temp(uint8_t value);
void Update_State_Of_Charge(uint8_t value);

#endif

