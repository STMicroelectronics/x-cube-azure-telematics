/**
  ******************************************************************************
  * @file    board_interrupts.c
  * @author  GPM Application Team
  * @brief   Implements HAL weak functions for Interrupts
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the ST_LICENSE file
  * in the root directory of this software component.
  * If no ST_LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"

#include "ipc_uart.h"
#if (USE_CMD_CONSOLE == 1)
#include "cmd.h"
#endif  /* (USE_CMD_CONSOLE == 1) */

/* NOTE : this code is designed for FreeRTOS */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Callback to treat UART Rx complete
  * @param  huart - pointer on UART handle
  * @retval -
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == MODEM_UART_INSTANCE)
  {
    IPC_UART_RxCpltCallback(huart);
  }
#if (USE_CMD_CONSOLE == 1)
  else if (huart->Instance == TRACE_INTERFACE_INSTANCE)
  {
    CMD_RxCpltCallback(huart);
  }
#endif  /* USE_CMD_CONSOLE */
  else
  {
    /* Nothing to do */
    __NOP();
  }
}

/**
  * @brief  Callback to treat UART Tx complete
  * @param  huart - pointer on UART handle
  * @retval -
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == MODEM_UART_INSTANCE)
  {
    IPC_UART_TxCpltCallback(huart);
  }
}

/**
  * @brief  Callback to treat Error
  * @param  huart - pointer on UART handle
  * @retval -
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == MODEM_UART_INSTANCE)
  {
    IPC_UART_ErrorCallback(huart);
  }
}

