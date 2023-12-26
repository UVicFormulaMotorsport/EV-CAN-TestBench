#ifndef SOME_UNIQUE_NAME_HERE
#define SOME_UNIQUE_NAME_HERE

// your declarations (and certain types of definitions) here

extern CAN_TxHeaderTypeDef   TxHeader;
extern CAN_RxHeaderTypeDef   RxHeader;

extern uint8_t               TxData[8];
extern uint32_t              TxMailbox;
extern uint8_t               RxData[8];


// For constant CAN IDs, might be easier to put them in here
enum CAN_IDs{
	IMD_CAN_ID_Tx = 0x23, // This is the ID for MCU sending msg to IMD
	IMD_CAN_ID_Rx = 0x24,// MCU will receive messages with id 0x24 from IMD
	PDU_CAN_ID_Tx = 0x710, // ID to send messages to the PDU
};


#endif



