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
#include "cmsis_os.h"
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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint32_t adc_buf1[ADC1_BUF_LEN]; // ADC1 - high priority readings

uint16_t adc1_APPS1; //These are the locations for the sensor inputs for APPS and BPS
uint16_t adc1_APPS2;
uint16_t adc1_BPS1;
uint16_t adc1_BPS2;

volatile uint32_t adc_buf2[ADC2_BUF_LEN]; // ADC2 - lower priority readings
uint16_t adc2_CoolantTemp;
uint16_t adc2_CoolantFlow;

//int adc_conv_complete_flag = 0;


//State of da vehicle
enum uv_vehicle_state_t vehicle_state = init; //TODO: Define new state machine logic

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf1, ADC1_BUF_LEN);
  //HAL_TIM_Base_Start_IT(&htim3); This is getting disabled, since measuring temp will now be an RTOS task
  HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_14); // Just to prove we reached this point in the code

  //ANYTHING WE NEED TO DO BEFORE THE KERNEL TAKES OVER SHOULD HAPPEN HERE:

  /* Step 1: Initialize whatever else needs initializing. Check that all devices exist.
   * Do some diagnostics on ourselves while we at it.
   */

  MC_Startup(); // Send all the needed CAN messages to the MC for it to be properly configured.

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

    //Update_Batt_Temp(69); // temp debugging


  while (1) //we should deadass never reach this point in the code lol
  {
	  // For debugging purposes
	  HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_13);


	  // into string and store in dma_result_buffer character array
	  Update_RPM(adc1_APPS1);
	  HAL_Delay(1000);
	  Update_RPM(adc1_APPS2);
	  HAL_Delay(1000);




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
// called when timer 3 period elapsed
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
//
//	// check that timer 3 created the interrupt
//	if (htim == &htim3){
//
//		// start a single round of ADC2 conversions
//		HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc_buf2, ADC2_BUF_LEN);
//
//		// restart timer
//		HAL_TIM_Base_Start_IT(&htim3);
//	}
//}


// Called when adc dma buffer is completely filled
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
  // Could toggle an LED here
	if(hadc->Instance == ADC1){ //TODO: Fix this so it is RTOS compatible
		adc1_APPS1 = 0;
		adc1_APPS2 = 0;
		adc1_BPS1 = 0;
		adc1_BPS2 = 0;

		//Hmmm this should honestly probably be a 16 bit type but aint nobody got time for dat
		uint32_t* buf1_pointer = adc_buf1;

		for(int i = 0; i < ADC1_SAMPLES; i++){ //BUG: pointer incremented by one rather than the length of the datatype
			adc1_APPS1 += *buf1_pointer++;
			adc1_APPS2 += *buf1_pointer++;
			adc1_BPS1 += *buf1_pointer++;
			adc1_BPS2 += *buf1_pointer++;
		}

		//The above code is actually even worse than I originally suspected. Seriously, WTF. If multiple writes occur, then they get summed,
		//Rather than initial writes getting overwritten by newer ones. UNDEFINED BEHAVIOR!

		// calculate APPS fraction travelled (see spreadsheet for equation derivation)
		// Byron here: I want these constants to be TUNABLE, this will need some debugging in the future
		adc1_APPS1 = 0.8082*adc1_APPS1 - 0.3709;
		adc1_APPS2 = 0.8405*adc1_APPS2 - 0.8747;

		// APPS1 & APPS2 plausibility check
			// suspend if error persists for 100 ms
		if((adc1_APPS1-adc1_APPS2)>0.1 || (adc1_APPS1-adc1_APPS2)<-0.1){
			//START TIMER FOR APPS IMPLAUSIBILITY
		}

		// APPS & BPS plausibility check
			// immediate suspend if error occurs
		if(((adc1_APPS1 > 0.25)||(adc1_APPS2 > 0.25))&&((adc1_BPS1 > 69)||(adc1_BPS2 > 69))){
			// checking for hard brake + accel
			// if found, enter suspend
		}
	}
	if (hadc->Instance == ADC2){
		adc2_CoolantTemp = adc_buf2[0];
		adc2_CoolantFlow = adc_buf2[1];
	}
}

// EXTI gpio pin a0 External Interrupt ISR Handler
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_0) // If The INT Source Is EXTI Line0 (A0 Pin)
    {
//ready_to_drive = 1;
    }
}

// Analog Watchdog Out-of-Range handler, ADC conversion values range from 0 to 4095
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc){
	if(hadc->Instance == ADC1){
		// triggered for voltages below 0.5V (below 400) or above 4.5V (above 3674)
		HAL_GPIO_WritePin(Red_LED_GPIO_Port,Red_LED_Pin, GPIO_PIN_SET);
		HAL_ADC_Stop_DMA(&hadc1);
		adc1_APPS1 = 0;
		adc1_APPS2 = 0;
	}
	if(hadc->Instance == ADC2){
		// handle coolant sensors short error
	}
}


/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  if (htim == &htim3){

  		// start a single round of ADC2 conversions
  		HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc_buf2, ADC2_BUF_LEN);

  		// restart timer
  		HAL_TIM_Base_Start_IT(&htim3);
  	}
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_12); //Me when the error is not being handled
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
