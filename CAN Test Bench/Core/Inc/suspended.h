/*
 * suspended.h
 *
 *  Created on: Mar 12, 2024
 *      Author: byo10
 */

#ifndef INC_SUSPENDED_H_
#define INC_SUSPENDED_H_

#define _MAX_SUSPENSION_TIME_BEFORE_SHUTDOWN_ 500 //ms

void _add_condition(uint8_t (*cond)());
void suspended_loop();


#endif /* INC_SUSPENDED_H_ */
