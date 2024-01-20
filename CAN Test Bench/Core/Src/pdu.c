// This where the code to handle switching channels with the PDU will go


#include "pdu.h"
#include "can.h"
#include "main.h"
#include "constants.h"


// PDU commands for 5A Circuit
void PDU_speaker_chirp(){
/* Chirp to indicate vehicle ready to drive*/
    TxHeader.StdId = PDU_CAN_ID_Tx; // This is the CAN ID
    TxHeader.DLC = 1; // Data Length Code

    TxData[0] = enable_speaker_msg;

    if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
    {
		 /* Transmission request Error */
		 Error_Handler();
    }
    // the delay has to be between 1 to 3 seconds by rules
	HAL_Delay(2000);
	TxData[0] = disable_speaker_msg;
	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}


void PDU_enable_brake_light(){

	TxHeader.StdId=PDU_CAN_ID_Tx; // PDU CAN ID
    TxHeader.DLC=1; // Data Length Code

    TxData[0] = enable_brake_light_msg;

	 if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	 {
		 /* Transmission request Error */
		 Error_Handler();
	 }
}

void PDU_disable_brake_light(){

	TxHeader.StdId=PDU_CAN_ID_Tx; // PDU CAN ID
    TxHeader.DLC=1; // Data Length Code

    TxData[0] = disable_brake_light_msg;

	 if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	 {
		 /* Transmission request Error */
		 Error_Handler();
	 }
}

void PDU_enable_motor_controller(){
	TxHeader.StdId=PDU_CAN_ID_Tx; // PDU CAN ID
	TxHeader.DLC=1; // Data Length Code

	TxData[0] = enable_motor_controller_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}
void PDU_disable_motor_controller(){
	TxHeader.StdId=PDU_CAN_ID_Tx; // PDU CAN ID
	TxHeader.DLC=1; // Data Length Code

	TxData[0] = disable_motor_controller_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}

void PDU_enable_shutdown_circuit(){
	TxHeader.StdId=PDU_CAN_ID_Tx; // PDU CAN ID
	TxHeader.DLC=1; // Data Length Code

	TxData[0] = enable_shutdown_circuit_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}

void PDU_disable_shutdown_circuit(){
	TxHeader.StdId=PDU_CAN_ID_Tx; // PDU CAN ID
	TxHeader.DLC=1; // Data Length Code

	TxData[0] = disable_shutdown_circuit_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}


// PDU commands for 20A Circuit
void PDU_enable_cooling_fans(){
	TxHeader.StdId=PDU_CAN_ID_Tx; // PDU CAN ID
	TxHeader.DLC=1; // Data Length Code

	TxData[0] = enable_left_cooling_fan_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}

	TxData[0] = enable_right_cooling_fan_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}

void PDU_disable_cooling_fans(){
	TxHeader.StdId=PDU_CAN_ID_Tx; // PDU CAN ID
	TxHeader.DLC=1; // Data Length Code

	TxData[0] = disable_left_cooling_fan_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}

	TxData[0] = disable_right_cooling_fan_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}


void PDU_enable_coolant_pump(){
	TxHeader.StdId=PDU_CAN_ID_Tx; // PDU CAN ID
	TxHeader.DLC=1; // Data Length Code

	TxData[0] = enable_coolant_pump_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}
void PDU_disable_coolant_pump(){
	TxHeader.StdId=PDU_CAN_ID_Tx; // PDU CAN ID
	TxHeader.DLC=1; // Data Length Code

	TxData[0] = disable_coolant_pump_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}
