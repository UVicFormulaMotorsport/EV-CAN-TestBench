# EV-CAN-TestBench
Code for EV CAN Test Bench

Go to folder core -> src for .c files 

Go to folder core -> inc for .h files

CAN initialization is done in can.c

Every device should have it's own .c file where CAN ID and data length are set.

Those can be changed with  
  TxHeader.DLC=1; // Data Length Code
  TxHeader.StdId=0x244; // This is the CAN ID

Refer to the CAN messages format & info spreadsheet under sharepoint System Specific -> Control System -> CAN Messages


Known Issues: TxHeader declaration needs to be in main currently since the code sends out a chirp for debugging purposes. 
TxHeader needs to be passed to CAN_Init function. If code is generated, it will turn the function to type void so needs to be changed in the can.c and can.h files


