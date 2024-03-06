// Code to make human readable CAN messages for the device

// This file will define message data as human readable stuff

#include "main.h"


// CAN ID is currently extended

// Needs to be changed to standard ID

// Faults
// The first byte returned from a status request message will have 8 status bits and the data as the rest
enum imd_status_bits{
	// I have just left this as plain binary. Could also be declared by bitshifting but I think this is easier
	//	    					12345678
	Isolation_status_bit0	= 0b00000001, // IS0
	Isolation_status_bit1	= 0b00000010, // IS1
	Low_Battery_Voltage		= 0b00000100, // LV 15V threshold (1 if below 15V)
	High_Battery_Voltage	= 0b00001000, // HV (1 if higher than Max_battery_working_voltage)
	Exc_off 				= 0b00010000, // EO (Excitation pulse 0 for operating)
	High_Uncertainty 		= 0b00100000, // HU (1 for greater than 5%)
	Touch_energy_fault 		= 0b01000000, // EF (1 for energy exceeds 0.2J)
	Hardware_Error			= 0b10000000, // HE (1 for error)
};


// The MCU needs to send a message to the IMD requesting info
// These are requests that will return status bits (defined above) & the value requested
enum imd_status_requests{
	// The electrical isolation is in bytes 2 & 3
	isolation_state = 0xE0,

	// Rp is the resistance from the positive of the battery to chassis
	// Rn is the resistance from the negative of the battery to chassis
	isolation_resistances = 0xE1,

	// Cp is the capacitance from the positive of the battery to chassis
	// Cn is the capacitance from the negative of the battery to chassis
	// Bytes 2 & 3 are Cp, bytes 5 & 6 are Cn, in nF
	isolation_capacitances = 0xE2,

	// High voltage battery voltages to chassis
	voltages_Vp_and_Vn = 0xE3,

	// GLV battery voltage
	battery_voltage = 0xE4,

	// Error flags are a series of bits described below
	Error_flags = 0xE5,

	// The IMD monitors charge stored in the system
	safety_touch_energy = 0xE6,

	// Also monitors if it is safe to touch
	safety_touch_current = 0xE7,

	// This is just a parameter we can set
	Max_battery_working_voltage = 0xF0,

	// We can read the temperature of the board
	Temperature = 0x80,
};

// If one of the error flags is set, then the harware error bit will go to 1
enum imd_error_flags{
	// Bits 0-6 are reserved
	Err_temp = 0x0080, // HT high temperature (0 if temperature is below 105C)
	Err_clock = 0x0100, // CE  (0 if clock is good)
	Err_Watchdog = 0x0200, // WD Watchdog (0 if watchdog is good)
	Err_Vpwr = 0x0400, // V_PWR power supply (0 if power supply voltage is good)
	Err_Vexi = 0x0800, // V_EXI Excitation voltage (0 if excitation voltage is good)
	Err_VxR = 0x1000, // VxR (0 if both Vx1 and Vx2 are good)
	Err_CH = 0x2000, // CH chassis connections (0 if both chassis connections are good)
	Err_Vx1 = 0x4000, // Vx1 connection (0 if SIM101 connection to Hv+ is good)
	Err_Vx2 = 0x8000, // Vx2 connection (0 if SIM101 connection to Hv- is good)
};

// Probably not useful and won't be used for a bit but may be useful in the future
enum imd_manufacturer_requests{
	// The numbers get broken up over multiple bytes
	Part_name_0 = 0x01,
	Part_name_1 = 0x02,
	Part_name_2 = 0x03,
	Part_name_3 = 0x04,
	Version_0 = 0x05,
	Version_1 = 0x06,
	Version_2 = 0x07,
	Serial_number_0 = 0x08,
	Serial_number_1 = 0x09,
	Serial_number_2 = 0x0A,
	Serial_number_3 = 0x0B,
	Uptime_counter = 0x0C,
};

enum imd_high_resolution_measurements{
	Vn_hi_res = 0x60,
	Vp_hi_res = 0x61,
	Vexc_hi_res = 0x62,
	Vb_hi_res = 0x63,
	Vpwr_hi_res = 0x65,
};

// ---------------------------------------------------------------
// Function declarations

// This will parse the data received from CAN message
void IMD_Parse_Message(int DLC, uint8_t Data[]);


// Functions to check states are okay
void Check_Status_Bits(uint8_t Data);
void Check_Error_Flags(uint8_t Data[]);

// Functions to check values are okay
void Check_Isolation_State(uint8_t Data[]);
void Check_Isolation_Resistances(uint8_t Data[]);
void Check_Isolation_Capacitances(uint8_t Data[]);
void Check_Voltages_Vp_and_Vn(uint8_t Data[]);
void Check_Battery_Voltage(uint8_t Data[]);
void Check_Safety_Touch_Energy(uint8_t Data[]);
void Check_Safety_Touch_Current(uint8_t Data[]);
void Check_Temperature(uint8_t Data[]);

// Functions to check on startup
void Check_Max_Battery_Working_Voltage(uint8_t Data[]);
void Check_Part_Name(uint8_t Data[]);
void Check_Version(uint8_t Data[]);
void Check_Serial_Number(uint8_t Data[]);
void Check_Uptime(uint8_t Data[]);

// High resolution measurements


// Function to request data from the IMD
void IMD_Request_Status(int Status);






