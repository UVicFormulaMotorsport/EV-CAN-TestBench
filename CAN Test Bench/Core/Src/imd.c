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


// Need a function to parse the CAN message data received from the IMD
void IMD_Parse_Message(int DLC, int Data[8]){
	// The first step is to look at the first byte to figure out what we're looking at
	// Need to make sure that Data[0] really is the first byte
	switch (Data[0]){
		case isolation_state:
			// do something
		break;
		case isolation_resistances:
			// do something
		break;
		case isolation_capacitances:
			// do something
		break;
		case voltages_Vp_and_Vn:
			// do something
		break;
		case battery_voltage:
			// do something
		break;
		case Error_flags:
			// do something
		break;
		case safety_touch_energy:
			// do something
		break;
		case safety_touch_current:
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
// This is the function that will be called when a CAN message is received that has the isolation state data
void Check_Isolation_State(int Data[8]){
	// The first byte is the status bits. If any are non-zero -> error -> stop power to shutdown circuit
	if (Data[1] != 0){
		disable_shutdown_circuit();
	}

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
// Functions to send messages to the IMD (below)
// These send the message to request data
// The IMD will then send a message with the same code and the data. This is read in functions above
// --------------------------------------------------------------------------------------
void Request_Isolation_State(void){
	TxHeader.StdId = IMD_CAN_ID;
	TxHeader.DLC = 1;
	TxData[0] = isolation_state;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK){
		/* Transmission request Error */
		Error_Handler();
	  }
}

void Request_Isolation_Resistances(void){
	TxHeader.StdId = IMD_CAN_ID;
	TxHeader.DLC = 1;
	TxData[0] = isolation_resistances;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK){
		/* Transmission request Error */
		Error_Handler();
	  }
}















// --------------------------------------------------------------------------------------
// Function to remove power to shutdown circuit



