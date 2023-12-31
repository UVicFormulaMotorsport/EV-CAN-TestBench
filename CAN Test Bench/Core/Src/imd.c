// This where the code to handle IMD errors and such will go


#include "imd.h"

// We need to include can.h because we will send CAN messages through the functions in that file
// When a CAN message comes it will throw an interrupt can.c deals with the incoming message
// the function in can.c gets the ID and sends the data to  the functions here
#include "can.h"
#include "main.h"
#include "constants.h"

// We need to include pdu.h for the shutdown circuit
#include "pdu.h"


int IMD_CAN_ID = 0x23; // This is the ID for MCU sending msg to IMD
// MCU will receive messages with id 0x24 from IMD

uint8_t IMD_status_bits = 0;

// If there is a hardware error, that one bit will be a 1 in the status bits -> read error flags
// error flags will return the status bits which will have a 1 in HE bit -> infinite loop
uint8_t IMD_error_flags_requested = 0;


// Need a function to parse the CAN message data received from the IMD
void IMD_Parse_Message(int DLC, int Data[]){
	// The first step is to look at the first byte to figure out what we're looking at
	// Need to make sure that Data[0] really is the first byte
	switch (Data[0]){
		case isolation_state:
			Check_Status_Bits(Data[1]);
			Check_Isolation_State(Data);
		break;
		case isolation_resistances:
			Check_Status_Bits(Data[1]);
		break;
		case isolation_capacitances:
			Check_Status_Bits(Data[1]);
			// do something
		break;
		case voltages_Vp_and_Vn:
			Check_Status_Bits(Data[1]);
			// do something
		break;
		case battery_voltage:
			Check_Status_Bits(Data[1]);
			// do something
		break;
		case Error_flags:
			Check_Status_Bits(Data[1]);
			Check_Error_Flags(Data);
			// do something
		break;
		case safety_touch_energy:
			Check_Status_Bits(Data[1]);
			// do something
		break;
		case safety_touch_current:
			Check_Status_Bits(Data[1]);
			// do something
		break;
		case Max_battery_working_voltage:
			// do something
		break;
		case Vn_hi_res:
			// do something
		break;
		case Vp_hi_res:
			// do something
		break;
		case Vexc_hi_res:
			// do something
		break;
		case Vb_hi_res:
			// do something
		break;
		case Vpwr_hi_res:
			// do something
		break;
		case Temperature:
			// do something
		break;
		default:
			Error_Handler();
		break;
	}

}

// --------------------------------------------------------------------------------------
// Functions to check status
// AFAIK the IMD will not send a CAN msg when the status changes - we need to constantly poll it
// --------------------------------------------------------------------------------------

// A lot of the messages will include status bits
// Check for faults
// Then check what the error is to display it for driver
void Check_Status_Bits(int Data){
	// The touch energy bit will be 1 when connected to batteries
	// High uncertainty isn't also something we really care about
	// No idea about excitation pulse
	int mask = 0b100011111;

	if ((Data & mask) != 0){
		disable_shutdown_circuit();

		if ((Data & Isolation_status_bit0) || (Data & Isolation_status_bit1)){
			// Isolation fault BAD
			// Want to display fault to dash
		}

		// This function is only passed the first byte of info so we can't read the error flags
		// If we pass the entire data array in then we will read the wrong data
		// Need to explicitly request error flags and then read it
		// Use a bool to check if we have already requested error flags otherwise it will request repeatedly
		if (Data & Hardware_Error){
			// TODO
			// display to dash
			if (!IMD_error_flags_requested){
				IMD_Request_Status(Error_flags);
				IMD_error_flags_requested = 1;
			}
		}
	// TODO
	// rest of status bits
	}
	// Could check other faults we don't really care about

	// If we made it here then there is no error so exit to check rest of message
}

// We need to look at the 2nd and 3rd bytes in the array for the error flags
void Check_Error_Flags(int Data[]){
	// Need to check the bits to see what caused the hardware error
	// Want to display message to dash for safety reasons
	uint16_t IMD_Error_Flags = (Data[1] << 8) | Data[2];

	if (IMD_Error_Flags & Err_Vx1){
		// print to dash I guess
	}
	if (IMD_Error_Flags & Err_Vx2){
		// print to dash I guess
	}
	if (IMD_Error_Flags & Err_CH){
		// print to dash I guess
	}
}


// This is the function that will be called when a CAN message is received that has the isolation state data
void Check_Isolation_State(int Data[]){

	// Then check the rest of the message
	int isolation = (Data[2] << 8) | Data[3];

	if (isolation < 500){
		disable_shutdown_circuit();
	}

	// Can check tolerances and such but are far less important
	// debugging
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
}




// --------------------------------------------------------------------------------------
// Function to send messages to the IMD (below)
// These send the message to request data. The specific status requested is passed as arg
// The IMD will then send a message with the same code and the data. This is read in functions above
// --------------------------------------------------------------------------------------
void IMD_Request_Status(int Status){
	TxHeader.StdId = IMD_CAN_ID;
	TxHeader.DLC = 1;
	TxData[0] = Status;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK){
		/* Transmission request Error */
		Error_Handler();
	  }
}




