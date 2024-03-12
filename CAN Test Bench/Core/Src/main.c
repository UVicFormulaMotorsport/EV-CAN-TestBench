/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "can.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

// The CAN info is defined in constants

#include "constants.h"
#include "bms.h"
#include "dash.h"
#include "imd.h"
#include "motor_controller.h"
#include "pdu.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// 12 bit ADC resolution
#define ADC_BUF_LEN 2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint32_t adc_buf1[ADC_BUF_LEN]; // ADC1
uint16_t adc1_1;
uint16_t adc1_2;

volatile uint32_t adc_buf2[ADC_BUF_LEN]; // ADC2
uint16_t adc2_1;
uint16_t adc2_2;

//int adc_conv_complete_flag = 0;
int ready_to_drive = 0;
int outofrange = 0;
int hardbreak = 0;

uint8_t vehicle_state_changed = 0;
enum vehicle_state_t vehicle_state = init;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Set_Vehicle_State(enum vehicle_state_t newstate){
	//used to set vehicle state
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  // The CAN clock must be enabled
  __HAL_RCC_CAN2_CLK_ENABLE();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CAN2_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_TIM13_Init();
  MX_TIM14_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf1, ADC_BUF_LEN);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

    Update_Batt_Temp(69); // temp debugging

    // This is the waiting to drive loop
    // The driver needs to press the brake and the start button before driving the car
    while (!ready_to_drive){
    	// we can check miscellaneous stuff here
    	// TODO - incorporate brake pedal input

    	// The ISR at the bottom will get triggered when the button is pressed
    	// That will set the ready to drive flag to 1 and this loop will exit
    }
    // After the car is ready to drive, we want to play the speaker chirp, then we can drive
    PDU_speaker_chirp();

  while (1)
  {
	  // For debugging purposes
	  HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_13);


	  // into string and store in dma_result_buffer character array
	  Update_RPM(adc1_1);
	  HAL_Delay(1000);
	  Update_RPM(adc1_2);
	  HAL_Delay(1000);



	  // The goal is to set some timers that will trigger interrupts for
	  // less important tasks

	  // Something like every second we check temperatures of things
	  // Checking ADC inputs could be in the loop
	  // Checking status of devices could be in an ISR for a timer

	  switch(vehicle_state){
	  case init:


	  break;
	  case ready:

		 break;
	  case suspended:

		  break;
	  case error:

		  break;
	  case limp: // aw hell naw, the car gone flacid

		  break;
	  default://physically how  are you here?

		  break;

	  }


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

// Called when adc dma buffer is completely filled
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
  // Could toggle an LED here
	adc1_1 = adc_buf1[0];
	adc1_2 = adc_buf1[1];
	if(adc1_1 > 3000){
		hardbreak = 1;
	}else{
		hardbreak = 0;
	}
}

// EXTI gpio pin a0 External Interrupt ISR Handler
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_0) // If The INT Source Is EXTI Line0 (A0 Pin)
    {
    	ready_to_drive = 1;
    }
}

// Analog Watchdog Out-of-Range handler
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc){
	HAL_GPIO_WritePin(Red_LED_GPIO_Port,Red_LED_Pin, GPIO_PIN_SET);
	HAL_ADC_Stop_DMA(&hadc1);
	adc1_1 = 0;
	adc1_2 = 0;
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_12);
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
