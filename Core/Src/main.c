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
#include <stdint.h>

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PIN_LENGTH 4
#define MAX_NAME_LEN 16
#define MAX_USERS 10
#define LOCKOUT_MS 10000   // 10 seconds

/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
    STATE_LOCKED,
    STATE_UNLOCKED
} AccessState_t;

typedef struct {
    char pin[PIN_LENGTH + 1];          // "1234"
    char name[MAX_NAME_LEN + 1];       // "Julia"
    uint8_t active;                    // 1 = slot used
} UserAccount_t;
typedef enum
{
    UART_MODE_MENU,
    UART_MODE_ADD_NAME,
    UART_MODE_ADD_PIN
} UartMode_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define PIN_LENGTH 4

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
/* Definitions for LCDtask */
osThreadId_t LCDtaskHandle;
const osThreadAttr_t LCDtask_attributes = {
  .name = "LCDtask",
  .priority = (osPriority_t) osPriorityHigh,
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

volatile uint8_t enteringPin = 0;
volatile uint8_t failedAttempts = 0;
volatile uint8_t lockoutActive = 0;
uint32_t lockoutEndTick = 0;

volatile UartMode_t uartMode = UART_MODE_MENU;
char newUserName[MAX_NAME_LEN + 1] = {0};



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
void lcdTask(void *argument);

static UserAccount_t userDb[MAX_USERS];
static int Db_AddUser(const char *name, const char *pin);
static void UART_ShowUsers(void);
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

  /* creation of LCDtask */
  LCDtaskHandle = osThreadNew(lcdTask, NULL, &LCDtask_attributes);

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
static int Db_AddUser(const char *name, const char *pin)
{
    // check for duplicate PIN
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (userDb[i].active && strcmp(userDb[i].pin, pin) == 0)
        {
            return -2;   // duplicate PIN
        }
    }

    // find empty slot
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (!userDb[i].active)
        {
            strncpy(userDb[i].name, name, MAX_NAME_LEN);
            userDb[i].name[MAX_NAME_LEN] = '\0';

            strncpy(userDb[i].pin, pin, PIN_LENGTH);
            userDb[i].pin[PIN_LENGTH] = '\0';

            userDb[i].active = 1;
            return i;
        }
    }

    return -1;   // no free slot
}

static void UART_ShowUsers(void)
{
    char line[64];

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"\r\n--- User List ---\r\n",
                      strlen("\r\n--- User List ---\r\n"),
                      HAL_MAX_DELAY);

    for (int i = 0; i < MAX_USERS; i++)
    {
        if (userDb[i].active)
        {
            snprintf(line, sizeof(line), "%d. %s - PIN: %s\r\n",
                     i + 1, userDb[i].name, userDb[i].pin);
            HAL_UART_Transmit(&huart2, (uint8_t*)line, strlen(line), HAL_MAX_DELAY);
        }
    }
}

static void LCD_ShowMaskedPin(uint8_t count)
{
    char stars[PIN_LENGTH + 1] = {0};

    for (uint8_t i = 0; i < count && i < PIN_LENGTH; i++)
    {
        stars[i] = '*';
    }

    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("ENTER PIN:");

    lcd_put_cur(1, 0);
    lcd_send_string("                ");
    lcd_put_cur(1, 0);
    lcd_send_string(stars);
}

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
        "\r\n--- Secure Access System ---\r\n\n"
        "1 - Unlock door\r\n"
        "2 - Lock door\r\n"
        "3 - Status\r\n"
        "4 - Add user\r\n"
        "5 - View users\r\n"
        "> ";

    HAL_UART_Transmit(&huart2, (uint8_t*)menu, strlen(menu), HAL_MAX_DELAY);
}

void ClearScreen(void)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)"\033[2J\033[H", 7, HAL_MAX_DELAY);
}

static void LCD_ShowState(AccessState_t state)
{
    lcd_clear();
    lcd_put_cur(0, 0);

    if (state == STATE_LOCKED)
    {
    	lcd_clear();
    	lcd_put_cur(0, 0);
        lcd_send_string(" ACCESS  SYSTEM");
        lcd_put_cur(1, 0);
        lcd_send_string("     LOCKED");
    }
    else
    {
    	lcd_clear();
    	lcd_put_cur(0, 0);
        lcd_send_string(" ACCESS  SYSTEM  ");
        lcd_put_cur(1, 0);
        lcd_send_string("    UNLOCKED");
    }
}

static void Db_Init(void)
{
    memset(userDb, 0, sizeof(userDb));

    strcpy(userDb[0].pin, "4567");
    strcpy(userDb[0].name, "Julia");
    userDb[0].active = 1;

    strcpy(userDb[1].pin, "4444");
    strcpy(userDb[1].name, "Guest");
    userDb[1].active = 1;

    strcpy(userDb[2].pin, "1111");
    strcpy(userDb[2].name, "Admin");
    userDb[2].active = 1;

    strcpy(userDb[3].pin, "9642");
    strcpy(userDb[3].name, "Valentino");
    userDb[3].active = 1;
}

static int Db_FindByPin(const char *pin)
{
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (userDb[i].active && strcmp(userDb[i].pin, pin) == 0)
            return i;
    }
    return -1;
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
    accessState = STATE_LOCKED;

    lcd_init();
    lcd_clear();
    Db_Init();

    // Set LEDs to match default state
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);    // Red ON
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);  // Green OFF

    ClearScreen();
    PrintMenu();

    uint32_t lastRemaining = 999;

    for (;;)
    {
        // Handle lockout countdown
        if (lockoutActive)
        {
            uint32_t now = HAL_GetTick();

            if (now >= lockoutEndTick)
            {
                lockoutActive = 0;
                failedAttempts = 0;
                lastRemaining = 999;
                enteringPin = 0;

                lcd_clear();
                lcd_put_cur(0, 0);
                lcd_send_string("LOCKOUT ENDED");
                lcd_put_cur(1, 0);
                lcd_send_string("Try again");

                HAL_UART_Transmit(&huart2,
                                  (uint8_t*)"\r\nLockout ended\r\n",
                                  strlen("\r\nLockout ended\r\n"),
                                  HAL_MAX_DELAY);

                osDelay(1500);
                LCD_ShowState(accessState);
                PrintMenu();
            }
            else
            {
                uint32_t remaining = (lockoutEndTick - now + 999) / 1000;

                if (remaining != lastRemaining)
                {
                    lastRemaining = remaining;

                    lcd_clear();
                    lcd_put_cur(0, 0);
                    lcd_send_string("LOCKED OUT");

                    char line[17];
                    snprintf(line, sizeof(line), "Wait %lus", remaining);
                    lcd_put_cur(1, 0);
                    lcd_send_string("                ");
                    lcd_put_cur(1, 0);
                    lcd_send_string(line);
                }

                osDelay(100);
                continue;
            }
        }

        // Handle submitted keypad PIN
        if (pin_ready)
        {
            pin_ready = 0;
            enteringPin = 0;

            int userIdx = Db_FindByPin(pin_entered);

            if (userIdx >= 0)
            {
                failedAttempts = 0;
                accessState = STATE_UNLOCKED;

                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);     // Green ON
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);   // Red OFF

                lcd_clear();
                lcd_put_cur(0, 0);
                lcd_send_string("ACCESS GRANTED");
                lcd_put_cur(1, 0);
                lcd_send_string(userDb[userIdx].name);

                char msg[64];
                snprintf(msg, sizeof(msg), "\r\nWelcome %s\r\n", userDb[userIdx].name);
                HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

                osDelay(1500);
                LCD_ShowState(accessState);
                PrintMenu();
            }
            else
            {
                accessState = STATE_LOCKED;
                failedAttempts++;

                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);     // Red ON
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);   // Green OFF

                if (failedAttempts >= 3)
                {
                    lockoutActive = 1;
                    lockoutEndTick = HAL_GetTick() + LOCKOUT_MS;
                    lastRemaining = 999;

                    lcd_clear();
                    lcd_put_cur(0, 0);
                    lcd_send_string("LOCKED OUT");
                    lcd_put_cur(1, 0);
                    lcd_send_string("Wait 10s");

                    HAL_UART_Transmit(&huart2,
                                      (uint8_t*)"\r\nToo many failed attempts. System locked for 10 seconds.\r\n",
                                      strlen("\r\nToo many failed attempts. System locked for 10 seconds.\r\n"),
                                      HAL_MAX_DELAY);

                    osDelay(1000);
                }
                else
                {
                    lcd_clear();
                    lcd_put_cur(0, 0);
                    lcd_send_string("ACCESS DENIED");

                    char line[17];
                    snprintf(line, sizeof(line), "Attempt %d of 3", failedAttempts);
                    lcd_put_cur(1, 0);
                    lcd_send_string("                ");
                    lcd_put_cur(1, 0);
                    lcd_send_string(line);

                    char msg[64];
                    snprintf(msg, sizeof(msg), "\r\nAccess denied (%d/3)\r\n", failedAttempts);
                    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

                    osDelay(1500);
                    LCD_ShowState(accessState);
                    PrintMenu();
                }
            }
        }

        if (transfer_cplt)
        {
            transfer_cplt = 0;

            if (uartMode == UART_MODE_MENU)
            {
                if (strcmp((char*)rx_buffer, "1") == 0)
                {
                    accessState = STATE_UNLOCKED;

                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);

                    HAL_UART_Transmit(&huart2,
                                      (uint8_t*)"Access granted\r\n",
                                      strlen("Access granted\r\n"),
                                      HAL_MAX_DELAY);

                    LCD_ShowState(accessState);
                    osDelay(1000);
                    ClearScreen();
                    PrintMenu();
                }
                else if (strcmp((char*)rx_buffer, "2") == 0)
                {
                    accessState = STATE_LOCKED;

                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

                    HAL_UART_Transmit(&huart2,
                                      (uint8_t*)"Access refused\r\n",
                                      strlen("Access refused\r\n"),
                                      HAL_MAX_DELAY);

                    LCD_ShowState(accessState);
                    osDelay(1000);
                    ClearScreen();
                    PrintMenu();
                }
                else if (strcmp((char*)rx_buffer, "3") == 0)
                {
                    if (accessState == STATE_LOCKED)
                    {
                        HAL_UART_Transmit(&huart2,
                                          (uint8_t*)"STATUS: LOCKED\r\n",
                                          strlen("STATUS: LOCKED\r\n"),
                                          HAL_MAX_DELAY);
                    }
                    else
                    {
                        HAL_UART_Transmit(&huart2,
                                          (uint8_t*)"STATUS: UNLOCKED\r\n",
                                          strlen("STATUS: UNLOCKED\r\n"),
                                          HAL_MAX_DELAY);
                    }

                    PrintMenu();
                }
                else if (strcmp((char*)rx_buffer, "4") == 0)
                {
                    uartMode = UART_MODE_ADD_NAME;

                    HAL_UART_Transmit(&huart2,
                                      (uint8_t*)"\r\nEnter new user name: ",
                                      strlen("\r\nEnter new user name: "),
                                      HAL_MAX_DELAY);
                }
                else if (strcmp((char*)rx_buffer, "5") == 0)
                {
                    UART_ShowUsers();
                    PrintMenu();
                }
                else
                {
                    HAL_UART_Transmit(&huart2,
                                      (uint8_t*)"Invalid option. Type 1, 2, 3, 4, or 5 then press ENTER.\r\n",
                                      strlen("Invalid option. Type 1, 2, 3, 4, or 5 then press ENTER.\r\n"),
                                      HAL_MAX_DELAY);
                    PrintMenu();
                }
            }
            else if (uartMode == UART_MODE_ADD_NAME)
            {
                strncpy(newUserName, (char*)rx_buffer, MAX_NAME_LEN);
                newUserName[MAX_NAME_LEN] = '\0';

                uartMode = UART_MODE_ADD_PIN;

                HAL_UART_Transmit(&huart2,
                                  (uint8_t*)"Enter 4-digit PIN: ",
                                  strlen("Enter 4-digit PIN: "),
                                  HAL_MAX_DELAY);
            }
            else if (uartMode == UART_MODE_ADD_PIN)
            {
                if (strlen((char*)rx_buffer) != PIN_LENGTH)
                {
                    HAL_UART_Transmit(&huart2,
                                      (uint8_t*)"\r\nPIN must be exactly 4 digits.\r\nEnter 4-digit PIN: ",
                                      strlen("\r\nPIN must be exactly 4 digits.\r\nEnter 4-digit PIN: "),
                                      HAL_MAX_DELAY);
                }
                else
                {
                    uint8_t valid = 1;
                    for (int i = 0; i < PIN_LENGTH; i++)
                    {
                        if (rx_buffer[i] < '0' || rx_buffer[i] > '9')
                        {
                            valid = 0;
                            break;
                        }
                    }

                    if (!valid)
                    {
                        HAL_UART_Transmit(&huart2,
                                          (uint8_t*)"\r\nPIN must contain digits only.\r\nEnter 4-digit PIN: ",
                                          strlen("\r\nPIN must contain digits only.\r\nEnter 4-digit PIN: "),
                                          HAL_MAX_DELAY);
                    }
                    else
                    {
                        int result = Db_AddUser(newUserName, (char*)rx_buffer);

                        if (result >= 0)
                        {
                            char msg[64];
                            snprintf(msg, sizeof(msg), "\r\nUser '%s' added successfully.\r\n", newUserName);
                            HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
                        }
                        else if (result == -2)
                        {
                            HAL_UART_Transmit(&huart2,
                                              (uint8_t*)"\r\nThat PIN is already in use.\r\n",
                                              strlen("\r\nThat PIN is already in use.\r\n"),
                                              HAL_MAX_DELAY);
                        }
                        else
                        {
                            HAL_UART_Transmit(&huart2,
                                              (uint8_t*)"\r\nUser database full.\r\n",
                                              strlen("\r\nUser database full.\r\n"),
                                              HAL_MAX_DELAY);
                        }

                        uartMode = UART_MODE_MENU;
                        memset(newUserName, 0, sizeof(newUserName));

                        ClearScreen();
                        PrintMenu();
                    }
                }
            }
        }

        osDelay(10);
    }

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
    char pin[PIN_LENGTH + 1] = {0};
    uint8_t idx = 0;

    for (;;)
    {
        if (lockoutActive)
        {
            osDelay(100);
            continue;
        }

        char key = scan_keypad();

        if (key != 99)
        {
            if (key >= '0' && key <= '9')
            {
                if (!enteringPin)
                {
                    enteringPin = 1;
                    idx = 0;
                    memset(pin, 0, sizeof(pin));
                }

                if (idx < PIN_LENGTH)
                {
                    pin[idx++] = key;
                    pin[idx] = '\0';
                    LCD_ShowMaskedPin(idx);
                }
            }
            else if (key == '*')
            {
                idx = 0;
                memset(pin, 0, sizeof(pin));
                enteringPin = 1;
                LCD_ShowMaskedPin(0);
            }
            else if (key == '#')
            {
                if (idx == PIN_LENGTH)
                {
                    strcpy(pin_entered, pin);
                    pin_ready = 1;
                }

                idx = 0;
                memset(pin, 0, sizeof(pin));
                enteringPin = 0;
            }

            while (scan_keypad() != 99)
            {
                osDelay(10);
            }
            osDelay(50);
        }

        osDelay(20);
    }
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
	AccessState_t lastState = (AccessState_t)99;
  /* Infinite loop */
  for(;;)
  {
      if (accessState != lastState)
      {
          lastState = accessState;

          if (accessState == STATE_LOCKED)
          {
              // locked position
              __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 1000);
              HAL_UART_Transmit(&huart2,
                                (uint8_t*)"Servo -> LOCKED\r\n",
                                strlen("Servo -> LOCKED\r\n"),
                                HAL_MAX_DELAY);
          }
          else if (accessState == STATE_UNLOCKED)
          {
              // unlocked position
              __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);
              HAL_UART_Transmit(&huart2,
                                (uint8_t*)"Servo -> UNLOCKED\r\n",
                                strlen("Servo -> UNLOCKED\r\n"),
                                HAL_MAX_DELAY);
          }
      }

      osDelay(50);
  }
}






/* USER CODE BEGIN Header_lcdTask */
/**
* @brief Function implementing the LCDtask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_lcdTask */
void lcdTask(void *argument)
{
    AccessState_t lastState = (AccessState_t)99;

    for(;;)
    {
        if (!enteringPin && !lockoutActive)
        {
            if (accessState != lastState)
            {
                lastState = accessState;
                LCD_ShowState(accessState);
            }
        }

        osDelay(50);
    }
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
