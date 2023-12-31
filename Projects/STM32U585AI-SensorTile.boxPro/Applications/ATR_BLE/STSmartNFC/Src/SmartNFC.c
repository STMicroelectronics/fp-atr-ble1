
/**
  ******************************************************************************
  * @file    SmartNFC.h
  * @author  System Research & Applications Team - Agrate Lab.
  * @version V2.1.0
  * @date    25-July-2023
  * @brief   Minimal NDEF header APIs implementation
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

/* Includes ------------------------------------------------------------------*/
#include "SmartNFC.h"
#include "TagType5.h"
#include <stdio.h>

/* Define  -------------------------------------------------------------------*/

/* Uncomment the following define for saving the Max & Min
 * only if the measure in inside the Valid Range that we want to control
 * Otherwise save the Max and Min for any read sensor's value */
/* #define STNFC_MAX_MIN_ONLY_ON_VALID_RANGE */

/* Macros  -------------------------------------------------------------------*/

#ifdef STNFC_MAX_MIN_ONLY_ON_VALID_RANGE
/* update the Max/Min only for value that are inside the log interval */
#define MCR_STNFC_ComputeMaxMinCompareTHs(Type)\
  void STNFC_ComputeMaxMinCompareTHs##Type##t(SNFC_VirtualSensor_t *VirtualSensor,SNFC_LogDefinition_t *LogDefinition,RTC_HandleTypeDef *hrtc)\
  {\
    /* Compare with Ths */\
    switch(VirtualSensor->ThsUsageType) {\
      case TH_EXT:\
        /* External Range: -->TH1 TH2<-- */\
        if((VirtualSensor->Sample.Type##Value<=VirtualSensor->Th1.Type##Value) | \
           (VirtualSensor->Sample.Type##Value>=VirtualSensor->Th2.Type##Value)) {\
          VirtualSensor->SampleDeltaDateTime= STNFC_GetDeltaDateTime(hrtc,LogDefinition);\
        }\
        break;\
      case TH_INT:\
        /* Internal Range: TH1<---->TH2 */\
        if((VirtualSensor->Sample.Type##Value>=VirtualSensor->Th1.Type##Value) & \
           (VirtualSensor->Sample.Type##Value<=VirtualSensor->Th2.Type##Value)) {\
          VirtualSensor->SampleDeltaDateTime= STNFC_GetDeltaDateTime(hrtc,LogDefinition);\
        }\
        break;\
      case TH_LESS:\
        /* Less than     : -->TH1 */\
        if(VirtualSensor->Sample.Type##Value<=VirtualSensor->Th1.Type##Value) {\
          VirtualSensor->SampleDeltaDateTime= STNFC_GetDeltaDateTime(hrtc,LogDefinition);\
        }\
        break;\
      case TH_BIGGER:\
        /* Bigger than   : TH1<-- */\
        if(VirtualSensor->Sample.Type##Value>=VirtualSensor->Th1.Type##Value) {\
          VirtualSensor->SampleDeltaDateTime= STNFC_GetDeltaDateTime(hrtc,LogDefinition);\
        }\
    }\
    \
    /* Compare with Max Min */\
    if(VirtualSensor->SampleDeltaDateTime!=0) {\
      if(VirtualSensor->Sample.Type##Value>VirtualSensor->MaxValue.Type##Value) {\
        VirtualSensor->MaxValue.Type##Value = VirtualSensor->Sample.Type##Value;\
        VirtualSensor->MaxDeltaDateTime = STNFC_GetDeltaDateTime(hrtc,LogDefinition);\
      }\
      \
      if(VirtualSensor->Sample.Type##Value<VirtualSensor->MinValue.Type##Value) {\
        VirtualSensor->MinValue.Type##Value = VirtualSensor->Sample.Type##Value;\
        VirtualSensor->MinDeltaDateTime = STNFC_GetDeltaDateTime(hrtc,LogDefinition);\
      }\
    }\
  }
#else /* STNFC_MAX_MIN_ONLY_ON_VALID_RANGE */
/* update the Max/Min for any valid value */
#define MCR_STNFC_ComputeMaxMinCompareTHs(Type)\
  void STNFC_ComputeMaxMinCompareTHs##Type##t(SNFC_VirtualSensor_t *VirtualSensor,SNFC_LogDefinition_t *LogDefinition,RTC_HandleTypeDef *hrtc)\
  {\
    /* Compare with Ths */\
    switch(VirtualSensor->ThsUsageType) {\
      case TH_EXT:\
        /* External Range: -->TH1 TH2<-- */\
        if((VirtualSensor->Sample.Type##Value<=VirtualSensor->Th1.Type##Value) | \
           (VirtualSensor->Sample.Type##Value>=VirtualSensor->Th2.Type##Value)) {\
          VirtualSensor->SampleDeltaDateTime= STNFC_GetDeltaDateTime(hrtc,LogDefinition);\
        }\
        break;\
      case TH_INT:\
        /* Internal Range: TH1<---->TH2 */\
        if((VirtualSensor->Sample.Type##Value>=VirtualSensor->Th1.Type##Value) & \
           (VirtualSensor->Sample.Type##Value<=VirtualSensor->Th2.Type##Value)) {\
          VirtualSensor->SampleDeltaDateTime= STNFC_GetDeltaDateTime(hrtc,LogDefinition);\
        }\
        break;\
      case TH_LESS:\
        /* Less than     : -->TH1 */\
        if(VirtualSensor->Sample.Type##Value<=VirtualSensor->Th1.Type##Value) {\
          VirtualSensor->SampleDeltaDateTime= STNFC_GetDeltaDateTime(hrtc,LogDefinition);\
        }\
        break;\
      case TH_BIGGER:\
        /* Bigger than   : TH1<-- */\
        if(VirtualSensor->Sample.Type##Value>=VirtualSensor->Th1.Type##Value) {\
          VirtualSensor->SampleDeltaDateTime= STNFC_GetDeltaDateTime(hrtc,LogDefinition);\
        }\
    }\
    \
    /* Compare with Max Min */\
    if(VirtualSensor->Sample.Type##Value>VirtualSensor->MaxValue.Type##Value) {\
      VirtualSensor->MaxValue.Type##Value = VirtualSensor->Sample.Type##Value;\
      VirtualSensor->MaxDeltaDateTime = STNFC_GetDeltaDateTime(hrtc,LogDefinition);\
    }\
    \
    if(VirtualSensor->Sample.Type##Value<VirtualSensor->MinValue.Type##Value) {\
      VirtualSensor->MinValue.Type##Value = VirtualSensor->Sample.Type##Value;\
      VirtualSensor->MinDeltaDateTime = STNFC_GetDeltaDateTime(hrtc,LogDefinition);\
    }\
  }
#endif /* STNFC_MAX_MIN_ONLY_ON_VALID_RANGE */

/* Exported Variables -------------------------------------------------------- */
char *ThresholdsUsageName[4] =
{
  "Ext",
  "Int",
  "Less",
  "Bigger"
};

/* Functions For making the Comparison with the Thresholds */
MCR_STNFC_ComputeMaxMinCompareTHs(Ui8)
MCR_STNFC_ComputeMaxMinCompareTHs(Ui16)
MCR_STNFC_ComputeMaxMinCompareTHs(Ui32)

/**
  * @brief  Initialize the ST-Smart NFC
  * @param  None
  * @retval None
  */
void InitSTSmartNFC(void)
{
  NfcType5_NDEFInitHeader();
}

/**
  * @brief  Set the data and time value.
  * @param  uint32_t DateTime Compressed Date&Time
  * @param  RTC_HandleTypeDef *hrtc RTC handler pointer
  * @param SNFC_LogDefinition_t *LogDef Pointer to Log definition
  * @retval Returns 1 in case of success, otherwise 0 on error..
  */
int32_t STNFC_SetDateTime(uint32_t DateTime, RTC_HandleTypeDef *hrtc, SNFC_LogDefinition_t *LogDef)
{
  int32_t RetValue = 0;
  RTC_TimeTypeDef StartTime;  /* Starting Time */
  RTC_DateTypeDef StartDate;  /* Starting Date */

  struct tm *currTime;

#ifdef  ATRBLE1_VERBOSE_PRINTF
  ATRBLE1_PRINTF("Read TimeStamp %u\r\n", DateTime);
#endif /* ATRBLE1_VERBOSE_PRINTF */
  LogDef->StartTimeStamp = DateTime;

  currTime = localtime(&LogDef->StartTimeStamp);

  StartDate.Year    = currTime->tm_year - 100;
  StartDate.Date    = currTime->tm_mday;
  StartDate.Month   = currTime->tm_mon + 1;
  StartDate.WeekDay = currTime->tm_wday;
  StartTime.Hours   = currTime->tm_hour;
  StartTime.Minutes = currTime->tm_min;
  StartTime.Seconds = currTime->tm_sec;

  ATRBLE1_PRINTF("\tYear    =%d\r\n", StartDate.Year);
  ATRBLE1_PRINTF("\tDate    =%d\r\n", StartDate.Date);
  ATRBLE1_PRINTF("\tMonth   =%d\r\n", StartDate.Month);
  ATRBLE1_PRINTF("\tWDay    =%d\r\n", StartDate.WeekDay);
  ATRBLE1_PRINTF("\tHours   =%d\r\n", StartTime.Hours);
  ATRBLE1_PRINTF("\tMinutes =%d\r\n", StartTime.Minutes);
  ATRBLE1_PRINTF("\tSeconds =%d\r\n", StartTime.Seconds);

  StartTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  StartTime.StoreOperation = RTC_STOREOPERATION_RESET;


  if (HAL_RTC_SetTime(hrtc, &(StartTime), RTC_FORMAT_BIN) == HAL_OK)
  {
    if (HAL_RTC_SetDate(hrtc, &(StartDate), RTC_FORMAT_BIN) == HAL_OK)
    {
      RetValue = 1;
    }
  }
  return RetValue;
}

/**
  * @brief  Get the Delta epoch time.
  * @param  RTC_HandleTypeDef *hrtc RTC handler pointer
  * @param SNFC_LogDefinition_t *LogDef Pointer to Log definition
  * @retval Returns Delta epoch time in case of success, otherwise 0 on error.
  */
time_t STNFC_GetDeltaDateTime(RTC_HandleTypeDef *hrtc, SNFC_LogDefinition_t *LogDef)
{
  RTC_TimeTypeDef CurTime;
  RTC_DateTypeDef CurDate;
  time_t DeltaDateTimeStamp;
  struct tm currTime;

  if (HAL_RTC_GetTime(hrtc, &CurTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    return 0;
  }

  if (HAL_RTC_GetDate(hrtc, &CurDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    return 0;
  }

  currTime.tm_year  = CurDate.Year + 100;
  currTime.tm_mday  = CurDate.Date;
  currTime.tm_mon   = CurDate.Month - 1;
  /* mktime doesn't look tm_wday */
  /* currTime.tm_wday  = CurDate.WeekDay; */

  currTime.tm_hour  = CurTime.Hours;
  currTime.tm_min   = CurTime.Minutes;
  currTime.tm_sec   = CurTime.Seconds;
  currTime.tm_isdst = 0;

  /* Find the Epoch Time */
  DeltaDateTimeStamp = mktime(&currTime);

  /* Compute the Delta Epoch Time */
  DeltaDateTimeStamp -= LogDef->StartTimeStamp;
#ifdef ATRBLE1_VERBOSE_PRINTF
  ATRBLE1_PRINTF("Delta TS=%u\r\n", (uint32_t) DeltaDateTimeStamp);
#endif /* ATRBLE1_VERBOSE_PRINTF */
  return DeltaDateTimeStamp;
}

/**
  * @brief  From Delta epoch time to Short Delta epoch time (without Seconds)
  * @param  time_t DeltaDateTimeStamp Delta epoch time stamp
  * @retval Returns Short Delta epoch time stamp
  */
uint32_t STNFC_ToShortDeltaDateTime(time_t DeltaDateTimeStamp)
{
  uint32_t ShortDeltaTimeStamp;

  DeltaDateTimeStamp /= 60;
  /* For using like Maximum 20bits */
  ShortDeltaTimeStamp = ((uint32_t) DeltaDateTimeStamp) & 0xFFFFF;

#ifdef ATRBLE1_VERBOSE_PRINTF
  ATRBLE1_PRINTF("Short Delta TS=%u\r\n", ShortDeltaTimeStamp);
#endif /* ATRBLE1_VERBOSE_PRINTF */
  return ShortDeltaTimeStamp;
}


