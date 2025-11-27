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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "vehicle.h"
#include "can_if.h"
#include "cli_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/*
 * Project: Mini ECU – CAN + FreeRTOS Telemetry Node
 *
 * Version history (high level):
 *   v1.0 - Basic timer-driven LED + UART logger (no RTOS).
 *   v1.1 - UART CLI with interrupt-driven RX.
 *   v2.0 - FreeRTOS tasks + CAN loopback telemetry.
 *   v2.1 - CAN_IF abstraction, RX queue, CLI-controlled logging.
 *   v2.2 - VehicleState model + CLI control + CAN telemetry integration.
 */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

UART_HandleTypeDef huart2;

/* Definitions for defaultTask (kept for CubeMX compatibility, currently unused) */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */

/* Global vehicle state shared between tasks */
VehicleState_t g_vehicle;

/* RTOS task handles */
static osThreadId_t vehicleTaskHandle;
static osThreadId_t cliTaskHandle;
static osThreadId_t canRxTaskHandle;

/* RTOS task attributes */
static const osThreadAttr_t canRxTask_attributes = {
  .name       = "CanRxTask",
  .priority   = osPriorityBelowNormal,
  .stack_size = 256 * 4
};

static const osThreadAttr_t vehicleTask_attributes = {
  .name       = "VehicleTask",
  .priority   = osPriorityNormal,
  .stack_size = 256 * 4
};

static const osThreadAttr_t cliTask_attributes = {
  .name       = "CliTask",
  .priority   = osPriorityAboveNormal,
  .stack_size = 256 * 4
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
static void VehicleTask(void *argument);
static void CliTask(void *argument);
static void CanRxTask(void *argument);
static void uart_print(const char *s);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void uart_print(const char *s)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)s, strlen(s), HAL_MAX_DELAY);
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
  MX_CAN1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  uart_print("\r\n=== Mini ECU – CAN + RTOS Telemetry Node ===\r\n");

  /* Initialize vehicle model */
  Vehicle_Init(&g_vehicle);

  /* Initialize CAN interface (filters, start, queue, notifications) */
  if (CAN_IF_Init() != HAL_OK)
  {
    uart_print("CAN_IF_Init FAILED, halting\r\n");
    Error_Handler();
  }

  /* Initialize CLI interface (starts UART RX internally) */
  CLI_IF_Init(&huart2, &g_vehicle);

  uart_print("Init complete, creating RTOS tasks...\r\n");

  /* Initialize the RTOS kernel */
  osKernelInitialize();

  /* Create VehicleTask: updates model + sends telemetry */
  vehicleTaskHandle = osThreadNew(VehicleTask, NULL, &vehicleTask_attributes);

  /* Create CliTask: runs CLI_IF_Task() in a loop */
  cliTaskHandle = osThreadNew(CliTask, NULL, &cliTask_attributes);

  /* Create CAN RX task: consumes messages from CAN_IF RX queue */
  canRxTaskHandle = osThreadNew(CanRxTask, NULL, &canRxTask_attributes);

  /* Start the RTOS scheduler (never returns) */
  osKernelStart();

  /* We should never reach here */
  uart_print("ERROR: osKernelStart returned!\r\n");

  /* USER CODE END 2 */

  /* Infinite loop (should never execute with RTOS running) */
  while (1)
  {
    /* USER CODE BEGIN WHILE */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{
  /* USER CODE BEGIN CAN1_Init 0 */
  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */
  /* Leave reset handling to HAL – no manual FORCE_RESET here */
  /* USER CODE END CAN1_Init 1 */

  hcan1.Instance = CAN1;

  /* Typical, safe timing for F4 (APB1 = 45 MHz):
     - Prescaler = 16
     - 1 (sync) + 13 + 2 = 16 TQ per bit
     => ~ 45 MHz / (16 * 16) ≈ 175 kbps (exact speed isn’t critical in loopback)
  */
  hcan1.Init.Prescaler           = 16;
  hcan1.Init.Mode                = CAN_MODE_LOOPBACK;   // single-board testing
  hcan1.Init.SyncJumpWidth       = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1            = CAN_BS1_13TQ;
  hcan1.Init.TimeSeg2            = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode   = DISABLE;
  hcan1.Init.AutoBusOff          = DISABLE;
  hcan1.Init.AutoWakeUp          = DISABLE;
  hcan1.Init.AutoRetransmission  = ENABLE;
  hcan1.Init.ReceiveFifoLocked   = DISABLE;
  hcan1.Init.TransmitFifoPriority= DISABLE;

  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN CAN1_Init 2 */
  /* USER CODE END CAN1_Init 2 */
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
  if (HAL_UART_Init(&huart2) != HAL_OK)
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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief Task that updates the vehicle model and sends CAN telemetry.
  */
static void VehicleTask(void *argument)
{
  (void)argument;

  const uint32_t period_ms = 100U;
  uint32_t last_wake = osKernelGetTickCount();

  for (;;)
  {
    /* 0.1 s step */
    Vehicle_Update(&g_vehicle, 0.1f);

    /* Broadcast telemetry on CAN */
    (void)CAN_IF_SendTelemetry(&g_vehicle);

    last_wake += period_ms;
    (void)osDelayUntil(last_wake);
  }
}

/**
  * @brief Task that runs the CLI interface.
  *
  * CLI_IF_Task() drains the ring buffer and parses commands.
  */
static void CliTask(void *argument)
{
  (void)argument;

  for (;;)
  {
    CLI_IF_Task();
    /* Small delay to yield CPU; CLI_IF_Task is non-blocking */
    osDelay(10);
  }
}

/**
  * @brief Task that waits for CAN frames and lets CAN_IF process them.
  */
static void CanRxTask(void *argument)
{
  (void)argument;

  osMessageQueueId_t q = CAN_IF_GetRxQueueHandle();
  if (q == NULL)
  {
    uart_print("CanRxTask: RX queue is NULL!\r\n");
    /* Loop with delay rather than crashing */
    for (;;)
    {
      osDelay(1000);
    }
  }

  for (;;)
  {
    CAN_IF_Msg_t msg;
    /* Wait forever for next CAN message */
    if (osMessageQueueGet(q, &msg, NULL, osWaitForever) == osOK)
    {
      /* Let CAN interface layer handle/log the message */
      CAN_IF_ProcessRxMsg(&msg);
    }
  }
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  *         Currently unused; kept for CubeMX compatibility.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop (not used in current design) */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
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
