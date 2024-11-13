// This where the code to handle motor controller errors and such will go


#include "motor_controller.h"

// We need to include can.h because we will send CAN messages through the functions in that file
// When a CAN message comes it will throw an interrupt can.c deals with the incoming message
// the function in can.c gets the ID and sends the data to the functions here
#include "can.h"
#include "main.h"
#include "constants.h"

// We need to include pdu.h for the shutdown circuit
#include "pdu.h"


const uint32_t MC_Expected_Serial_Number = 0x627E7A01;
const uint16_t MC_Expected_FW_Version = 0xDC01;


const uint32_t max_motor_speed = 3277; // this limits rpm of motor for speed control setpoint
uint8_t desired_motor_speed[2];



void MC_Parse_Message(int DLC, uint8_t Data[]){
	// The first step is to look at the first byte to figure out what we're looking at
	// TODO Need to make sure that Data[0] really is the first byte
	switch (Data[0]){
		// important checks
		case motor_controller_errors_warnings:
			MC_Check_Error_Warning(Data);
		break;

		// .
		// .
		// .

		default: // This is a code that is not recognized (bad)
			Error_Handler();
		break;
	}

}

// this function is used to get data from the motor controller
void MC_Request_Data(uint8_t RegID){

	TxHeader.StdId = MC_CAN_ID_Tx;
	TxHeader.DLC = 3;
	TxData[0] = 0x3D; // this is the code to request data from MC
	TxData[1] = RegID;
	TxData[2] = 0;

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK){
		/* Transmission request Error */
		Error_Handler();
	}

}



// We can either send 2 or 4 bytes of data
// This will correspond to a DLC of 3 or 5, respectively
// The first byte needs to be the register ID, then either 2 or 4 bytes of data
// The number size is either 2 or 4 and is very important
// If the motor controller wants 0x1234, the input to this function should be 0x1234
void MC_Send_Data(uint8_t RegID, uint8_t data_to_send[], uint8_t size){

	TxHeader.StdId = MC_CAN_ID_Tx;
	TxHeader.DLC = size;
	TxData[0] = RegID;

	switch (size){
	case 2:
		TxData[1] = data_to_send[2];
		TxData[2] = data_to_send[1];
	break;

	case 4:
		TxData[1] = data_to_send[4];
		TxData[2] = data_to_send[3];
		TxData[3] = data_to_send[2];
		TxData[4] = data_to_send[1];
	break;

	default:
		// bad
	break;
	}

	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK){
		/* Transmission request Error */
		Error_Handler();
	}

}

// The speed/torque control will either take the ADC value from APPSs then do math and send msg
// Or we'll do the math in the main and then just send motor controller message
void MC_Torque_Control(int todo){
	// Need to figure out best way to do this
}

//NGL, probably want these in driving loop instead
#if 0
void MC_Speed_Control(int ADC_value){

	// TODO verify endian and deal with source of data
	ADC_percentage = ADC_value / (0xFFF);
	uint16_t scaled_motor_speed = ADC_percentage*max_motor_speed;

	desired_motor_speed[0] = (scaled_motor_speed & 0xff00) >> 8;
	desired_motor_speed[1] = scaled_motor_speed & 0x00ff;

	MC_Send_Data(N_set, desired_motor_speed, 2);
}
#endif


void MC_Check_Error_Warning(uint8_t Data[]){

	// The motor controller will send a message with 6 bytes of data
	// The first byte is the register ID, which in this case is 0x8F
	// The second and third byte are low bytes which correspond to errors
	// The order of bits in the low bytes is
	// 7 6 5 4 3 2 1 0  15 14 13 12 11 10 9 8

	// The fourth and fifth bytes are high bytes and correspond to warnings
	// The order of bits in the high bytes is
	// 23 22 21 20 19 18 17 16   31 30 29 28 27 26 25 24

	// We need to and the bytes with the right bitmasks and check for errors


	// Split four bytes into 2 high and 2 low bytes

	uint16_t MC_errors = (Data[1] << 8) | Data[2];
	uint16_t MC_warnings = (Data[3] << 8) | Data[4];

	// All the error flags should be zero
	if (MC_errors){
		// Compare errors to errors bitmask to determine error
		if (MC_errors & eprom_read_error){
			// bad
		}
		if (MC_errors & hardware_fault){
			// bad
		}
		if (MC_errors & rotate_field_enable_not_present_run){
			// bad
		}
		if (MC_errors & CAN_timeout_error){
			// bad
		}
		if (MC_errors & feedback_signal_error){
			// bad
		}
		if (MC_errors & mains_voltage_min_limit){
			// bad
		}
		if (MC_errors & motor_temp_max_limit){
			// bad
		}
		if (MC_errors & IGBT_temp_max_limit){
			// bad
		}
		if (MC_errors & mains_voltage_max_limit){
			// bad
		}
		if (MC_errors & critical_AC_current){
			// bad
		}
		if (MC_errors & race_away_detected){
			// bad
		}
		if (MC_errors & ecode_timeout_error){
			// bad
		}
		if (MC_errors & watchdog_reset){
			// bad
		}
		if (MC_errors & AC_current_offset_fault){
			// bad
		}
		if (MC_errors & internal_hardware_voltage_problem){
			// bad
		}
		if (MC_errors & bleed_resistor_overload){
			// bad
		}
	}

	if (MC_warnings){
		// compare warnings to warnings bitmask
		if (MC_warnings & parameter_conflict_detected){
			// not great
		}
		if (MC_warnings & special_CPU_fault){
			// not great
		}
		if (MC_warnings & rotate_field_enable_not_present_norun){
			// not great
		}
		if (MC_warnings & auxiliary_voltage_min_limit){
			// not great
		}
		if (MC_warnings & feedback_signal_problem){
			// not great
		}
		if (MC_warnings & warning_5){
			// not great
		}
		if (MC_warnings & motor_temperature_warning){
			// not great
		}
		if (MC_warnings & IGBT_temperature_warning){
			// not great
		}
		if (MC_warnings & Vout_saturation_max_limit){
			// not great
		}
		if (MC_warnings & warning_9){
			// not great
		}
		if (MC_warnings & speed_actual_resolution_limit){
			// not great
		}
		if (MC_warnings & check_ecode_ID ){
			// not great
		}
		if (MC_warnings & tripzone_glitch_detected){
			// not great
		}
		if (MC_warnings & ADC_sequencer_problem){
			// not great
		}
		if (MC_warnings & ADC_measurement_problem){
			// not great
		}
		if (MC_warnings & bleeder_resistor_warning){
			// not great
		}
	}
}

void MC_Validate(){

}

void MC_Check_Serial_Number(uint8_t Data[]){
	// TODO
}

void MC_Check_Firmware(uint8_t Data[]){
	// TODO
}

void MC_Startup(void* args){
	// MC_Send_Data(...)
	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_14);
	uv_init_task_args* params = (uv_init_task_args*) args;

	uv_init_task_response response = {UV_OK,MOTOR_CONTROLLER,0,NULL};
	//We need to do a bunch of stuff to actually initialize the motor controller here

	motor_controller_settings* settings = (motor_controller_settings*) params->specific_args;



	if(xQueueSendToBack(params->init_info_queue,&response,100) != pdPASS){
		//OOPS
		uvPanic("Failed to enqueue MC OK Response",0);
	}





	//Kill yourself
	vTaskSuspend(params->meta_task_handle);
}



