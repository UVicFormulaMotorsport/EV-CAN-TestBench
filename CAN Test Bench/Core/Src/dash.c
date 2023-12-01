// This where the code to handle displaying data, errors and such to the dash will go


#include "dash.h"
#include "can.h"
#include "main.h"

void Update_RPM(int value){

	// Need ID
    TxHeader.StdId=0x80; // This is the CAN ID
    TxHeader.DLC=2; // Data Length Code


	// Need message
    int foo = value;

    TxData[0] = 0x00;
    TxData[1] = 0x10;


	  if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	  {
		/* Transmission request Error */
		Error_Handler();
	  }

}

// send/receive things
