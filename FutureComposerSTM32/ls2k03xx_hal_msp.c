/**
  ******************************************************************************
  * @file         ls2k03xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 Ilikara <3435193369@qq.com>
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

extern DMA_HandleTypeDef hdma_tim_up;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* External functions --------------------------------------------------------*/

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(huart->Instance==UART0)
  {
    GPIO_InitStruct.Pin = 40;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_M;
    HAL_GPIO_Init(&GPIO_InitStruct);

    GPIO_InitStruct.Pin = 41;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_M;
    HAL_GPIO_Init(&GPIO_InitStruct);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  if(huart->Instance==UART0)
  {
    // Todo
  }
}

/**
  * @brief TIM_Base MSP Initialization
  * This function configures the hardware resources used in this example
  * @param htim_base: TIM_Base handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(htim_base->Instance==ATIM)
  {
    hdma_tim_up.Instance = DMA_Channel5;
    hdma_tim_up.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tim_up.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim_up.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim_up.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_tim_up.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_tim_up.Init.Mode = DMA_CIRCULAR;
    hdma_tim_up.Init.Priority = DMA_PRIORITY_LOW;

    if (HAL_DMA_Init(&hdma_tim_up) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(htim_base, hdma[TIM_DMA_ID_UPDATE], hdma_tim_up);

    // ATIM CH3N GPIO86
    GPIO_InitStruct.Pin = 86;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_M;
    HAL_GPIO_Init(&GPIO_InitStruct);
  }
}

/**
  * @brief TIM_Base MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param htim_base: TIM_Base handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==ATIM)
  {
    /* TIM DMA DeInit */
    HAL_DMA_DeInit(&hdma_tim_up);
  }
}
