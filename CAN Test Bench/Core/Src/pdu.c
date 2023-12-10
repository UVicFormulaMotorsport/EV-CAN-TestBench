// This where the code to handle switching channels with the PDU will go


#include "pdu.h"
#include "can.h"
#include "main.h"


// PDU commands for 5A Circuit
void speaker_chirp(){
/* Chirp to indicate vehicle ready to drive*/
    TxHeader.StdId=0x710; // This is the CAN ID
    TxHeader.DLC=1; // Data Length Code

    TxData[0] = enable_speaker_msg;

    if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
    {
		 /* Transmission request Error */
		 Error_Handler();
    }
    //TODO: is this the correct delay?
	HAL_Delay(1000);
	TxData[0] = disable_speaker_msg;
	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}


void enable_brake_light(){

	TxHeader.StdId=0x710; // PDU CAN ID
    TxHeader.DLC=1; // Data Length Code

    TxData[0] = enable_brake_light_msg;

	 if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	 {
		 /* Transmission request Error */
		 Error_Handler();
	 }
}

void disable_brake_light(){

	TxHeader.StdId=0x710; // PDU CAN ID
    TxHeader.DLC=1; // Data Length Code

    TxData[0] = disable_brake_light_msg;

	 if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	 {
		 /* Transmission request Error */
		 Error_Handler();
	 }
}

void enable_motor_controller(){
	TxHeader.StdId=0x710; // PDU CAN ID
	TxHeader.DLC=1; // Data Length Code

	TxData[0] = enable_motor_controller_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}
void disable_motor_controller(){
	TxHeader.StdId=0x710; // PDU CAN ID
	TxHeader.DLC=1; // Data Length Code

	TxData[0] = disable_motor_controller_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}

void enable_shutdown_circuit(){
	TxHeader.StdId=0x710; // PDU CAN ID
	TxHeader.DLC=1; // Data Length Code

	TxData[0] = enable_shutdown_circuit_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}

void disable_shutdown_circuit(){
	TxHeader.StdId=0x710; // PDU CAN ID
	TxHeader.DLC=1; // Data Length Code

	TxData[0] = disable_shutdown_circuit_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}


// PDU commands for 20A Circuit
void enable_cooling_fans(){
	TxHeader.StdId=0x710; // PDU CAN ID
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

void disable_cooling_fans(){
	TxHeader.StdId=0x710; // PDU CAN ID
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


void enable_coolant_pump(){
	TxHeader.StdId=0x710; // PDU CAN ID
	TxHeader.DLC=1; // Data Length Code

	TxData[0] = enable_coolant_pump_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}
void disable_coolant_pump(){
	TxHeader.StdId=0x710; // PDU CAN ID
	TxHeader.DLC=1; // Data Length Code

	TxData[0] = disable_coolant_pump_msg;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}
