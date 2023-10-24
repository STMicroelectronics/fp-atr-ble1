/**
  ******************************************************************************
  * @file    LogFlashControl.h
  * @author  System Research & Applications Team - Agrate Lab.
  * @version V2.1.0
  * @date    25-July-2023
  * @brief   API implementation for controlling the log saved in flash
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SensorTileBoxPro.h"
#include "ATRBLE1_config.h"
#include "main.h"
#include "LogFlashControl.h"
#include "SmartNFC.h"
#include "LogControl.h"

/* Local types ---------------------------------------------------------------*/

/* Local defines -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint32_t WritingAddress;
static uint32_t BufferValueToWrite[4];
static uint32_t ValuesSavedOnBuffer = 0;

/* Private Function Prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
  * @brief Deleting Log Area
  * @param None
  * @retval None
  */
void DeleteLogArea(void)
{
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t SectorError = 0;

  ATRBLE1_PRINTF("Delete Log Flash Area\r\n");
  ATRBLE1_PRINTF("\tStart FLASH Erase\r\n");

  if (CurrentActiveBank == 1)
  {
    EraseInitStruct.Banks       = FLASH_BANK_1;
  }
  else
  {
    EraseInitStruct.Banks       = FLASH_BANK_2;
  }

  EraseInitStruct.Page        = FLASH_PAGE_NB - 1 /* For BoardId And board Name */ - MAX_LOG_SIZE / FLASH_PAGE_SIZE;
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.NbPages     = MAX_LOG_SIZE / FLASH_PAGE_SIZE;

  /* Disable instruction cache prior to internal cacheable memory update */
  if (HAL_ICACHE_Disable() != HAL_OK)
  {
    Error_Handler(ATRBLE1_ERROR_FLASH, __FILE__, __LINE__);
  }

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
  {
    /* Error occurred while sector erase.
      User can add here some code to deal with this error.
      SectorError will contain the faulty sector and then to know the code error on this sector,
      user can call function 'HAL_FLASH_GetError()'
      FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
    Error_Handler(ATRBLE1_ERROR_FLASH, __FILE__, __LINE__);
  }
  else
  {
    ATRBLE1_PRINTF("\tEnd FLASH Erase %ld Pages of %dBytes\r\n", EraseInitStruct.NbPages, FLASH_PAGE_SIZE);
  }

  /* Lock the Flash to disable the flash control register access (recommended
  to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  /* Re-enable instruction cache */
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler(ATRBLE1_ERROR_FLASH, __FILE__, __LINE__);
  }


  WritingAddress = LOG_FIRST_ADDRESS;
  ValuesSavedOnBuffer = 0;
}

/** @brief Writes a 32 bit on Flash
  * @param uint32_t Value data to write on Flash
  * @retval None
  */
void WriteLogData(uint32_t Value)
{

  BufferValueToWrite[ValuesSavedOnBuffer] = Value;
  ValuesSavedOnBuffer++;

  if (ValuesSavedOnBuffer == 4)
  {
    /* We need to write the data on Flash */

    /* Check if we could add Data to the Flash */
    if (WritingAddress != LOG_LAST_ADDRESS)
    {
      /* Disable instruction cache prior to internal cacheable memory update */
      if (HAL_ICACHE_Disable() != HAL_OK)
      {
        Error_Handler(ATRBLE1_ERROR_FLASH, __FILE__, __LINE__);
      }
      /* Unlock the Flash to enable the flash control register access *************/
      HAL_FLASH_Unlock();

      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, WritingAddress, ((uint32_t)BufferValueToWrite)) == HAL_OK)
      {
        WritingAddress += 16;
      }
      else
      {
        /* Error occurred while writing data in Flash memory.
           User can add here some code to deal with this error
           FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
        Error_Handler(ATRBLE1_ERROR_FLASH, __FILE__, __LINE__);
      }

      /* Lock the Flash to disable the flash control register access (recommended
       to protect the FLASH memory against possible unwanted operation) *********/
      HAL_FLASH_Lock();

      /* Re-enable instruction cache */
      if (HAL_ICACHE_Enable() != HAL_OK)
      {
        Error_Handler(ATRBLE1_ERROR_FLASH, __FILE__, __LINE__);
      }
    }
    else
    {
      ATRBLE1_PRINTF("\r\nEnd of Log Space... Stop the current Log\r\n");
      StopCurrentLog();
    }

    ValuesSavedOnBuffer = 0;
  }
}


/** @brief Dump latest bytes on flash
  * @param None
  * @retval None
  */
void DumpLogData(void)
{
  /* check if we need to dump the last bytes */
  if ((ValuesSavedOnBuffer != 0) & (WritingAddress != LOG_LAST_ADDRESS))
  {
    /* Disable instruction cache prior to internal cacheable memory update */
    if (HAL_ICACHE_Disable() != HAL_OK)
    {
      Error_Handler(ATRBLE1_ERROR_FLASH, __FILE__, __LINE__);
    }
    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();


    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, WritingAddress, ((uint32_t)BufferValueToWrite)) != HAL_OK)
    {
      /* Error occurred while writing data in Flash memory.
         User can add here some code to deal with this error
         FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
      Error_Handler(ATRBLE1_ERROR_FLASH, __FILE__, __LINE__);
    }

    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();

    /* Re-enable instruction cache */
    if (HAL_ICACHE_Enable() != HAL_OK)
    {
      Error_Handler(ATRBLE1_ERROR_FLASH, __FILE__, __LINE__);
    }
    ValuesSavedOnBuffer = 0;
  }
}

