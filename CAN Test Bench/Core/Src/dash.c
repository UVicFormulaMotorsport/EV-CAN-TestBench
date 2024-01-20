// This where the code to handle displaying data, errors and such to the dash will go

#include "dash.h"
#include "can.h"
#include "main.h"


// If we pass it a negative number then a garbage number will show up
void Update_RPM(int16_t value){

	// Need ID
    TxHeader.StdId = Dash_RPM; // This is the CAN ID
    TxHeader.DLC = 2; // Data Length Code


	// Need message
    TxData[0] = (value >> 8)& 0xFF;
    TxData[1] = value & 0xFF;

	  if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	  {
		/* Transmission request Error */
		Error_Handler();
	  }

}

// If we pass it a negative number then a garbage number will show up
void Update_Batt_Temp(uint8_t value){

	// Need ID
    TxHeader.StdId = Dash_Battery_Temperature; // This is the CAN ID
    TxHeader.DLC = 1; // Data Length Code


	// Need message
    TxData[0] = value;

	  if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	  {
		/* Transmission request Error */
		Error_Handler();
	  }

}


void Update_State_Of_Charge(uint8_t value){

	// Need ID
    TxHeader.StdId = Dash_State_of_Charge; // This is the CAN ID
    TxHeader.DLC = 1; // Data Length Code


	// Need message
    TxData[0] = value;

	  if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	  {
		/* Transmission request Error */
		Error_Handler();
	  }

}




