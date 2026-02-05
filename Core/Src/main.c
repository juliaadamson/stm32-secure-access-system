/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>   // add at top of file for strlen/strcmp
#include "LCD1602.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
    STATE_LOCKED,
    STATE_UNLOCKED
} AccessState_t;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PIN_LENGTH 4

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* Definitions for AccessControl */
osThreadId_t AccessControlHandle;
const osThreadAttr_t AccessControl_attributes = {
  .name = "AccessControl",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for Button */
osThreadId_t ButtonHandle;
const osThreadAttr_t Button_attributes = {
  .name = "Button",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 128 * 4
};
/* Definitions for KeyboardTask */
osThreadId_t KeyboardTaskHandle;
const osThreadAttr_t KeyboardTask_attributes = {
  .name = "KeyboardTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 128 * 4
};
/* Definitions for ServoMotor */
osThreadId_t ServoMotorHandle;
const osThreadAttr_t ServoMotor_attributes = {
  .name = "ServoMotor",
  .priority = (osPriority_t) osPriorityHigh1,
  .stack_size = 128 * 4
};
/* USER CODE BEGIN PV */
volatile AccessState_t accessState = STATE_LOCKED;
uint8_t tx_buffer[27]="Welcome to BinaryUpdates!\n\r";
uint8_t rx_idx;
uint8_t rx_data[1];
uint8_t rx_buffer[100];
uint8_t transfer_cplt;

volatile uint8_t pin_ready = 0;
char pin_entered[PIN_LENGTH + 1] = {0};



// Keypad character map corresponding to your layout
const char keypad_map[4][4] = {
    {'1','4','7','*'},
    {'2','5','8','0'},
    {'3','6','9','#'},
    {'A','B','C','D'}
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM1_Init(void);
void AccessControlTask(void *argument);
void ButtonTask(void *argument);
void KeyboardInput(void *argument);
void ServoTask(void *argument);

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




  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

  HAL_UART_Receive_IT(&huart2, rx_data, 1);

  HAL_TIM_Base_Start(&htim1);

  lcd_init();
  lcd_put_cur(0 ,0);
  lcd_send_string("HELLO");

  lcd_put_cur(1,0);
  lcd_send_string("HELP");
  HAL_Delay(3000);
  lcd_clear();

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  HAL_UART_Receive_IT(&huart2, rx_data, 1);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of AccessControl */
  AccessControlHandle = osThreadNew(AccessControlTask, NULL, &AccessControl_attributes);

  /* creation of Button */
  ButtonHandle = osThreadNew(ButtonTask, NULL, &Button_attributes);

  /* creation of KeyboardTask */
  KeyboardTaskHandle = osThreadNew(KeyboardInput, NULL, &KeyboardTask_attributes);

  /* creation of ServoMotor */
  ServoMotorHandle = osThreadNew(ServoTask, NULL, &ServoMotor_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 170-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 340-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_5|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA5 PA7 PA8
                           PA9 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_5|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC4 PC5 PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB3 PB4 PB5
                           PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */



/**
  * @brief  Scans the 4x4 keypad to find which key is pressed.
  * @retval The character of the pressed key, or '\0' (null character) if no key is pressed.
  */
char scan_keypad(void)
{
    // Array of row pins for easier iteration
    GPIO_TypeDef* row_ports[] = {GPIOA, GPIOC, GPIOB, GPIOA};
    uint16_t row_pins[] = {GPIO_PIN_9, GPIO_PIN_7, GPIO_PIN_6, GPIO_PIN_7};

    for (int row = 0; row < 4; row++)
    {
        // Drive all rows HIGH first
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);

        // Drive the current row LOW
        HAL_GPIO_WritePin(row_ports[row], row_pins[row], GPIO_PIN_RESET);

        // Check columns for a LOW signal.
        // A LOW signal means a key in this row has been pressed.
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET) return keypad_map[row][0];
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET) return keypad_map[row][1];
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_RESET) return keypad_map[row][2];
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == GPIO_PIN_RESET) return keypad_map[row][3];
    }

    // If we get here, no key was pressed
    return 99;
}


void PrintMenu(void)
{
    const char *menu =
        "\r\n=== Secure Access System ===\r\n"
        "1 - Unlock door\r\n"
        "2 - Lock door\r\n"
        "3 - Status\r\n"
        "> ";

    HAL_UART_Transmit(&huart2, (uint8_t*)menu, strlen(menu), HAL_MAX_DELAY);
}

void ClearScreen(void)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)"\033[2J\033[H", 7, HAL_MAX_DELAY);
}





void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_RxCpltCallback can be implemented in the user file.
   */

  //uint8_t i;

  if (huart->Instance == USART2)
  {
      // Ignore LF (10)
      if (rx_data[0] == '\n')
      {
          // do nothing
      }
      // ENTER pressed (CR)
      else if (rx_data[0] == '\r')
      {
          rx_buffer[rx_idx] = '\0';
          rx_idx = 0;
          transfer_cplt = 1;

          HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
      }
      else
      {
          if (rx_idx < sizeof(rx_buffer) - 1)
          {
              rx_buffer[rx_idx++] = rx_data[0];
              HAL_UART_Transmit(&huart2, rx_data, 1, HAL_MAX_DELAY); // echo
          }
      }

      HAL_UART_Receive_IT(&huart2, rx_data, 1);
  }
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_AccessControlTask */
/**
  * @brief  Function implementing the AccessControl thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_AccessControlTask */
void AccessControlTask(void *argument)
{
  /* USER CODE BEGIN 5 */
    // Start in LOCKED state (choose your default)
    accessState = STATE_LOCKED;

    // Set LEDs to match default state
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);   // Red ON
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);  // Green OFF

    ClearScreen();

    // Print menu once at startup
    PrintMenu();

  /* Infinite loop */
    for (;;)
    {
    	if (pin_ready)
    	{
    	    pin_ready = 0;

    	    // For now just print it (later we'll validate)
    	    char msg[64];
    	    snprintf(msg, sizeof(msg), "\r\nGot PIN: %s\r\n", pin_entered);
    	    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    	    // Next: check if it matches allowed PINs, then unlock/lock
    	}

        // transfer_cplt is set to 1 in HAL_UART_RxCpltCallback() when user presses ENTER
        if (transfer_cplt)
        {
            transfer_cplt = 0;

            // User chose option 1: UNLOCK
            if (strcmp((char*)rx_buffer, "1") == 0)
            {
                accessState = STATE_UNLOCKED;

                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);     // Green ON
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);  // Red OFF

                HAL_UART_Transmit(&huart2, (uint8_t*)"Access granted\r\n", 16, HAL_MAX_DELAY);

            }
            // User chose option 2: LOCK
            else if (strcmp((char*)rx_buffer, "2") == 0)
            {
                accessState = STATE_LOCKED;

                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);    // Red ON
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);   // Green OFF

                HAL_UART_Transmit(&huart2, (uint8_t*)"Access refused\r\n", 16, HAL_MAX_DELAY);

            }
            // User chose option 3: STATUS
            else if (strcmp((char*)rx_buffer, "3") == 0)
            {
                if (accessState == STATE_LOCKED)
                {
                    HAL_UART_Transmit(&huart2, (uint8_t*)"STATUS: LOCKED\r\n",
                                      strlen("STATUS: LOCKED\r\n"),
                                      HAL_MAX_DELAY);
                }
                else
                {
                    HAL_UART_Transmit(&huart2, (uint8_t*)"STATUS: UNLOCKED\r\n",
                                      strlen("STATUS: UNLOCKED\r\n"),
                                      HAL_MAX_DELAY);
                }
            }

            else
            {
            	HAL_UART_Transmit(&huart2, (uint8_t*)"Invalid option. Type 1, 2, or 3 then press ENTER.\r\n", 16, HAL_MAX_DELAY);
            }

            // Re-print menu after handling command
            osDelay(3000);
            ClearScreen();
            PrintMenu();
        }
        osDelay(10);
    }

  // In case we exit from task loop
  osThreadTerminate(NULL);
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_ButtonTask */
/**
* @brief Function implementing the Button thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ButtonTask */
void ButtonTask(void *argument)
{
  /* USER CODE BEGIN ButtonTask */

  /* Infinite loop */
  for(;;)
  {
	  if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
	  {
		  osDelay(30);

		  if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
		  {
			  printf("Unlocked\n");

			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);

		      osDelay(1000);

			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

			  while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
			  {
				  osDelay(10);
			  }
			  osDelay(30);
		  }

	  }
	  osDelay(10);

  }
  osThreadTerminate(NULL);
  /* USER CODE END ButtonTask */
}

/* USER CODE BEGIN Header_KeyboardInput */
/**
* @brief Function implementing the KeyboardTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_KeyboardInput */
void KeyboardInput(void *argument)
{
  /* USER CODE BEGIN KeyboardInput */

	char pin[PIN_LENGTH + 1] = {0};
	uint8_t idx = 0;

    const char *banner = "Enter 4-digit PIN on keypad. #=Enter, *=Clear\r\n> ";
    HAL_UART_Transmit(&huart2, (uint8_t*)banner, strlen(banner), HAL_MAX_DELAY);

  /* Infinite loop */
    for (;;)
    {

        char key = scan_keypad();

        if (key != 99)  // 99 = no key pressed (your function)
        {
            if (key >= '0' && key <= '9')
            {
                if (idx < PIN_LENGTH)
                {
                    pin[idx++] = key;
                    pin[idx] = '\0';

                    // mask output
                    HAL_UART_Transmit(&huart2, (uint8_t*)"*", 1, HAL_MAX_DELAY);
                }
            }
            else if (key == '*')
            {
                idx = 0;
                memset(pin, 0, sizeof(pin));
                HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nCleared\r\n> ",
                                  strlen("\r\nCleared\r\n> "),
                                  HAL_MAX_DELAY);
            }
            else if (key == '#')
            {
                if (idx == PIN_LENGTH)
                {
                    // Only write the shared globals when we're ready to submit
                    strcpy(pin_entered, pin);
                    pin_ready = 1;

                    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nPIN submitted\r\n> ",
                                      strlen("\r\nPIN submitted\r\n> "),
                                      HAL_MAX_DELAY);
                }
                else
                {
                    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nNeed 4 digits\r\n> ",
                                      strlen("\r\nNeed 4 digits\r\n> "),
                                      HAL_MAX_DELAY);
                }

                // reset local entry buffer
                idx = 0;
                memset(pin, 0, sizeof(pin));
            }

            osDelay(250); // debounce / prevents repeats
        }

        osDelay(20);
    }
  /* USER CODE END KeyboardInput */
}

/* USER CODE BEGIN Header_ServoTask */
/**
* @brief Function implementing the ServoMotor thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ServoTask */
void ServoTask(void *argument)
{
  /* USER CODE BEGIN ServoTask */
  /* Infinite loop */
  for(;;)
  {
      __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 250);
      osDelay(1000);
      __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 750);
      osDelay(1000);
      __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 1250);
      osDelay(1000);


  }
  /* USER CODE END ServoTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */


  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
