// This where the code to handle motor controller errors and such will go


#include "motor_controller.h"

// We need to include can.h because we will send CAN messages through the functions in that file
// When a CAN message comes it will throw an interrupt can.c deals with the incoming message
// the function in can.c gets the ID and sends the data to the functions here
#include "can.h"
#include "main.h"
#include "constants.h"

// We need to include pdu.h for the shutdown circuit
#include "pdu.h"


void MC_Parse_Message(int DLC, uint8_t Data[]){
	// The first step is to look at the first byte to figure out what we're looking at
	// TODO Need to make sure that Data[0] really is the first byte
	switch (Data[0]){
		// important checks
		case motor_controller_errors_warnings:
			MC_Check_Error_Warning(Data);
		break;

		// .
		// .
		// .

		default: // This is a code that is not recognized (bad)
			Error_Handler();
		break;
	}

}

// this function is used to get data from the motor controller
void MC_Request_Data(int RegID){

	TxHeader.StdId = MC_CAN_ID_Tx;
	TxHeader.DLC = 3;
	TxData[0] = 0x3D;
	TxData[1] = RegID;
	TxData[2] = 0;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK){
		/* Transmission request Error */
		Error_Handler();
	}

}



// We can either send 2 or 4 bytes of data
void MC_Send_Data(int RegID, uint8_t data, int size){

	TxHeader.StdId = MC_CAN_ID_Tx;
	TxHeader.DLC = size;
	TxData[0] = RegID;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK){
		/* Transmission request Error */
		Error_Handler();
	}

}


void MC_Check_Error_Warning(uint8_t Data[]){

	// The motor controller will send a message with 6 bytes of data
	// The first byte is the register ID, which in this case is 0x8F
	// The second and third byte are low bytes which correspond to errors
	// The order of bits in the low bytes is
	// 7 6 5 4 3 2 1 0  15 14 13 12 11 10 9 8

	// The fourth and fifth bytes are high bytes and correspond to warnings
	// The order of bits in the high bytes is
	// 23 22 21 20 19 18 17 16   31 30 29 28 27 26 25 24

	// We need to and the bytes with the right bitmasks and check for errors


	// Split four bytes into 2 high and 2 low bytes

	uint16_t MC_errors = (Data[1] << 8) | Data[2];
	uint16_t MC_warnings = (Data[3] << 8) | Data[4];

	// All the error flags should be zero
	if (MC_errors){
		// Compare errors to errors bitmask to determine error
		if (MC_errors & eprom_read_error){
			// bad
		}
		if (MC_errors & hardware_fault){
			// bad
		}
		if (MC_errors & rotate_field_enable_not_present_run){
			// bad
		}
		if (MC_errors & CAN_timeout_error){
			// bad
		}
		if (MC_errors & feedback_signal_error){
			// bad
		}
		if (MC_errors & mains_voltage_min_limit){
			// bad
		}
		if (MC_errors & motor_temp_max_limit){
			// bad
		}
		if (MC_errors & IGBT_temp_max_limit){
			// bad
		}
		if (MC_errors & mains_voltage_max_limit){
			// bad
		}
		if (MC_errors & critical_AC_current){
			// bad
		}
		if (MC_errors & race_away_detected){
			// bad
		}
		if (MC_errors & ecode_timeout_error){
			// bad
		}
		if (MC_errors & watchdog_reset){
			// bad
		}
		if (MC_errors & AC_current_offset_fault){
			// bad
		}
		if (MC_errors & internal_hardware_voltage_problem){
			// bad
		}
		if (MC_errors & bleed_resistor_overload){
			// bad
		}
	}

	if (MC_warnings){
		// compare warnings to warnings bitmask
		if (MC_warnings & parameter_conflict_detected){
			// not great
		}
		if (MC_warnings & special_CPU_fault){
			// not great
		}
		if (MC_warnings & rotate_field_enable_not_present_norun){
			// not great
		}
		if (MC_warnings & auxiliary_voltage_min_limit){
			// not great
		}
		if (MC_warnings & feedback_signal_problem){
			// not great
		}
		if (MC_warnings & warning_5){
			// not great
		}
		if (MC_warnings & motor_temperature_warning){
			// not great
		}
		if (MC_warnings & IGBT_temperature_warning){
			// not great
		}
		if (MC_warnings & Vout_saturation_max_limit){
			// not great
		}
		if (MC_warnings & warning_9){
			// not great
		}
		if (MC_warnings & speed_actual_resolution_limit){
			// not great
		}
		if (MC_warnings & check_ecode_ID ){
			// not great
		}
		if (MC_warnings & tripzone_glitch_detected){
			// not great
		}
		if (MC_warnings & ADC_sequencer_problem){
			// not great
		}
		if (MC_warnings & ADC_measurement_problem){
			// not great
		}
		if (MC_warnings & bleeder_resistor_warning){
			// not great
		}
	}
}



void MC_Startup(){
	// MC_Send_Data(...)
}



