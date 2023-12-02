// This where the code to handle displaying data, errors and such to the dash will go


#include "dash.h"
#include "can.h"
#include "main.h"


// If we pass it a negative number then a garbage number will show up
void Update_RPM(int16_t value){

	// Need ID
    TxHeader.StdId=0x80; // This is the CAN ID
    TxHeader.DLC=2; // Data Length Code


	// Need message
    TxData[0] = (value >> 8)& 0xFF;
    TxData[1] = value & 0xFF;



	  if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	  {
		/* Transmission request Error */
		Error_Handler();
	  }

}

// send/receive things
