//Motor Controller
// Includes
#include "motor_controller.h"  // Functions and constants for motor control
#include "can.h"               // CAN communication functions
#include "main.h"              // Application-level definitions
#include "cmsis_os.h"          // RTOS-related functionality
#include "pdu.h"               // Shutdown circuit interface

// --------------------------------------------
// External Resources (provided by can.c)
// --------------------------------------------
extern QueueHandle_t canRxQueue;             // Queue for CAN receive messages
extern QueueHandle_t canTxQueue;             // Queue for CAN transmit messages
//extern SemaphoreHandle_t canResponseSemaphore; // Semaphore for synchronizing CAN responses

// --------------------------------------------
// Motor Controller Constants
// --------------------------------------------
const uint32_t MC_Expected_Serial_Number = 0x627E7A01; // Expected serial number
const uint16_t MC_Expected_FW_Version = 0xDC01;        // Expected firmware version
const uint32_t max_motor_speed = 3277;                 // Max RPM for speed control

//uint32_t TxMailbox; //Ensure TxMailbox is defined as a uint32_t variable in your motor_controller.c file or globally in can.c if used across multiple files:

// Motor controller settings with default values
motor_controller_settings mc_default_settings = {
    .can_id_tx = 0x200,              // CAN transmit ID
    .can_id_rx = 0x201,              // CAN receive ID
    .mc_CAN_timeout = 2,             // CAN timeout in seconds
    .proportional_gain = 10,         // Proportional gain (Kp)
    .integral_time_constant = 400,   // Integral time constant (Ti)
    .integral_memory_max = 0.6       // Max integral memory (60%)
};

// --------------------------------------------
// Function Prototypes
// --------------------------------------------
static uint32_t Parse_Bamocar_Response(uint8_t *data, uint8_t length);
static void MotorControllerErrorHandler(uint8_t *data, uint8_t length);
static uint16_t MotorControllerSpinTest(void);
static bool WaitFor_CAN_Response(void);
void MC_Request_Data(uint8_t RegID);
//void MotorController_Init(void);
//void MC_Startup(void);


// --------------------------------------------
// Function Definitions
// --------------------------------------------



// Spin Motor Test
/**
 * Commands the motor to spin at a low RPM and validates the motor's response:
 * 1. Sends a spin command via CAN.
 * 2. Waits for the motor to reach the desired speed.
 * 3. Checks the actual speed from the motor controller.
 * 4. Stops the motor after validation.
 * @return 0 if the test is successful, 1 for timeout, or 2 for insufficient speed.
 */
static uint16_t MotorControllerSpinTest(void) {
    uint8_t spin_command[2] = {0x00, 0x10}; // Command to spin at low RPM

    // Use CAN_TxHeaderTypeDef for message header
    CAN_TxHeaderTypeDef txHeader;
    txHeader.StdId = mc_default_settings.can_id_tx;    // Set CAN ID
    txHeader.IDE = CAN_ID_STD;                // Use standard ID
    txHeader.RTR = CAN_RTR_DATA;              // Data frame
    txHeader.DLC = 3;                         // Data length code

    // Transmit the message
    if (HAL_CAN_AddTxMessage(&hcan2, &txHeader, (uint8_t[]){N_set, spin_command[0], spin_command[1]}, &TxMailbox) != HAL_OK) {
        return 1; // Transmission error
    }

    vTaskDelay(pdMS_TO_TICKS(500)); // Allow time for the motor to spin

    // Request actual motor speed
    MC_Request_Data(N_actual);
    if (!WaitFor_CAN_Response()) {
        return 2; // Timeout occurred
    }

    uint16_t actual_speed = (RxData[0] << 8) | RxData[1];
    if (actual_speed < 0x10) { // If the motor did not reach the expected speed
        return 3; // Motor failed to spin
    }

    // Stop the motor
    uint8_t stop_command[2] = {0x00, 0x00};
    if (HAL_CAN_AddTxMessage(&hcan2, &txHeader, (uint8_t[]){N_set, stop_command[0], stop_command[1]}, &TxMailbox) != HAL_OK) {
        return 4; // Transmission error
    }

    return 0; // Success
}

// Parse Bamocar Response
/**
 * Parses a 32-bit response value from a Bamocar CAN message.
 * Combines the four bytes of the payload into a single 32-bit integer.
 * @param data: Pointer to the CAN message payload.
 * @param length: Length of the data payload (expected to be 4 bytes).
 * @return Parsed 32-bit value.
 */
static uint32_t Parse_Bamocar_Response(uint8_t *data, uint8_t length) {
    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

// Handle Motor Controller Errors/Warnings
/**
 * Processes error and warning information from the motor controller.
 * 1. Extracts error and warning flags from the CAN message payload.
 * 2. Logs or triggers a panic if critical errors are detected.
 * @param data: Pointer to the CAN message payload.
 * @param length: Length of the data payload.
 */
static void MotorControllerErrorHandler(uint8_t *data, uint8_t length) {
    uint16_t MC_errors = (data[1] << 8) | data[2]; // Low Bytes: error flags
    uint16_t MC_warnings = (data[3] << 8) | data[4]; // High Bytes: warning flags

    // Check for errors
    if (MC_errors & eprom_read_error) {
        uvPanic("EPROM Read Error", 0);
    }
    if (MC_errors & hardware_fault) {
        uvPanic("Hardware Fault", 0);
    }
    if (MC_errors & rotate_field_enable_not_present_run) {
        uvPanic("Rotating Field Enable Not Present (Run Active)", 0);
    }
    if (MC_errors & CAN_timeout_error) {
        uvPanic("CAN Timeout Error", 0);
    }
    if (MC_errors & feedback_signal_error) {
        uvPanic("Feedback Signal Error", 0);
    }
    if (MC_errors & mains_voltage_min_limit) {
        uvPanic("Mains Voltage Below Minimum Limit", 0);
    }
    if (MC_errors & motor_temp_max_limit) {
        uvPanic("Motor Temperature Exceeded Maximum Limit", 0);
    }
    if (MC_errors & IGBT_temp_max_limit) {
        uvPanic("IGBT Temperature Exceeded Maximum Limit", 0);
    }
    if (MC_errors & mains_voltage_max_limit) {
        uvPanic("Mains Voltage Exceeded Maximum Limit", 0);
    }
    if (MC_errors & critical_AC_current) {
        uvPanic("Critical AC Current Detected", 0);
    }
    if (MC_errors & race_away_detected) {
        uvPanic("Race Away Detected", 0);
    }
    if (MC_errors & ecode_timeout_error) {
        uvPanic("Ecode Timeout Error", 0);
    }
    if (MC_errors & watchdog_reset) {
        uvPanic("Watchdog Reset Occurred", 0);
    }
    if (MC_errors & AC_current_offset_fault) {
        uvPanic("AC Current Offset Fault", 0);
    }
    if (MC_errors & internal_hardware_voltage_problem) {
        uvPanic("Internal Hardware Voltage Problem", 0);
    }
    if (MC_errors & bleed_resistor_overload) {
        uvPanic("Bleed Resistor Overload", 0);
    }

    // Check for warnings
    if (MC_warnings & parameter_conflict_detected) {
        //uvLog("Parameter Conflict Detected");
    }
    if (MC_warnings & special_CPU_fault) {
        //uvLog("Special CPU Fault Detected");
    }
    if (MC_warnings & rotate_field_enable_not_present_norun) {
        //uvLog("Rotating Field Enable Not Present (No Run Active)");
    }
    if (MC_warnings & auxiliary_voltage_min_limit) {
        //uvLog("Auxiliary Voltage Below Minimum Limit");
    }
    if (MC_warnings & feedback_signal_problem) {
       //uvLog("Feedback Signal Problem Detected");
    }
    if (MC_warnings & warning_5) {
        //uvLog("Warning 5 Detected");
    }
    if (MC_warnings & motor_temperature_warning) {
        //uvLog("Motor Temperature Warning (>87%)");
    }
    if (MC_warnings & IGBT_temperature_warning) {
        //uvLog("IGBT Temperature Warning (>87%)");
    }
    if (MC_warnings & Vout_saturation_max_limit) {
        //uvLog("Output Voltage Saturation Reached Maximum Limit");
    }
    if (MC_warnings & warning_9) {
        //uvLog("Warning 9 Detected");
    }
    if (MC_warnings & speed_actual_resolution_limit) {
        //uvLog("Speed Actual Resolution Limit Exceeded");
    }
    if (MC_warnings & check_ecode_ID) {
        //uvLog("Check Ecode ID Warning");
    }
    if (MC_warnings & tripzone_glitch_detected) {
        //uvLog("Tripzone Glitch Detected");
    }
    if (MC_warnings & ADC_sequencer_problem) {
        //uvLog("ADC Sequencer Problem Detected");
    }
    if (MC_warnings & ADC_measurement_problem) {
        //uvLog("ADC Measurement Problem Detected");
    }
    if (MC_warnings & bleeder_resistor_warning) {
        //uvLog("Bleeder Resistor Warning");
    }
}

// Request Data from Motor Controller
/**
 * Sends a CAN request to the motor controller to retrieve a specific register value.
 * Constructs a CAN message with the specified register ID and sends it via the CAN queue.
 * @param RegID: The ID of the register to request data from.
 */
void MC_Request_Data(uint8_t RegID) {
    // Define the CAN header
    CAN_TxHeaderTypeDef txHeader;
    uint32_t TxMailbox;

    // Set up the CAN message header
    txHeader.StdId = mc_default_settings.can_id_tx; // Set the standard CAN ID from settings
    txHeader.IDE = CAN_ID_STD;             // Use standard ID
    txHeader.RTR = CAN_RTR_DATA;           // Data frame
    txHeader.DLC = 3;                      // Data length code

    // Define the message payload
    uint8_t data[3] = {0x3D, RegID, 0};    // Register request format

    // Transmit the CAN message
    if (HAL_CAN_AddTxMessage(&hcan2, &txHeader, data, &TxMailbox) != HAL_OK) {
        // Handle transmission error
        uvPanic("CAN Request Transmission Failed", 0);
    }
}

// Wait for CAN Response
/**
 * Waits for a CAN response from the motor controller.
 * Uses an RTOS semaphore to synchronize and check if a response is received within the timeout period.
 * @return True if a response is received, otherwise false.
 */
static bool WaitFor_CAN_Response(void) {
   // return xSemaphoreTake(canResponseSemaphore, pdMS_TO_TICKS(mc_default_settings.mc_CAN_timeout)) == pdTRUE;
}
//-------------------------------------------------------------------------------------------------------------

//void MC_Parse_Message(int DLC, uint8_t Data[]){
//	// The first step is to look at the first byte to figure out what we're looking at
//	// TODO Need to make sure that Data[0] really is the first byte
//	switch (Data[0]){
//		// important checks
//		case motor_controller_errors_warnings:
//			MC_Check_Error_Warning(Data);
//		break;
//
//		//case N_actual:
//		// Decode actual motor speed from Data bytes
//			//uint16_t actual_speed = (Data[1] << 8) | Data[2];
//		//break;
//
//		        // Additional cases for other message types:
//		        // case motor_temperature:
//		        // case DC_bus_voltage:
//		        // etc.
//
//		default: // This is a code that is not recognized (bad)
//			Error_Handler();
//		break;
//	}
//
//}

//// this function is used to get data from the motor controller
//void MC_Request_Data(uint8_t RegID){
//
//	TxHeader.StdId = MC_CAN_ID_Tx;
//	TxHeader.DLC = 3;
//	TxData[0] = 0x3D; // this is the code to request data from MC
//	TxData[1] = RegID;
//	TxData[2] = 0;
//
//	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK){
//		/* Transmission request Error */
//		Error_Handler();
//	}
//
//}



// We can either send 2 or 4 bytes of data
// This will correspond to a DLC of 3 or 5, respectively
// The first byte needs to be the register ID, then either 2 or 4 bytes of data
// The number size is either 2 or 4 and is very important
// If the motor controller wants 0x1234, the input to this function should be 0x1234
//void MC_Send_Data(uint8_t RegID, uint8_t data_to_send[], uint8_t size){
//
//	TxHeader.StdId = MC_CAN_ID_Tx;
//	TxHeader.DLC = size;
//	TxData[0] = RegID;
//
//	switch (size){
//	case 2:
//		TxData[1] = data_to_send[2];
//		TxData[2] = data_to_send[1];
//	break;
//
//	case 4:
//		TxData[1] = data_to_send[4];
//		TxData[2] = data_to_send[3];
//		TxData[3] = data_to_send[2];
//		TxData[4] = data_to_send[1];
//	break;
//
//	default:
//		// bad
//	break;
//	}
//
//	if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK){
//		/* Transmission request Error */
//		Error_Handler();
//	}
//
//}

// The speed/torque control will either take the ADC value from APPSs then do math and send msg
// Or we'll do the math in the main and then just send motor controller message
//void MC_Torque_Control(int todo){
//	// Need to figure out best way to do this
//}

//NGL, probably want these in driving loop instead
//#if 0
//void MC_Speed_Control(int ADC_value){
//
//	// TODO verify endian and deal with source of data
//	ADC_percentage = ADC_value / (0xFFF);
//	uint16_t scaled_motor_speed = ADC_percentage*max_motor_speed;
//
//	desired_motor_speed[0] = (scaled_motor_speed & 0xff00) >> 8;
//	desired_motor_speed[1] = scaled_motor_speed & 0x00ff;
//
//	MC_Send_Data(N_set, desired_motor_speed, 2);
//}
//#endif


//void MC_Check_Error_Warning(uint8_t Data[]){
//
//	// The motor controller will send a message with 6 bytes of data
//	// The first byte is the register ID, which in this case is 0x8F
//	// The second and third byte are low bytes which correspond to errors
//	// The order of bits in the low bytes is
//	// 7 6 5 4 3 2 1 0  15 14 13 12 11 10 9 8
//
//	// The fourth and fifth bytes are high bytes and correspond to warnings
//	// The order of bits in the high bytes is
//	// 23 22 21 20 19 18 17 16   31 30 29 28 27 26 25 24
//
//	// We need to and the bytes with the right bitmasks and check for errors
//
//
//	// Split four bytes into 2 high and 2 low bytes
//
//	uint16_t MC_errors = (Data[1] << 8) | Data[2];
//	uint16_t MC_warnings = (Data[3] << 8) | Data[4];
//
//	// All the error flags should be zero
//	if (MC_errors){
//		// Compare errors to errors bitmask to determine error
//		if (MC_errors & eprom_read_error){
//			// bad
//			uvPanic("Eprom read error", 0);
//		}
//		if (MC_errors & hardware_fault){
//			// bad
//			uvPanic("Hardware Fault", 0);
//		}
//		if (MC_errors & rotate_field_enable_not_present_run){
//			// bad
//			uvPanic("Rotating Field Enable Not Present", 0);
//		}
//		if (MC_errors & CAN_timeout_error){
//			// bad
//			uvPanic("CAN Timeout Error", 0);
//		}
//		if (MC_errors & feedback_signal_error){
//			// bad
//			uvPanic("Feedback Signal Error", 0);
//		}
//		if (MC_errors & mains_voltage_min_limit){
//			// bad
//			uvPanic("Mains Voltage Min Limit", 0);
//		}
//		if (MC_errors & motor_temp_max_limit){
//			// bad
//			uvPanic("Motor Temp Max Limit", 0);
//		}
//		if (MC_errors & IGBT_temp_max_limit){
//			// bad
//			uvPanic("IGBT Temp Max Limit", 0);
//		}
//		if (MC_errors & mains_voltage_max_limit){
//			// bad
//			uvPanic("Mains Voltage Max Limit", 0);
//		}
//		if (MC_errors & critical_AC_current){
//			// bad
//			uvPanic("Critical AC Current", 0);
//		}
//		if (MC_errors & race_away_detected){
//			// bad
//			uvPanic("Race Away Detected", 0);
//		}
//		if (MC_errors & ecode_timeout_error){
//			// bad
//			uvPanic("Ecode Time Out Error", 0);
//		}
//		if (MC_errors & watchdog_reset){
//			// bad
//			uvPanic("Watchdog Reset", 0);
//		}
//		if (MC_errors & AC_current_offset_fault){
//			// bad
//			uvPanic("AC Current Offset Fault", 0);
//		}
//		if (MC_errors & internal_hardware_voltage_problem){
//			// bad
//			uvPanic("Internal Hardware Voltage Problem", 0);
//		}
//		if (MC_errors & bleed_resistor_overload){
//			// bad
//			uvPanic("Motor Controller not found", 0);
//		}
//	}
//
//	if (MC_warnings){
//		// compare warnings to warnings bitmask
//		if (MC_warnings & parameter_conflict_detected){
//			// not great
//		}
//		if (MC_warnings & special_CPU_fault){
//			// not great
//		}
//		if (MC_warnings & rotate_field_enable_not_present_norun){
//			// not great
//		}
//		if (MC_warnings & auxiliary_voltage_min_limit){
//			// not great
//		}
//		if (MC_warnings & feedback_signal_problem){
//			// not great
//		}
//		if (MC_warnings & warning_5){
//			// not great
//		}
//		if (MC_warnings & motor_temperature_warning){
//			// not great
//		}
//		if (MC_warnings & IGBT_temperature_warning){
//			// not great
//		}
//		if (MC_warnings & Vout_saturation_max_limit){
//			// not great
//		}
//		if (MC_warnings & warning_9){
//			// not great
//		}
//		if (MC_warnings & speed_actual_resolution_limit){
//			// not great
//		}
//		if (MC_warnings & check_ecode_ID ){
//			// not great
//		}
//		if (MC_warnings & tripzone_glitch_detected){
//			// not great
//		}
//		if (MC_warnings & ADC_sequencer_problem){
//			// not great
//		}
//		if (MC_warnings & ADC_measurement_problem){
//			// not great
//		}
//		if (MC_warnings & bleeder_resistor_warning){
//			// not great
//		}
//	}
//}


//void MC_Check_Serial_Number(uint8_t Data[]){
//	// TODO check serial number
////    uint32_t serial_number = MC_Request_Data(SERIAL_NUMBER_REGISTER);  // Request serial number
////    return serial_number == MC_Expected_Serial_Number;
//
//    uint32_t serial_number = MC_Request_Data(SERIAL_NUMBER_REGISTER);
//    if (serial_number != MC_Expected_Serial_Number) {
//        uvPanic("Motor Controller Serial Number Mismatch", 0);  // Call error handler directly
//    }
//}
//
//void MC_Check_Firmware(uint8_t Data[]){
//	// TODO check motor controllers firmware matches
////	uint16_t firmware_version = MC_Request_Data(FIRMWARE_VERSION_REGISTER);  // Request firmware version
////	return firmware_version == MC_Expected_FW_Version;
//
//    uint16_t firmware_version = MC_Request_Data(FIRMWARE_VERSION_REGISTER);
//    if (firmware_version != MC_Expected_FW_Version) {
//        uvPanic("Motor Controller Firmware Version Mismatch", 0);  // Call error handler directly
//    }
//}

//void MC_Startup(void* args){
//	// MC_Send_Data(...)
//	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_14);
//	uv_init_task_args* params = (uv_init_task_args*) args;
//
//	uv_init_task_response response = {UV_OK,MOTOR_CONTROLLER,0,NULL};
//	//We need to do a bunch of stuff to actually initialize the motor controller here
//
//	motor_controller_settings* settings = (motor_controller_settings*) params->specific_args;
//
//	    // Step 1: Check serial number
//	    MC_Check_Serial_Number();
//
//	    // Step 2: Check firmware version
//	    MC_Check_Firmware();
//
////	 // Step 1: Check if motor controller exists (verify serial number)
////	    if (!MC_Check_Serial_Number()) {
////	        response.status = UV_ERROR;
////	        response.error_code = MOTOR_CONTROLLER_NOT_FOUND;
////	        xQueueSendToBack(params->init_info_queue, &response, 100);
////	        uvPanic("Motor Controller not found", 0);
////	        return;
////	    }
////
////	    // Step 2: Check if motor firmware version is correct
////	    if (!MC_Check_Firmware()) {
////	        response.status = UV_ERROR;
////	        response.error_code = MOTOR_CONTROLLER_FW_MISMATCH;
////	        xQueueSendToBack(params->init_info_queue, &response, 100);
////	        uvPanic("Firmware version mismatch", 0);
////	        return;
////	    }
////
////	    // Step 3: Send a low-RPM command to spin the motor briefly to verify response
////	    uint8_t spin_command[2] = {0x00, 0x10};  // Command to set low RPM
////	    MC_Send_Data(N_set, spin_command, 2);
////	    vTaskDelay(pdMS_TO_TICKS(500));  // Non-blocking delay in RTOS to let motor spin
////
////	    // Step 4: Check for any errors after spinning the motor
////	    if (MC_Check_Error_Warning()) {
////	        response.status = UV_ERROR;
////	        response.error_code = MOTOR_CONTROLLER_ERROR;
////	        xQueueSendToBack(params->init_info_queue, &response, 100);
////	        uvPanic("Error detected after motor spin", 0);
////	        return;
////	    }
////
////	    // Step 5: Initialization successful, send confirmation response
////	    response.status = UV_OK;
////	    response.error_code = MOTOR_CONTROLLER_OK;
////	    if (xQueueSendToBack(params->init_info_queue, &response, 100) != pdPASS) {
////	        uvPanic("Failed to enqueue MC OK Response", 0);
////	    }
//
//	//Kill yourself
//	vTaskSuspend(params->meta_task_handle);
//}




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


// Motor Controller Initialization
/**
 * Initializes the motor controller by performing the following steps:
 * 1. Verifies the serial number from the motor controller.
 * 2. Checks the firmware version to ensure compatibility.
 * 3. Executes a motor spin test at low RPM to validate functionality.
 * 4. Checks for errors and warnings from the motor controller.
 * 5. Logs successful initialization if all checks pass.
 */
void MC_Startup(void* args){
	// MC_Send_Data(...)
	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_14);
	uv_init_task_args* params = (uv_init_task_args*) args;

	uv_init_task_response response = {UV_OK,MOTOR_CONTROLLER,0,NULL};
	//We need to do a bunch of stuff to actually initialize the motor controller here

	motor_controller_settings* settings = (motor_controller_settings*) params->specific_args;

	goto end_mc_init;

	MC_Request_Data(SERIAL_NUMBER_REGISTER);
	    if (!WaitFor_CAN_Response()) {
	        uvPanic("Serial Number Request Timeout", 0); // No response received
	    } else {
	        uint32_t serial_number = Parse_Bamocar_Response(RxData, 8); // Parse response
	        if (serial_number != MC_Expected_Serial_Number) {
	            uvPanic("Serial Number Mismatch", serial_number); // Serial number mismatch
	        }
	    }

	    // Step 2: Check Firmware Version
	    MC_Request_Data(FIRMWARE_VERSION_REGISTER);
	    if (!WaitFor_CAN_Response()) {
	        uvPanic("Firmware Version Request Timeout", 0); // No response received
	    } else {
	        uint16_t firmware_version = (RxData[0] << 8) | RxData[1]; // Parse firmware version
	        if (firmware_version != MC_Expected_FW_Version) {
	            uvPanic("Firmware Version Mismatch", firmware_version); // Firmware mismatch
	        }
	    }

	    // Step 3: Spin Motor Test
	    if (MotorControllerSpinTest() != 0) {
	        uvPanic("Motor Spin Test Failed", 0); // Spin test failed
	    }

	    // Step 4: Check for Errors and Warnings
	    MC_Request_Data(motor_controller_errors_warnings);
	    if (!WaitFor_CAN_Response()) {
	        uvPanic("Error/Warning Request Timeout", 0); // No response received
	    } else {
	        MotorControllerErrorHandler(RxData, 8); // Process errors and warnings
	    }






	    end_mc_init:

	if(xQueueSendToBack(params->init_info_queue,&response,100) != pdPASS){
		//OOPS
		uvPanic("Failed to enqueue MC OK Response",0);
	}





	//Kill yourself
	vTaskSuspend(params->meta_task_handle);
}




