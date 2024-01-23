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


void MC_Parse_Message(int DLC, int Data[]){
	// The first step is to look at the first byte to figure out what we're looking at
	// TODO Need to make sure that Data[0] really is the first byte
	switch (Data[0]){
		// important checks
		case 1:
			// TODO
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
void MC_Send_Data(int RegID, int data, int size){

	TxHeader.StdId = MC_CAN_ID_Tx;
	TxHeader.DLC = size;
	TxData[0] = RegID;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK){
		/* Transmission request Error */
		Error_Handler();
	}

}





// Need to worry about control algorithms in a bit



