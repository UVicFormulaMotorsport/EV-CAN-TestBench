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

// Need function to send message to motor controller


// Need functions to check specific values







