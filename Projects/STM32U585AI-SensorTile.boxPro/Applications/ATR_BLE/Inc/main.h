/**
  ******************************************************************************
  * @file    main.h
  * @author  System Research & Applications Team - Agrate Lab.
  * @version V2.1.0
  * @date    25-July-2023
  * @brief   Header for main.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"
#include "SensorTileBoxPro.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#define MCR_HEART_BIT()   \
  {                         \
    BSP_LED_On(LED_YELLOW); \
    BSP_LED_On(LED_GREEN);  \
    HAL_Delay(200);         \
    BSP_LED_Off(LED_YELLOW);\
    BSP_LED_Off(LED_GREEN); \
    HAL_Delay(400);         \
    BSP_LED_On(LED_YELLOW); \
    BSP_LED_On(LED_GREEN);  \
    HAL_Delay(200);         \
    BSP_LED_Off(LED_YELLOW);\
    BSP_LED_Off(LED_GREEN); \
    HAL_Delay(1000);        \
  }

#define MCR_HEART_BIT2()  \
  {                         \
    BSP_LED_On(LED_YELLOW); \
    BSP_LED_On(LED_RED);    \
    HAL_Delay(200);         \
    BSP_LED_Off(LED_YELLOW);\
    BSP_LED_Off(LED_RED);   \
    HAL_Delay(400);         \
    BSP_LED_On(LED_YELLOW); \
    BSP_LED_On(LED_RED);    \
    HAL_Delay(200);         \
    BSP_LED_Off(LED_YELLOW);\
    BSP_LED_Off(LED_RED);   \
    HAL_Delay(1000);        \
  }

/* Exported Variables --------------------------------------------------------*/
extern void *HandleGGComponent;
extern int32_t CurrentActiveBank;
extern volatile uint32_t SwitchOffLedBlink;
extern FinishGood_TypeDef FinishGood;

/* Exported Functions Prototypes ---------------------------------------------*/
extern void Error_Handler(int32_t ErrorCode, char *File, int32_t Line);
extern void LogStartPeriodicallyTimer(void);
extern void LogStopPeriodicallyTimer(void);
extern void BlinkStartPeriodicallyTimer(void);
extern void BlinkStopPeriodicallyTimer(void);
extern void AppDetectMemsEvent(void);
extern void DisableInterruptFromMems(void);
extern void EnableInterruptFromMems(void);
/* Exported defines -----------------------------------------------------------*/
#define ATRBLE1_ERROR_INIT_BLE 1
#define ATRBLE1_ERROR_FLASH 2
#define ATRBLE1_ERROR_SENSOR 3
#define ATRBLE1_ERROR_HW_INIT 4
#define ATRBLE1_ERROR_BLE 5
#define ATRBLE1_ERROR_TIMER 6
#define ATRBLE1_ERROR_NFC_READING 7
#define ATRBLE1_ERROR_NFC_WRITING 8

/* STM32 Unique ID */
#define STM32_UUID ((uint32_t *)0x0BFA0700)

/* STM32 MCU_ID */
#define STM32_MCU_ID ((uint32_t *)0xE0044000)

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
