/* USER CODE BEGIN Header */
///**
//  ******************************************************************************
//  * @file    can.h
//  * @brief   This file contains all the function prototypes for
//  *          the can.c file
//  ******************************************************************************
//  * @attention
//  *
//  * Copyright (c) 2023 STMicroelectronics.
//  * All rights reserved.
//  *
//  * This software is licensed under terms that can be found in the LICENSE file
//  * in the root directory of this software component.
//  * If no LICENSE file comes with this software, it is provided AS-IS.
//  *
//  ******************************************************************************
//  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "constants.h"
#include "uvfr_utils.h"
/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan2;

/* USER CODE BEGIN Private defines */

#define CAN_TX_DAEMON_NAME "CanTxDaemon"
#define CAN_RX_DAEMON_NAME "CanRxDaemon"

typedef struct uv_CAN_msg uv_CAN_msg;
typedef enum uv_status_t uv_status;

/* USER CODE END Private defines */

void MX_CAN2_Init(void);

/* USER CODE BEGIN Prototypes */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan2);
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan2);

uv_status uvSendCanMSG(uv_CAN_msg * msg);

void CANbusTxSvcDaemon(void* args);
void CANbusRxSvcDaemon(void* args);

//int callFunctionFromCANid(uint32_t CAN_id, uint8_t* data, uint8_t length);
void insertCANMessageHandler(uint32_t id, void* handlerfunc);
void nuke_hash_table();
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

