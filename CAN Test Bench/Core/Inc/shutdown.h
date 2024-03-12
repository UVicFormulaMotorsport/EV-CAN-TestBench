/*
 * shutdown.h
 *
 *  Created on: Mar 6, 2024
 *      Author: Surface
 */

#ifndef INC_SHUTDOWN_H_
#define INC_SHUTDOWN_H_


void _Shutdown();
void _Suspend(uint8_t (*cond)());





void Trigger_Shutdown_Circuit();
void Disable_Motor_Controller();
void Limp_Mode();






#endif /* INC_SHUTDOWN_H_ */
