/*
 * oled.h
 *
 *  Created on: Nov 12, 2024
 *      Author: byo10
 */

#ifndef _OLED_H_
#define _OLED_H_

#include "uvfr_utils.h"

#ifdef USE_OLED_DEBUG

void wait(uint32_t t);
void refresh_OLED(volatile unsigned int Freq, volatile unsigned int Res );
void oled_Write_Cmd(unsigned char);
void oled_Write_Data(unsigned char);
void oled_Write(unsigned char);
void oled_config(void);

void amogus(void);

#endif

#endif

