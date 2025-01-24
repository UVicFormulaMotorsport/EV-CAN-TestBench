/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
#include "constants.h"
#include "imd.h"
#include "motor_controller.h"
#include "dash.h"
#include "bms.h"
#include "pdu.h"
#include "uvfr_utils.h"
#include "main.h"


#ifndef HAL_CAN_ERROR_INVALID_CALLBACK
#define HAL_CAN_ERROR_INVALID_CALLBACK (0x00400000U)
#endif

static QueueHandle_t Tx_msg_queue = NULL;

void handleCANbusError(const CAN_HandleTypeDef* hcan, const uint32_t err_to_ignore){
	if(hcan == NULL){
		uvPanic("null can handle",0);
		return;
	}




	uint32_t errcode = HAL_CAN_GetError(hcan);

	if(errcode & err_to_ignore){
		return;
	}

	if(errcode == HAL_CAN_ERROR_NONE){
		return;
	}else if(errcode & HAL_CAN_ERROR_EWG){ //protocol error
		uvPanic("CAN protocol error",0);
	}else if(errcode & HAL_CAN_ERROR_EPV){ //passive
		uvPanic("CAN passive error",0);
	}else if(errcode & HAL_CAN_ERROR_BOF){ //Bus-off error
		uvPanic("CAN Bus off error",0);
	}else if(errcode & HAL_CAN_ERROR_STF){ //Stuff error
		uvPanic("CAN Stuff error",0);
	}else if(errcode & HAL_CAN_ERROR_FOR){ //Form error
		uvPanic("CAN Form error",0);
	}else if(errcode & HAL_CAN_ERROR_ACK){ //Acknowledgement error
		uvPanic("CAN Ack error",0);
	}else if(errcode & HAL_CAN_ERROR_BR){ //bit recessive
		uvPanic("CAN Recessive Bit error",0);
	}else if(errcode & HAL_CAN_ERROR_BD){ //bit dominant
		uvPanic("CAN Dominant Bit error",0);
	}else if(errcode & HAL_CAN_ERROR_CRC){//cyclic redundancy check
		uvPanic("CAN CRC Failed",0);
	}else if(errcode & HAL_CAN_ERROR_RX_FOV0){//overrun rx fifo0
		uvPanic("CAN RX FIFO0",0);
	}else if(errcode & HAL_CAN_ERROR_RX_FOV1){//overrun rx fifo1
		uvPanic("CAN RX FIFO1",0);
	}else if(errcode & HAL_CAN_ERROR_TX_ALST0){//tx mailbox 0 arbitration lost
		uvPanic("CAN0 arbitration",0);
	}else if(errcode & HAL_CAN_ERROR_TX_TERR0){//tx mailbox 0 transmit error
		uvPanic("CAN0 transmit err",0);
	}else if(errcode & HAL_CAN_ERROR_TX_ALST1){//tx mailbox 1 arbitration lost error
		uvPanic("CAN1 arbitration",0);
	}else if(errcode & HAL_CAN_ERROR_TX_TERR1){//tx mailbox 1 transmit error
		uvPanic("CAN1 transmit err",0);
	}else if(errcode & HAL_CAN_ERROR_TX_ALST2){//tx mailbox 2 arbitration lost error
		uvPanic("CAN2 arbitration",0);
	}else if(errcode & HAL_CAN_ERROR_TX_TERR2){//tx mailbox 2 transmit error
		uvPanic("CAN2 transmit err",0);
	}else if(errcode & HAL_CAN_ERROR_TIMEOUT){//timeout
		uvPanic("CAN timeout",0);
	}else if(errcode & HAL_CAN_ERROR_NOT_INITIALIZED){//is not initialized
		uvPanic("CAN not init",0);
	}else if(errcode & HAL_CAN_ERROR_NOT_READY){//Not ready
		uvPanic("CAN_not_ready",0);
	}else if(errcode & HAL_CAN_ERROR_NOT_STARTED){//not started
		uvPanic("HAL_CAN_NOT_STARTED",0);
	}else if(errcode & HAL_CAN_ERROR_PARAM){//Param
		uvPanic("CAN Param",0);
	}else if(errcode & HAL_CAN_ERROR_INVALID_CALLBACK){//invalid callback
		uvPanic("Invalid_callback",0);
	}else if(errcode & HAL_CAN_ERROR_INTERNAL){//internal error
		uvPanic("HAL_internal",0);
	}else{
		//no clue how we got here

	}


}


/* USER CODE END 0 */

CAN_HandleTypeDef hcan2;

/* CAN2 init function */
void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 8;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_12TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_8TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = ENABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */

  	// Define the CAN Filter
    CAN_FilterTypeDef FilterConfig;

    // Set the data length and ID to a temporary value
     TxHeader.DLC= 1; // Data Length Code
     TxHeader.StdId= 0x244; // This is the CAN ID

     TxHeader.IDE=CAN_ID_STD; //set identifier to standard
     TxHeader.RTR=CAN_RTR_DATA;
     TxHeader.ExtId = 0x01;
     TxHeader.TransmitGlobalTime = DISABLE;

    //filter one (stack light blink)
     FilterConfig.FilterFIFOAssignment=CAN_RX_FIFO0; //set fifo assignment
     FilterConfig.FilterIdHigh = 0x0000; // filter of zero allows all messages
     FilterConfig.FilterIdLow = 0x0000;
     FilterConfig.FilterMaskIdHigh = 0x0000;
     FilterConfig.FilterMaskIdLow = 0x0000;
     FilterConfig.FilterScale=CAN_FILTERSCALE_32BIT; //set filter scale
     FilterConfig.FilterActivation=ENABLE;
     FilterConfig.FilterBank = 0;
     FilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
     FilterConfig.SlaveStartFilterBank = 14;
     FilterConfig.FilterBank = 14;

     // try to configure the filter
     if (HAL_CAN_ConfigFilter(&hcan2, &FilterConfig) != HAL_OK)
      {
       /* Filter configuration Error */
       Error_Handler();
      }

      // Try to set up interrupts for receiving mailbox
  	  if (HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
  		 {
  		  Error_Handler();
  		 }

  	  // Try to start CAN communication - this is not sending a message, this just initializes it
      // If HAL_CAN_Start returns an error, then we want to go into the error handler
      if (HAL_CAN_Start(&hcan2) != HAL_OK)
    	{
         /* Start Error */
         Error_Handler();
    	}

  /* USER CODE END CAN2_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspInit 0 */

  /* USER CODE END CAN2_MspInit 0 */
    /* CAN2 clock enable */
    __HAL_RCC_CAN2_CLK_ENABLE();
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN2 GPIO Configuration
    PB5     ------> CAN2_RX
    PB6     ------> CAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN2 interrupt Init */
    HAL_NVIC_SetPriority(CAN2_TX_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN2_RX1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX1_IRQn);
  /* USER CODE BEGIN CAN2_MspInit 1 */

  /* USER CODE END CAN2_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspDeInit 0 */

  /* USER CODE END CAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN2_CLK_DISABLE();
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN2 GPIO Configuration
    PB5     ------> CAN2_RX
    PB6     ------> CAN2_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5|GPIO_PIN_6);

    /* CAN2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_RX1_IRQn);
  /* USER CODE BEGIN CAN2_MspDeInit 1 */

  /* USER CODE END CAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

// When a CAN message comes, the interrupt will call this function
// We need to figure out what device sent it, what the data is, and handle it appropriately
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan2){
  if (HAL_CAN_GetRxMessage(hcan2, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
  {
    Error_Handler();
  }

  //uint8_t Data[8] = {0};
  //int CAN_ID = 0;
  //int DLC = 0;

  // Extract the ID
//  if (RxHeader.IDE == CAN_ID_STD){
//	  CAN_ID = RxHeader.StdId;
//  }
//
//  if (RxHeader.IDE == CAN_ID_EXT){
//  	  CAN_ID = RxHeader.ExtId;
//  }
//
//
//  // Extract the data length
//  DLC = RxHeader.DLC; // Data Length Code


   // The data length will be different for each message, so we need to handle the possibilities
//   for (int i = 0; i < RxHeader.DLC; ++i){
//	   Data[i] = RxData[i];
//   }

   //TANNER AND FLO CALL YOUR FUNCTION HERE TO DO STUFF



}

// Here is where the second mailbox ISR would live
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan2){
	// do something
}
// CAN_Messages_Tanner
#define table_size 100 //CHAANGE THIS TO HOW MANY CAN_MESSAGES YOU NEED TO HANDLE!!!!!!


#ifdef BRUH
=======
/** struct for CAN messages
 *  Contains a 32-bit CAN id
 *  Contains a funtion pointer that will point to whatever funciton the CAN id corresponds to
 *  Contains a pointer to next, which is a handle for two CAN id's having the hash index
 *  The next will be a pointer in the hash table at the hash index that is duplicate and will point to the CAN id of the thing with the same hash
*/
typedef struct CAN_Message {
    uint32_t CAN_id;
    void* function;
    struct CAN_Message* next;
}CAN_Message;

/** Hash Table To Store CAN Messages
 *  Creates a hash table of size table_size and type CAN_Message
 *  Initialize all CAN messages in the hash table
*/
CAN_Message CAN_message_table[table_size] = {0};

/** HASH FUNCTION
*   Take a can id and return a "random" hash id
*   The hash id is in range from 0 to table_size
*   The hash id is similar to an array index in its implementation
*/
unsigned int create_hash_id(uint32_t Incoming_CAN_id) {
    unsigned int hash = 0;
    uint32_t id = Incoming_CAN_id;

    for (int i = 0; i < 4; i++) {
        hash += ((id >> (8 * i)) & 0xFF) * i;
    }
    return hash % table_size;
}

/** Function to take CAN id and find its corresponding function
*   Given a CAN id, find it in the hash table and call the function if it exists
*   If it doesn't exist, return 1
*   If it does exist but there are multiple can ids with the same hash
*   follow the next pointer until the right CAN id is found
*   Then call the function
*/
int call_function_from_CAN_id(uint32_t CAN_id, uint8_t* data, uint8_t length) {
    unsigned int index = create_hash_id(CAN_id);
    CAN_Message* current = &CAN_message_table[index];

    while (current != NULL) {
        if (current->CAN_id == CAN_id) {
            void (*function_ptr)(uint8_t* data, uint8_t length) = (void (*)(uint8_t*, uint8_t))current->function;
            if (function_ptr != NULL) {
                function_ptr(data, length);
                return 0;
            }else{
                return 1;
            }
        }
        current = current->next;
    }
    return 1;
}

/** Function to insert a message into the hash table
 *  Checks if specific hash id already exists in the hash table
 *  If not, insert the message
 *  If it already exists, malloc a new message and insert it
*/
void insert_CAN_message(CAN_Message message) {
    unsigned int index = create_hash_id(message.CAN_id);

    if(CAN_message_table[index].CAN_id == message.CAN_id) {
        CAN_Message* temp = malloc(sizeof(CAN_Message));
        temp->CAN_id = message.CAN_id;
        temp->function = message.function;
        temp->next = CAN_message_table[index].next;
        CAN_message_table[index].next = temp;

    }else{
        CAN_message_table[index] = message;
        CAN_message_table[index].next = NULL;
    }
}


/**  Function to free all malloced memory
*    Index through the hash table and free all the malloced memory at each index
*/
void nuke_hash_table() {
    CAN_Message* temp;
    for (int i = 0; i < table_size; i++) {
        temp = CAN_message_table + i*sizeof(CAN_Message);
        if(temp->next != NULL){
            temp = temp->next;
            CAN_Message* tmp2;
            while(temp != NULL){
                tmp2 = temp->next;
                free(temp);
                temp = tmp2;
            }
        }
    }

  

uv_status __uvCANtxCritSection(){
	return UV_OK;
}
#endif

/** @brief Function to send can message.
 *
 * This function is the canonical team method of sending a CAN message.
 * It invokes the canTxDaemon, to avoid any conflicts due to a context switch mid transmission
 * Is it a little bit convoluted? Yes.
 * Is that worth it? Still yes.
 */
uv_status uvSendCanMSG(uv_CAN_msg* msg){

	static TaskHandle_t can_tx_daemon_handle = NULL;
	static uv_task_id can_tx_daemon_task_id;

	if(can_tx_daemon_handle == NULL ){
		can_tx_daemon_handle = xTaskGetHandle(CAN_TX_DAEMON_NAME);
		if(can_tx_daemon_handle == NULL){
			//This means that the can_tx_daemon is probably not running
			uvPanic("CANbusTxHandler not running",0);
			return UV_ERROR;
		}
	}

	/** Check that the CAN Tx daemon is actually active
	 *
	 */

	if(msg == NULL){
		return UV_ERROR;
	}

	//BaseType_t higher_priority_task_woken = pdFALSE;

	if(xQueueSendToBack(Tx_msg_queue,&msg,0) != pdPASS){
		uvPanic("couldnt enqueue CAN message",0);
	}else{
		return UV_OK;
	}
	return UV_ERROR;
}

/** @brief Background task that handles any CAN messages that are being sent
 *
 * This task sits idle, until the time is right (it receives a notification from the uvSendCanMSG function)
 * Once this condition has been met, it will actually call the @c HAL_CAN_AddTxMessage function.
 * This is a very high priority task, meaning that it will pause whatever other code is going in order to run
 *
 */
void CANbusTxSvcDaemon(void* args){

	//CAN_TxHeaderTypeDef tx_header;

	Tx_msg_queue = xQueueCreate(8,sizeof(uv_CAN_msg*));

	//BaseType_t retval;

	uv_CAN_msg* tx_msg = NULL;

	//tx_header.TransmitGlobalTime = DISABLE;


	BaseType_t result;
	//uint32_t notif_val = 0;
	for(;;){


		result = xQueueReceive(Tx_msg_queue,&tx_msg,portMAX_DELAY);

		if(result == pdTRUE){

			if(tx_msg == NULL){
				uvPanic("cannot send null CAN msg",0);
			}

			if((tx_msg->flags)& UV_CAN_EXTENDED_ID){
				TxHeader.IDE = CAN_ID_EXT;
				TxHeader.ExtId = tx_msg->msg_id;
			}else{
				TxHeader.IDE = CAN_ID_STD;
				TxHeader.StdId = tx_msg->msg_id;
			}

			TxHeader.DLC = tx_msg->dlc;




			if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, tx_msg->data, &TxMailbox) != HAL_OK){
								/* Transmission request Error */
				uvPanic("Unable to Transmit CAN msg",0);
			}
		}

	}//main for loop

}

void CANbusRxSVCDaemon(void* args){

	for(;;){

	}

}

/* USER CODE END 1 */


/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

