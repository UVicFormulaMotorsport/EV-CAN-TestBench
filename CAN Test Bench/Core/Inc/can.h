/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
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
/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan2;

/* USER CODE BEGIN Private defines */

#define CAN_TX_DAEMON_NAME "CanTxDaemon"
#define CAN_RX_DAEMON_NAME "CanRxDaemon"

/* USER CODE END Private defines */

void MX_CAN2_Init(void);

/* USER CODE BEGIN Prototypes */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan2);
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan2);

int call_function_from_CAN_id(uint32_t CAN_id, uint8_t* data, uint8_t length);
void insert_CAN_message(CAN_Message message);
void nuke_hash_table();

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

