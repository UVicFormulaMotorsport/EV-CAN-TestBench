// This where the code to handle BMS errors and such will go

#include "main.c"
#include "bms.h"
#include "constants.h"
#include "pdu.h"
#include "can.h"
#include "tim.h"

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
	BMS_Battery_current = BMS_MSG1_BATTERY_CURRENT(Data);
	min_battery_temp = BMS_MSG1_MIN_BATTERY_TEMP(Data);
	max_battery_temp = BMS_MSG1_MAX_BATTERY_TEMP(Data);
	battery_SOC = BMS_MSG1_SOC(Data);
	BMS_battery_voltage = BMS_MSG1_BATTERY_VOLTAGE(Data);

	//step 2: do some checks to make sure everything makes sense
	if(BMS_battery_voltage < MIN_BATTERY_VOLTAGE){ //under voltage

	} else if(BMS_battery_voltage > MAX_BATTERY_VOLTAGE){ //over voltage

	}

	if(min_battery_temp < 0){ //to cold to start

	}

	if(max_battery_temp > 60){ //too hot

	}

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
	if(BMS_errors_1 & BMS_ERRORS1_VEHICLE_SHTDWN_mask){//ruh roh scoobs, time to shut down the car

	}

	if(BMS_internal_state & BMS_ILLEGAL_STATES){

	}
}

void BMS_Parse_Message3(int DLC, uint8_t Data[]){
	BMS_errors_2 = BMS_MSG3_ERRORS_REGISTER_2(msg);
	BMS_discrete_inputs_2 = BMS_MSG3_DISCRETE_INPUTS_2(msg);

	if(BMS_errors_2 & BMS_ERRORS2_VEHICLE_SHTDWN_mask){//ruh roh scoobs, time to shut down the car

	}
}

void reset_BMS_WDT(){

}





