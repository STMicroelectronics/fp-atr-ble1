/**
  ******************************************************************************
  * @file    SmartNFC.h
  * @author  System Research & Applications Team - Agrate Lab.
  * @version V2.1.0
  * @date    25-July-2023
  * @brief   Smart NFC protocol
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
#ifndef __SMART_NFC_H
#define __SMART_NFC_H

#include "stm32u5xx_hal.h"
#include "ATRBLE1_config.h"
#include "SensorTileBoxPro.h"
#include "SensorTileBoxPro_nfctag.h"
#include "SmartNFCType.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Exported Defines ----------------------------------------------------------*/
/* 12 for UID (Length + 3 offset + 8 for UID) */
#define ATRBLE1_UID_EXTRA_LENGHT 0xC

/* -SensorTile.boxPro includes the st25dv8K */
#define STSMART_NFC_MAX_SIZE NFCTAG_4K_SIZE

/* Dimension of the CC file in bytes */
#define ST25DV_CC_SIZE            4

/* Address due to extended CC file + NDEF header before payload == (4 or 8) + 24 + ATRBLE1_UID_EXTRA_LENGHT*/
#define ATRBLE1_START_ADDR_OFFSET (0x18+ST25DV_CC_SIZE + ATRBLE1_UID_EXTRA_LENGHT)

/* Exported Macros -----------------------------------------------------------*/
#define MCR_STNFC_COMPARE_WITH_LIMITS(Type,VirtualSensor,value)\
  {\
    /* Limit the Value */\
    if(value>=VirtualSensor.MaxLimit.Type##Value) {\
      value=VirtualSensor.MaxLimit.Type##Value;\
    }\
    \
    if(value<=VirtualSensor.MinLimit.Type##Value) {\
      value=VirtualSensor.MinLimit.Type##Value;\
    }\
    /* Save the Value */\
    VirtualSensor.Sample.Type##Value = value;\
  }

/* Exported Variables --------------------------------------------------------*/
extern char *ThresholdsUsageName[4];

/* Exported Prototypes -------------------------------------------------------*/
extern void InitSTSmartNFC(void);


extern int32_t  STNFC_SetDateTime(uint32_t DateTime, RTC_HandleTypeDef *hrtc, SNFC_LogDefinition_t *LogDef);
extern time_t STNFC_GetDeltaDateTime(RTC_HandleTypeDef *hrtc, SNFC_LogDefinition_t *LogDef);
extern uint32_t STNFC_ToShortDeltaDateTime(time_t DeltaDateTimeStamp);

extern void STNFC_ComputeMaxMinCompareTHsUi8t(SNFC_VirtualSensor_t *VirtualSensor, SNFC_LogDefinition_t *LogDefinition,
                                              RTC_HandleTypeDef *hrtc);
extern void STNFC_ComputeMaxMinCompareTHsUi16t(SNFC_VirtualSensor_t *VirtualSensor, SNFC_LogDefinition_t *LogDefinition,
                                               RTC_HandleTypeDef *hrtc);
extern void STNFC_ComputeMaxMinCompareTHsUi32t(SNFC_VirtualSensor_t *VirtualSensor, SNFC_LogDefinition_t *LogDefinition,
                                               RTC_HandleTypeDef *hrtc);


#ifdef __cplusplus
}
#endif

#endif /* __SMART_NFC_H */

