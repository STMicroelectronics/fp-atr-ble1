
/**
  ******************************************************************************
  * @file    LogControl.h
  * @author  System Research & Applications Team - Agrate/Catania Lab.
  * @version V2.1.0
  * @date    25-July-2023
  * @brief   Header for LogControl.c file
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
#ifndef _LOG_CONTROL_H
#define _LOG_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported Variables --------------------------------------------------------*/
extern SNFC_CodeDefinition_t SmarTNFCHeader;
extern SNFC_LogDefinition_t LogDefinition;
extern SNFC_VirtualSensor_t AllVirtualSensorsArray[ATRBLE1_VIRTUAL_SENSORS_NUM];
extern SNFC_VirtualSensor_t *ConfiguratedVirtualSensorsArray[ATRBLE1_VIRTUAL_SENSORS_NUM];
extern SNFC_LogStatus_t CurrentLogStatus;
extern volatile int32_t ReadSensorAndLog;

/* Exported Functions Prototypes ---------------------------------------------*/
extern void AppLogStart(void);
extern void AppSaveEvent(void);
extern void AppLogDetectRFActivity(void);
extern void StartNewLog(void);
extern void StopCurrentLog(void);

/* Log Management from BLE PnP-L */
extern void BleDeleteCurrentLog(void);
extern void BlePauseCurrentLog(void);
extern void BleResumeCurrentLog(void);
extern void BleReadCurrentLog(void);
extern void BleStartNewLog(void);
#define BleStopCurrentLog StopCurrentLog

/* Exported defines ----------------------------------------------------------*/

#define STTS22H_SAMPLE_TO_CODED(Value)        ((uint16_t)(((Value)*5) - ((-10)*5)))
#define LPS22DF_SAMPLE_TO_CODED(Value)       ((uint16_t)(((Value)*2) - ((260)*2)))

#define STTS22H_CODED_TO_SAMPLE(Value)       ((((float)(Value))/5)-10)
#define LPS22DF_CODED_TO_SAMPLE(Value)       ((((float)(Value))/2)+260)

#ifndef TRUE
#define TRUE (1)
#endif /* TRUE */

#ifndef FALSE
#define FALSE (0)
#endif /* FALSE */

/* Events type */
#define NO_EVENT    0x00
#define SYNC_EVENT  0x01
#define ASYNC_EVENT 0x02

#ifdef __cplusplus
}
#endif

#endif /* _LOG_CONTROL_H */
