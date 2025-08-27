/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// Array to hold the current, dynamically changing motor speeds


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUM_MOTORS 6
#define UART_BUFFER_SIZE NUM_MOTORS
// The step size for acceleration/deceleration
#define RAMP_STEP 5
// Time interval for updating speed (in milliseconds)
#define RAMP_INTERVAL_MS 2
// Variable to store the last time the ramp was updated
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
// Array to hold the raw speed values received from UART
uint8_t uart_rx_buffer[UART_BUFFER_SIZE];
// Array to hold the parsed motor speeds
volatile uint8_t motor_speeds[NUM_MOTORS] = {64,64,64,64,64,64};
volatile uint8_t data_ready_flag = 0;
uint32_t last_ramp_time = 0;
volatile int16_t current_motor_speeds[NUM_MOTORS];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void set_motor_speed(uint8_t motor_index, uint8_t speed_value);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * @brief  This function is executed when the UART reception is complete.
  * @param  huart: UART handle
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        // Copy received data to the motor_speeds array
        for (int i = 0; i < NUM_MOTORS; i++)
        {
            motor_speeds[i] = uart_rx_buffer[i];
        }
        // Set a flag to indicate that new data is ready to be processed
        data_ready_flag = 1;

        // Re-arm the UART interrupt to receive the next packet of data
        HAL_UART_Receive_IT(&huart3, uart_rx_buffer, UART_BUFFER_SIZE);
    }
}

/**
  * @brief  Parses the speed value and sets the motor direction and PWM duty cycle.
  * @param  motor_index: The index of the motor to control (0-5).
  * @param  speed_value: The raw speed value from UART (0-127).
  * @retval None
  */
void set_motor_speed(uint8_t motor_index, uint8_t speed_value)
{
    GPIO_TypeDef* dir_port;
    uint16_t dir_pin;
    TIM_HandleTypeDef* pwm_timer;
    uint32_t pwm_channel;
    uint16_t pwm_value = 0;

    // --- Motor Pin and Timer Configuration ---
    switch (motor_index)
    {
        case 0: // Motor 1
            dir_port = MOTOR1_DIR_GPIO_Port;
            dir_pin = MOTOR1_DIR_Pin;
            pwm_timer = &htim1;
            pwm_channel = TIM_CHANNEL_1;
            break;
        case 1: // Motor 2
            dir_port = MOTOR2_DIR_GPIO_Port;
            dir_pin = MOTOR2_DIR_Pin;
            pwm_timer = &htim1;
            pwm_channel = TIM_CHANNEL_3;
            break;
        case 2: // Motor 3
            dir_port = MOTOR3_DIR_GPIO_Port;
            dir_pin = MOTOR3_DIR_Pin;
            pwm_timer = &htim2;
            pwm_channel = TIM_CHANNEL_2;
            break;
        case 3: // Motor 4
            dir_port = MOTOR4_DIR_GPIO_Port;
            dir_pin = MOTOR4_DIR_Pin;
            pwm_timer = &htim2;
            pwm_channel = TIM_CHANNEL_4;
            break;
        case 4: // Motor 5
            dir_port = MOTOR5_DIR_GPIO_Port;
            dir_pin = MOTOR5_DIR_Pin;
            pwm_timer = &htim3;
            pwm_channel = TIM_CHANNEL_1;
            break;
        case 5: // Motor 6
            dir_port = MOTOR6_DIR_GPIO_Port;
            dir_pin = MOTOR6_DIR_Pin;
            pwm_timer = &htim3;
            pwm_channel = TIM_CHANNEL_3;
            break;
        default:
            return; // Invalid motor index
    }

    // --- Speed and Direction Parsing Logic ---
    if (speed_value == 64) // Motor Stop
    {
        HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_RESET); // Direction doesn't matter
        pwm_value = 0;
    }
    else if (speed_value > 64) // Forward
    {
    	if(dir_pin == MOTOR3_DIR_Pin || dir_pin == MOTOR6_DIR_Pin){
    		HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_SET); // Set direction to Forward

    	}else{
            HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_RESET); // Set direction to Forward
    	}
            // Map 65-127 to a PWM value (e.g., 0-999 for ARR=999)
            // (speed_value - 65) maps 65->0, 127->62
            // We scale this to the timer's ARR value (999)
            pwm_value = (uint16_t)(((speed_value - 65) / 62.0f) * 999);
    }
    else // Backward (speed_value < 64)
    {
    	if(dir_pin == MOTOR3_DIR_Pin || dir_pin == MOTOR6_DIR_Pin){
			HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_RESET); // Set direction to Forward

		}else{
			HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_SET); // Set direction to Forward
		}
        // Map 0-63 to a PWM value. 63 is slowest, 0 is fastest.
        // (63 - speed_value) maps 63->0, 0->63
        pwm_value = (uint16_t)(((63 - speed_value) / 63.0f) * 999);
    }

    // --- Set the PWM Duty Cycle ---
    __HAL_TIM_SET_COMPARE(pwm_timer, pwm_channel, pwm_value);
}

/* USER CODE BEGIN 0 */
/**
  * @brief  Sets the motor direction and PWM duty cycle based on a new PWM value.
  * @param  motor_index: The index of the motor to control (0-5).
  * @param  pwm_value: The calculated PWM value to set.
  * @param  speed_value: The raw speed value from UART (for direction logic).
  * @retval None
  */
void set_motor_speed_trapezoid(uint8_t motor_index, int16_t pwm_value, uint8_t speed_value)
{
    GPIO_TypeDef* dir_port;
    uint16_t dir_pin;
    TIM_HandleTypeDef* pwm_timer;
    uint32_t pwm_channel;

    // --- Motor Pin and Timer Configuration ---
    switch (motor_index)
    {
        case 0: // Motor 1
            dir_port = MOTOR1_DIR_GPIO_Port;
            dir_pin = MOTOR1_DIR_Pin;
            pwm_timer = &htim1;
            pwm_channel = TIM_CHANNEL_1;
            break;
        case 1: // Motor 2
            dir_port = MOTOR2_DIR_GPIO_Port;
            dir_pin = MOTOR2_DIR_Pin;
            pwm_timer = &htim1;
            pwm_channel = TIM_CHANNEL_3;
            break;
        case 2: // Motor 3
            dir_port = MOTOR3_DIR_GPIO_Port;
            dir_pin = MOTOR3_DIR_Pin;
            pwm_timer = &htim2;
            pwm_channel = TIM_CHANNEL_2;
            break;
        case 3: // Motor 4
            dir_port = MOTOR4_DIR_GPIO_Port;
            dir_pin = MOTOR4_DIR_Pin;
            pwm_timer = &htim2;
            pwm_channel = TIM_CHANNEL_4;
            break;
        case 4: // Motor 5
            dir_port = MOTOR5_DIR_GPIO_Port;
            dir_pin = MOTOR5_DIR_Pin;
            pwm_timer = &htim3;
            pwm_channel = TIM_CHANNEL_1;
            break;
        case 5: // Motor 6
            dir_port = MOTOR6_DIR_GPIO_Port;
            dir_pin = MOTOR6_DIR_Pin;
            pwm_timer = &htim3;
            pwm_channel = TIM_CHANNEL_3;
            break;
        default:
            return; // Invalid motor index
    }

    // --- Direction Parsing Logic ---
    // The direction only needs to be set once the motor starts moving,
    // or if the target direction changes.
    if (speed_value > 64) // Forward
    {
        if(dir_pin == MOTOR3_DIR_Pin || dir_pin == MOTOR6_DIR_Pin){
            HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_RESET);
        }
    }
    else if (speed_value < 64) // Backward
    {
        if(dir_pin == MOTOR3_DIR_Pin || dir_pin == MOTOR6_DIR_Pin){
            HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_RESET);
        } else {
            HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_SET);
        }
    }
    // If speed_value == 64, direction is irrelevant.

    // --- Set the PWM Duty Cycle ---
    __HAL_TIM_SET_COMPARE(pwm_timer, pwm_channel, pwm_value);
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

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

    // Initialize all motors to stop
      for (int i = 0; i < NUM_MOTORS; i++)
      {
          set_motor_speed(i, 64);
      }

      // Start listening for UART data
      HAL_UART_Receive_IT(&huart3, uart_rx_buffer, UART_BUFFER_SIZE);
  /* USER CODE END 2 */

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

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

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
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
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 71;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, MOTOR3_DIR_Pin|MOTOR4_DIR_Pin|MOTOR5_DIR_Pin|MOTOR6_DIR_Pin
                          |MOTOR2_DIR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MOTOR1_DIR_GPIO_Port, MOTOR1_DIR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : MOTOR3_DIR_Pin MOTOR4_DIR_Pin MOTOR5_DIR_Pin MOTOR6_DIR_Pin
                           MOTOR2_DIR_Pin */
  GPIO_InitStruct.Pin = MOTOR3_DIR_Pin|MOTOR4_DIR_Pin|MOTOR5_DIR_Pin|MOTOR6_DIR_Pin
                          |MOTOR2_DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : MOTOR1_DIR_Pin */
  GPIO_InitStruct.Pin = MOTOR1_DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MOTOR1_DIR_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
