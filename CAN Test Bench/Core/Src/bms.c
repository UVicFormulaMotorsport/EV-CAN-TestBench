// This where the code to handle BMS errors and such will go

#include "main.h"
#include "bms.h"
#include "constants.h"
#include "pdu.h"
#include "can.h"
#include "tim.h"
#include "dash.h"

//BMS State Vars
uint8_t BMS_discrete_inputs_1;
uint32_t BMS_discrete_inputs_2;
uint32_t BMS_internal_state;
uint32_t BMS_errors_1;
uint32_t BMS_errors_2;



//Battery Pack Info Vars
int16_t BMS_battery_current;
int8_t min_battery_temp;
int8_t max_battery_temp;
uint8_t battery_SOC;//state of charge
uint16_t BMS_battery_voltage;




// TODO
void BMS_Parse_Message1(int DLC, uint8_t Data[]){
	//step 1: translate message into useful data
	BMS_discrete_inputs_1 = BMS_MSG1_DISCRETE_INPUTS_1(Data);
	BMS_battery_current = BMS_MSG1_BATTERY_CURRENT(Data);
	min_battery_temp = BMS_MSG1_MIN_BATTERY_TEMP(Data);
	max_battery_temp = BMS_MSG1_MAX_BATTERY_TEMP(Data);
	battery_SOC = BMS_MSG1_SOC(Data);
	BMS_battery_voltage = BMS_MSG1_BATTERY_VOLTAGE(Data);

	//step 2: do some checks to make sure everything makes sense
	if(BMS_battery_voltage < MIN_BATTERY_VOLTAGE){ //under voltage

	} else if(BMS_battery_voltage > MAX_BATTERY_VOLTAGE){ //over voltage

	}

	if(min_battery_temp < 0){ //too cold to start

	}

	if(max_battery_temp > 60){ //too hot

	}

	Update_Batt_Temp(max_battery_temp);
	Update_State_Of_Charge(battery_SOC);

//	if(battery_SOC > 95){
//		//disable regen
//	}else{
//		//re-enable regen
//	}



}

void BMS_Parse_Message2(int DLC, uint8_t Data[]){
	BMS_internal_state = BMS_MSG2_BMS_INTERNAL_STATE(Data);
	BMS_errors_1 = BMS_MSG2_ERRORS_REGISTER_1(Data);

	//check the errors
	uint32_t errcheck = 0x01U;
	for (int i = 0; i<30; i++){

		if(BMS_errors_1 & (errcheck)){
			//_throw_error(BMS_Overcurrent + i); //This is for when we actually get logging and differentiation between error types
			_throw_error(default_shutdown);

		}
		errcheck = errcheck << 1;
	}

	if (BMS_errors_1 & BMS_ERRORS1_RESERVED_MASK){
		_throw_error(default_shutdown);
	}

	if(BMS_internal_state & BMS_ILLEGAL_STATES){//illegal state for vehicle

	}
}

void BMS_Parse_Message3(int DLC, uint8_t Data[]){
	BMS_errors_2 = BMS_MSG3_ERRORS_REGISTER_2(Data);
	BMS_discrete_inputs_2 = BMS_MSG3_DISCRETE_INPUTS_2(Data);

	uint32_t errcheck = 0x01U;
		for (int i = 0; i<30; i++){

		if(BMS_errors_1 & (errcheck)){
			_throw_error(BMS_Overcurrent + i);

		}
		errcheck = errcheck << 1;
	}


}

void reset_BMS_WDT(){

}





