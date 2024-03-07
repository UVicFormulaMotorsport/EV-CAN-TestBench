// This where the code to handle IMD errors and such will go


#include "imd.h"

// We need to include can.h because we will send CAN messages through the functions in that file
// When a CAN message comes it will throw an interrupt can.c deals with the incoming message
// the function in can.c gets the ID and sends the data to  the functions here
#include "can.h"
#include "main.h"
#include "constants.h"

// We need to include pdu.h for the shutdown circuit
#include "pdu.h"



uint8_t IMD_status_bits = 0;
uint8_t IMD_High_Uncertainty = 0;



// Declarations for the IMD part name to be checked on startup
uint32_t IMD_Read_Part_Name[4];
const uint32_t IMD_Expected_Part_Name[4];

uint8_t IMD_Part_Name_0_Set = 0;
uint8_t IMD_Part_Name_1_Set = 0;
uint8_t IMD_Part_Name_2_Set = 0;
uint8_t IMD_Part_Name_3_Set = 0;
uint8_t IMD_Part_Name_Set = 0;


uint32_t IMD_Read_Version[3];
const uint32_t IMD_Expected_Version[3];

uint8_t IMD_Version_0_Set = 0;
uint8_t IMD_Version_1_Set = 0;
uint8_t IMD_Version_2_Set = 0;
uint8_t IMD_Version_Set = 0;


// Declarations for the IMD serial number to be checked on startup
uint32_t IMD_Read_Serial_Number[4];
const uint32_t IMD_Expected_Serial_Number[4] = {0xB8DD9AF9,
												0x6094F48B,
												0x1F1C3794,
												0xFCF9A95B};
uint8_t IMD_Serial_Number_0_Set = 0;
uint8_t IMD_Serial_Number_1_Set = 0;
uint8_t IMD_Serial_Number_2_Set = 0;
uint8_t IMD_Serial_Number_3_Set = 0;
uint8_t IMD_Serial_Number_Set = 0;


int32_t IMD_Temperature;


// If there is a hardware error, that one bit will be a 1 in the status bits -> read error flags
// error flags will return the status bits which will have a 1 in HE bit -> infinite loop
uint8_t IMD_error_flags_requested = 0;




// Need a function to parse the CAN message data received from the IMD
void IMD_Parse_Message(int DLC, uint8_t Data[]){
	// The first step is to look at the first byte to figure out what we're looking at

	switch (Data[0]){
		// important checks
		case isolation_state:
			IMD_Check_Status_Bits(Data[1]);
			IMD_Check_Isolation_State(Data);
		break;

		case isolation_resistances:
			IMD_Check_Status_Bits(Data[1]);
			IMD_Check_Isolation_Resistances(Data);
		break;

		case isolation_capacitances:
			IMD_Check_Status_Bits(Data[1]);
			IMD_Check_Isolation_Capacitances(Data);
		break;

		case voltages_Vp_and_Vn:
			IMD_Check_Status_Bits(Data[1]);
			IMD_Check_Voltages_Vp_and_Vn(Data);
		break;

		case battery_voltage:
			IMD_Check_Status_Bits(Data[1]);
			IMD_Check_Battery_Voltage(Data);
		break;

		case Error_flags:
			IMD_Check_Status_Bits(Data[1]);
			IMD_Check_Error_Flags(Data);
		break;

		case safety_touch_energy:
			IMD_Check_Status_Bits(Data[1]);
			IMD_Check_Safety_Touch_Energy(Data);
		break;

		case safety_touch_current:
			IMD_Check_Status_Bits(Data[1]);
			IMD_Check_Safety_Touch_Current(Data);
		break;

		// high resolution measurements
		case Vn_hi_res:
			// do something
		break;

		case Vp_hi_res:
			// do something
		break;

		case Vexc_hi_res:
			// do something
		break;

		case Vb_hi_res:
			// do something
		break;

		case Vpwr_hi_res:
			// do something
		break;

		case Temperature:
			IMD_Check_Temperature(Data);
		break;

		case Max_battery_working_voltage:
			IMD_Check_Max_Battery_Working_Voltage(Data);
		break;

		// ugly syntax below
		case Part_name_0:
		case Part_name_1:
		case Part_name_2:
		case Part_name_3:
			IMD_Check_Part_Name(Data);
		break;

		case Version_0:
		case Version_1:
		case Version_2:
			IMD_Check_Version(Data);
		break;

		case Serial_number_0:
		case Serial_number_1:
		case Serial_number_2:
		case Serial_number_3:
			IMD_Check_Serial_Number(Data);
		break;

		case Uptime_counter:
			// call check uptime counter
		break;


		default: // This is a code that is not recognized (bad)
			Error_Handler();
		break;
	}

}


// --------------------------------------------------------------------------------------
// This sends the message to request data. The specific status requested is passed as arg
// The IMD will then send a message with the same code and the data
// --------------------------------------------------------------------------------------
void IMD_Request_Status(uint8_t Status){
	TxHeader.IDE = CAN_ID_EXT;
	TxHeader.ExtId = IMD_CAN_ID_Tx;
	TxHeader.DLC = 1;
	TxData[0] = Status;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK){
		/* Transmission request Error */
		Error_Handler();
    }
	TxHeader.IDE = CAN_ID_STD;
}

// --------------------------------------------------------------------------------------











// --------------------------------------------------------------------------------------
// Functions to check status
// AFAIK the IMD will not send a CAN msg when the status changes - we need to constantly poll it
// --------------------------------------------------------------------------------------

// A lot of the messages will include status bits
// Check for faults
// Then check what the error is to display it for driver
void IMD_Check_Status_Bits(uint8_t Data){
	// The touch energy bit will be 1 when connected to batteries
	// High uncertainty isn't also something we really care about
	// No idea about excitation pulse
	uint8_t mask = 0b10001111;

	if ((Data & mask) != 0){
		// Send message to error handler to shutdown car
		//Trigger_Shutdown_Circuit();

		if ((Data & Isolation_status_bit0) || (Data & Isolation_status_bit1)){
			// Isolation fault BAD
			// Want to display fault to dash
		}

		// This function is only passed the first byte of info so we can't read the error flags
		// If we pass the entire data array in then we will read the wrong data
		// Need to explicitly request error flags and then read it
		// Use a bool to check if we have already requested error flags otherwise it will request repeatedly
		if (Data & Hardware_Error){
			// TODO
			// display to dash
			if (!IMD_error_flags_requested){
				IMD_Request_Status(Error_flags);
				IMD_error_flags_requested = 1;
			}
		}

		if (Data & Low_Battery_Voltage){
			// display low voltage on dash
			// If the HV battery ever throws this error it is because of a disconnect
		}
		if (Data & High_Battery_Voltage){
			// display high voltage on dash
			// If the HV battery ever throws this error it is bad
		}
	}
	// Could check other faults we don't really care about

	IMD_High_Uncertainty = Data & High_Uncertainty;
	// If we made it here then there is no error so exit to check rest of message
}

// We need to look at the 2nd and 3rd bytes in the array for the error flags
void IMD_Check_Error_Flags(uint8_t Data[]){
	// Need to check the bits to see what caused the hardware error
	// Want to display message to dash for safety reasons
	uint16_t IMD_Error_Flags = (Data[1] << 8) | Data[2];

	if (IMD_Error_Flags & Err_Vx1){
		// print to dash I guess
	}
	if (IMD_Error_Flags & Err_Vx2){
		// print to dash I guess
	}
	if (IMD_Error_Flags & Err_CH){
		// print to dash I guess
	}
	if (IMD_Error_Flags & Err_VxR){
		// print to dash I guess
	}
	if (IMD_Error_Flags & Err_Vexi){
		// print to dash I guess
	}
	if (IMD_Error_Flags & Err_Vpwr){
		// print to dash I guess
	}
	if (IMD_Error_Flags & Err_Watchdog){
		// print to dash I guess
	}
	if (IMD_Error_Flags & Err_clock){
		// print to dash I guess
	}
	if (IMD_Error_Flags & Err_temp){
		// print to dash I guess
	}
}



// This is the function that will be called when a CAN message is received that has the isolation state data
void IMD_Check_Isolation_State(uint8_t Data[]){

	uint16_t isolation = (Data[2] << 8) | Data[3];

	// If the isolation is less than 500 Ohms / volt and the uncertainty is less than 5%
	if ( (isolation < 500) && (Data[4] <= 5) ){

		// TODO disable shutdown circuit and display error
		IMD_High_Uncertainty = 0;
	}

}

// Not sure if we necessarily need to check isolation resistances
// check isolation state will be much more important
// We should, however, check this on startup
void IMD_Check_Isolation_Resistances(uint8_t Data[]){

	uint16_t Rp_resistance = (Data[2] << 8) | Data[3];

	// If the isolation resistance between the positive terminal and the chassis
	// is less than 250 kOhms and the uncertainty is less than 5%
	if ( (Rp_resistance < 250) && (Data[4] <= 5) ){

		// TODO disable shutdown circuit and display error
		IMD_High_Uncertainty = 0;
	}


	uint16_t Rn_resistance = (Data[5] << 8) | Data[6];

	// If the isolation resistance between the negative terminal and the chassis
	// is less than 250 kOhms and the uncertainty is less than 5%
	if ( (Rn_resistance < 250) && (Data[7] <= 5) ){

		// TODO disable shutdown circuit and display error
		IMD_High_Uncertainty = 0;
	}
}


void IMD_Check_Isolation_Capacitances(uint8_t Data[]){

	// I don't know how useful this will be

}


void IMD_Check_Voltages_Vp_and_Vn(uint8_t Data[]){

	// This could potentially be useful on startup

}


void IMD_Check_Battery_Voltage(uint8_t Data[]){

	// This could be useful to compare with BMS and make sure things are working well
	// startup function really

}

void IMD_Check_Temperature(uint8_t Data[]){
	// TODO

	// byte 1-4 in motorola
	IMD_Temperature = (Data[4] << 24) | (Data[3] << 16) | (Data[2] << 8) | Data[1];

}

// -----------------------------------------------------------------------------------
// These functions could check to see if stuff is safe to touch

void IMD_Check_Safety_Touch_Energy(uint8_t Data[]){

	// I don't really know how to make use of these functions

}


void IMD_Check_Safety_Touch_Current(uint8_t Data[]){
	// TODO
}






// ----------------------------------------------------------------------------
// Data that could be checked on startup to make sure everything is good

void IMD_Check_Max_Battery_Working_Voltage(uint8_t Data[]){
	uint16_t Max_Battery_Voltage = (Data[1] << 8) | Data[2];

	if (Max_Battery_Voltage != 571){
		// Max_Battery_Voltage not configured properly
	}

}


// This function checks the part name of the IMD matches expected
// The part name is split into 4 messages, each of 4 bytes
// Because it is split over 4 messages, we need to compare only once we have read all messages
void IMD_Check_Part_Name(uint8_t Data[]){
	// TODO

	// This function will be called from the CAN msg parser
	// It will get the array of data bits. We need to check which part name
	// We then store the 4 bytes in an array of 32 bit int to compare at the end

	switch (Data[0]){
		case Part_name_0:
			IMD_Read_Part_Name[0] = (Data[4] << 24) | (Data[3] << 16) | (Data[2] << 8) | Data[1];
			IMD_Part_Name_0_Set = 1;
		break;
		case Part_name_1:
			IMD_Read_Part_Name[1] = (Data[4] << 24) | (Data[3] << 16) | (Data[2] << 8) | Data[1];
			IMD_Part_Name_1_Set = 1;
		break;
		case Part_name_2:
			IMD_Read_Part_Name[2] = (Data[4] << 24) | (Data[3] << 16) | (Data[2] << 8) | Data[1];
			IMD_Part_Name_2_Set = 1;
		break;
		case Part_name_3:
			IMD_Read_Part_Name[3] = (Data[4] << 24) | (Data[3] << 16) | (Data[2] << 8) | Data[1];
			IMD_Part_Name_3_Set = 1;
		break;
	}

	if (IMD_Part_Name_0_Set && IMD_Part_Name_1_Set && IMD_Part_Name_2_Set && IMD_Part_Name_3_Set){
		IMD_Part_Name_Set = 1;
	}

	if (IMD_Part_Name_Set){
		// Check part number matches expected
		for (int i = 0; i < 4; ++i){
			if (IMD_Read_Part_Name[0] != IMD_Expected_Part_Name[0]){
				//error
			}
		}

	}

}

void IMD_Check_Version(uint8_t Data[]){
	// TODO

	// This function will be called from the CAN msg parser
	// It will get the array of data bits. We need to check which firmware version
	// We then store the 4 bytes in an array of 32 bit int to compare at the end

	switch (Data[0]){
		case Version_0:
			IMD_Read_Version[0] = (Data[3] << 16) | (Data[2] << 8) | Data[1];
			IMD_Version_0_Set = 1;
		break;
		case Version_1:
			IMD_Read_Version[1] = (Data[3] << 16) | (Data[2] << 8) | Data[1];
			IMD_Version_1_Set = 1;
		break;
		case Version_2:
			IMD_Read_Version[2] = (Data[3] << 16) | (Data[2] << 8) | Data[1];
			IMD_Version_2_Set = 1;
		break;
	}

	if (IMD_Version_0_Set && IMD_Version_1_Set && IMD_Version_2_Set){
		IMD_Version_Set = 1;
	}

	if (IMD_Version_Set){
		// Check part number matches expected
		for (int i = 0; i < 3; ++i){
			if (IMD_Read_Version[0] != IMD_Expected_Version[0]){
				//error
			}
		}

	}
}

// This function checks the serial number of the IMD matches expected
// The part name is split into 4 messages, each of 4 bytes
// Because it is split over 4 messages, we need to compare only once we have read all messages
void IMD_Check_Serial_Number(uint8_t Data[]){

	// This function will be called from the CAN msg parser
	// It will get the array of data bits. We need to check which serial number
	// We then store the 4 bytes in an array of 32 bit int to compare at the end
	// The serial number is found by concatenating 3 - 2 - 1 -0

	switch (Data[0]){
		case Serial_number_0:
			IMD_Read_Serial_Number[0] = (Data[1] << 24) | (Data[2] << 16) | (Data[3] << 8) | Data[4];
			IMD_Serial_Number_0_Set = 1;
		break;
		case Serial_number_1:
			IMD_Read_Serial_Number[1] = (Data[1] << 24) | (Data[2] << 16) | (Data[3] << 8) | Data[4];
			IMD_Serial_Number_1_Set = 1;
		break;
		case Serial_number_2:
			IMD_Read_Serial_Number[2] = (Data[1] << 24) | (Data[2] << 16) | (Data[3] << 8) | Data[4];
			IMD_Serial_Number_2_Set = 1;
		break;
		case Serial_number_3:
			IMD_Read_Serial_Number[3] = (Data[1] << 24) | (Data[2] << 16) | (Data[3] << 8) | Data[4];
			IMD_Serial_Number_3_Set = 1;
		break;
	}

	if (IMD_Serial_Number_0_Set && IMD_Serial_Number_1_Set && IMD_Serial_Number_2_Set && IMD_Serial_Number_3_Set){
		IMD_Serial_Number_Set = 1;
	}

	if (IMD_Serial_Number_Set){
		// Check serial number matches expected
		for (int i = 0; i < 4; ++i){
			if (IMD_Read_Serial_Number[i] != IMD_Expected_Serial_Number[i]){
				//error
			}
		}
	}

}

void IMD_Check_Uptime(uint8_t Data[]){
	// TODO
}

void IMD_Startup(){
	// TODO
	// Run check for serial number, max voltage, and such

	// The first check is the serial number

	IMD_Request_Status(Serial_number_0);
	IMD_Request_Status(Serial_number_1);
	IMD_Request_Status(Serial_number_2);
	IMD_Request_Status(Serial_number_3);

	IMD_Request_Status(Version_0);
	IMD_Request_Status(Version_1);
	IMD_Request_Status(Version_2);

	IMD_Request_Status(Part_name_0);
	IMD_Request_Status(Part_name_1);
	IMD_Request_Status(Part_name_2);
	IMD_Request_Status(Part_name_3);

	IMD_Request_Status(Max_battery_working_voltage);
	IMD_Request_Status(isolation_state);
	// Can check further things

}




