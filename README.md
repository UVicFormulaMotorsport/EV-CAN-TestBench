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


