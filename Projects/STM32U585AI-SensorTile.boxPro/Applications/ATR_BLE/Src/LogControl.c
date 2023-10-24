/**
  ******************************************************************************
  * @file    LogControl.c
  * @author  System Research & Applications Team - Agrate Lab.
  * @version V2.1.0
  * @date    25-July-2023
  * @brief   LogControl API implementation
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
#include "main.h"
#include "SensorTileBoxPro.h"
#include "SensorTileBoxPro_env_sensors.h"
#include "SensorTileBoxPro_env_sensors_ex.h"
#include "SensorTileBoxPro_motion_sensors.h"
#include "SensorTileBoxPro_motion_sensors_ex.h"
#include "SensorTileBoxPro_nfctag.h"
#include "SensorTileBoxPro_nfctag_ex.h"
#include "SmartNFC.h"
#include "TagType5.h"
#include "LogFlashControl.h"
#include "LogControl.h"
#include "lsm6dsv16x_tilt_angle_mode0.h"

#include "BLE_Manager.h"

/* Exported variables --------------------------------------------------------*/
/* ST Smart NFC Protocol */
SNFC_CodeDefinition_t SmarTNFCHeader;
SNFC_LogDefinition_t LogDefinition;
SNFC_VirtualSensor_t AllVirtualSensorsArray[ATRBLE1_VIRTUAL_SENSORS_NUM];
SNFC_VirtualSensor_t *ConfiguratedVirtualSensorsArray[ATRBLE1_VIRTUAL_SENSORS_NUM];
SNFC_LogStatus_t CurrentLogStatus = STNFC_LOG_STOP;
volatile int32_t ReadSensorAndLog = NO_EVENT;

/* Private Defines -----------------------------------------------------------*/

/* Context of LSM6DSV16X */
#define LSM6DSV16X_CONTEX (&(((LSM6DSV16X_Object_t *)MotionCompObj[LSM6DSV16X_0])->Ctx))

/* Uncomment for enabling verbose output */
/* #define ATRBLE1_VERBOSE_OUTPUT */

/* NFC field */
#define FIELD_UNDEF   0
#define FIELD_FALLING 1
#define FIELD_RISING  2

#define ST25_RETRY_NB     ((uint8_t) 15)
#define ST25_RETRY_DELAY  ((uint8_t) 40)

/**
  * @brief Iterate ST25DV command depending on the command return status.
  * @param cmd A ST25DV function returning a NFCTAG_StatusTypeDef status.
  */
#define ST25_RETRY(cmd) {              \
  int st25_retry = ST25_RETRY_NB;    \
  int st25_status = NFCTAG_ERROR;    \
  while((st25_status != NFCTAG_OK) & (st25_retry>0)) { \
    st25_status = cmd;               \
    if(st25_status != NFCTAG_OK) {   \
      HAL_Delay(ST25_RETRY_DELAY);   \
    }                                \
    st25_retry--;                    \
  }                                  \
}

/* Imported variables --------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;

extern void *MotionCompObj[];

/* Imported prototypes -------------------------------------------------------*/

/* Private typedefs ----------------------------------------------------------*/
/**
  * @brief  Valid Configuration Type
  */
typedef enum
{
  STNFC_VALID_CONFIG = 0,
  STNFC_NOT_VALID_CONFIG,
  STNFC_NOT_CHANGED_CONFIG,
  STNFC_ERROR_READING_CONFIG
} SNFC_ValidConfiguration_t;


/* Private Variables ---------------------------------------------------------*/
/* R/F Activity from ST25DV */
static uint32_t RFActivityStatus = FIELD_UNDEF;
static volatile uint8_t FirstEventNotApplicable = 0;

static uint32_t NumberOfSamplesSavedInFlash = 0;

/* Private Function prototypes -----------------------------------------------*/
static void SetAllAvailableVirtualSensors(void);
static void SaveVirtualSensorsConfiguration(void);
static void ResetMaxMinValuesAllVirtualSensors(void);
static void SaveMaxMinValuesForVirtualSensors(void);
static void BleResetSaveMinValuesForVirtualSensors(void);
static void UpdateLastSamplePointerAndSampleCounter(SNFC_LogDefinition_t *LogDefinition);
static void SetNFCBehavior(void);
static void ReadConfiguration(SNFC_LogDefinition_t *LogDefinition, uint32_t CurrentStartDateTime, int32_t OnlyChecks);
static void SaveDefaultConfiguration(void);
static int32_t InitDeInitAccEventThreshold(void);
static void CheckIfNewConfiguration(void);
static uint8_t Understand6DOrientation(void);
static int32_t DetectorValueForAccEvent(void);
static int32_t AccNormVectorApproxEvaluator(BSP_MOTION_SENSOR_Axes_t Value_XYZ_mg);
/**
  * @brief Initialize the Log application
  * @param None
  * @retval None
  */
void AppLogStart(void)
{
  /* Reset the Structures used for Controlling the Log */
  memset(&SmarTNFCHeader, 0, sizeof(SNFC_CodeDefinition_t));
  memset(&LogDefinition, 0, sizeof(SNFC_LogDefinition_t));
  memset(AllVirtualSensorsArray, 0, ATRBLE1_VIRTUAL_SENSORS_NUM * sizeof(SNFC_VirtualSensor_t));
  memset(ConfiguratedVirtualSensorsArray, 0, ATRBLE1_VIRTUAL_SENSORS_NUM * sizeof(SNFC_VirtualSensor_t *));

  SetAllAvailableVirtualSensors();

  /* Init NDEF header */
  InitSTSmartNFC();

  ATRBLE1_PRINTF("Check if there is a valid Configuration\r\n");
  {
    SNFC_LogDefinition_t LocalLogDefinition;
    ReadConfiguration(&LocalLogDefinition, ATRBLE1_DEFAULT_EPOCH_START_TIME, 1);
    if (LocalLogDefinition.StartDateTime != 0)
    {
      StartNewLog();
    }
    else
    {
      SaveDefaultConfiguration();
    }
  }

  /* Set the NFC behavior */
  ATRBLE1_PRINTF("Set NFC Behavior\r\n");
  SetNFCBehavior();
}

/**
  * @brief Save one Sync/Async Event
  * @param None
  * @retval None
  */
void AppSaveEvent(void)
{
  if (ReadSensorAndLog != NO_EVENT)
  {
    /* We update the Max/Min on NFC and we save on Flash the new Sample */

    ST25_RETRY(BSP_NFCTAG_SetRFSleep_Dyn(BSP_NFCTAG_INSTANCE));
#ifdef ATRBLE1_VERBOSE_OUTPUT
    ATRBLE1_PRINTF("\t-->Sleep RF\r\n");
#endif /* ATRBLE1_VERBOSE_OUTPUT */

    if (ReadSensorAndLog & ASYNC_EVENT)
    {
      uint32_t DataBuf32;
      ReadSensorAndLog &= ~ASYNC_EVENT;
      ATRBLE1_PRINTF("Async Event:\r\n");

      if (AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].SampleDeltaDateTime != 0)
      {
#ifdef ATRBLE1_VERBOSE_OUTPUT
        ATRBLE1_PRINTF("Save LSM6DSV16X WakeUp=%d\r\n", AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Sample.Ui16Value);
#else /* ATRBLE1_VERBOSE_OUTPUT */
        ATRBLE1_PRINTF("Save LSM6DSV16X WakeUp\r\n");
#endif /* ATRBLE1_VERBOSE_OUTPUT */

        DataBuf32 = LSM6DSV16X_WU_VS_ID |
                    (((uint32_t)AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].SampleDeltaDateTime) << 3);
        /* Save the data in Flash */
        WriteLogData(DataBuf32);

        DataBuf32 = AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Sample.Ui16Value;
        /* Save the data in Flash */
        WriteLogData(DataBuf32);

        NumberOfSamplesSavedInFlash++;

        AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].SampleDeltaDateTime = 0;
      }

      if (AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxDeltaDateTime != 0)
      {
#ifdef ATRBLE1_VERBOSE_OUTPUT
        ATRBLE1_PRINTF("Save Max Value for LSM6DSV16X =%d\r\n",
                       AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxValue.Ui16Value);
#else /* ATRBLE1_VERBOSE_OUTPUT */
        ATRBLE1_PRINTF("Save Max Value for LSM6DSV16X\r\n");
#endif /* ATRBLE1_VERBOSE_OUTPUT */

        DataBuf32 = STNFC_ToShortDeltaDateTime(AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxDeltaDateTime);
        if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32,
                                 AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxPositionPointer, 4) != NFCTAG_OK)
        {
          Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
        }

        DataBuf32 = (uint32_t) AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxValue.Ui16Value;
        if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32,
                                 AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxPositionPointer + 4, 4) != NFCTAG_OK)
        {
          Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
        }

        AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxDeltaDateTime = 0;
      }


      if (AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].SampleDeltaDateTime != 0)
      {
#ifdef ATRBLE1_VERBOSE_OUTPUT
        ATRBLE1_PRINTF("Save LSM6DSV16X 6D=%d\r\n", AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Sample.Ui16Value);
#else /* ATRBLE1_VERBOSE_OUTPUT */
        ATRBLE1_PRINTF("Save LSM6DSV16X 6D\r\n");
#endif /* ATRBLE1_VERBOSE_OUTPUT */

        DataBuf32 = LSM6DSV16X_6D_VS_ID |
                    (((uint32_t)AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].SampleDeltaDateTime) << 3);
        /* Save the data in Flash */
        WriteLogData(DataBuf32);

        DataBuf32 = AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Sample.Ui8Value;
        /* Save the data in Flash */
        WriteLogData(DataBuf32);

        NumberOfSamplesSavedInFlash++;

        AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].SampleDeltaDateTime = 0;
      }

      if (AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].SampleDeltaDateTime != 0)
      {
#ifdef ATRBLE1_VERBOSE_OUTPUT
        ATRBLE1_PRINTF("Save LSM6DSV16X MLC=%d\r\n", AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Sample.Ui8Value);
#else /* ATRBLE1_VERBOSE_OUTPUT */
        ATRBLE1_PRINTF("Save LSM6DSV16X MLC\r\n");
#endif /* ATRBLE1_VERBOSE_OUTPUT */

        DataBuf32 = LSM6DSV16X_MLC_VS_ID |
                    (((uint32_t)AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].SampleDeltaDateTime) << 3);
        /* Save the data in Flash */
        WriteLogData(DataBuf32);

        DataBuf32 = AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Sample.Ui8Value;
        /* Save the data in Flash */
        WriteLogData(DataBuf32);

        NumberOfSamplesSavedInFlash++;

        AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].SampleDeltaDateTime = 0;
      }
    }

    if (ReadSensorAndLog & SYNC_EVENT)
    {
      ReadSensorAndLog &= ~SYNC_EVENT;
      ATRBLE1_PRINTF("Sync Event:\r\n");

      if (AllVirtualSensorsArray[LPS22DF_VS_ID].Enable)
      {
        float Value;
        uint16_t ValueToCheck;
        uint8_t Status;
        uint32_t DataBuf32;
        do
        {
          HAL_Delay(2);
          BSP_ENV_SENSOR_Get_DRDY_Status(LPS22DF_0, ENV_PRESSURE, &Status);
        } while (Status != 1);
        BSP_ENV_SENSOR_GetValue(LPS22DF_0, ENV_PRESSURE, &Value);
        ATRBLE1_PRINTF("\t\tLPS22DF:\tPress= %f\r\n", Value);
        ValueToCheck = LPS22DF_SAMPLE_TO_CODED(Value);
        /* Check the Value respect Min and Max Limit Values*/
        MCR_STNFC_COMPARE_WITH_LIMITS(Ui16, AllVirtualSensorsArray[LPS22DF_VS_ID], ValueToCheck);
        /* Compare with Ths and Update the Max/Min Sample Value */
        STNFC_ComputeMaxMinCompareTHsUi16t(&AllVirtualSensorsArray[LPS22DF_VS_ID], &LogDefinition, &hrtc);

        if (AllVirtualSensorsArray[LPS22DF_VS_ID].SampleDeltaDateTime != 0)
        {
#ifdef ATRBLE1_VERBOSE_OUTPUT
          ATRBLE1_PRINTF("\tSave LPS22DF=%d\r\n", AllVirtualSensorsArray[LPS22DF_VS_ID].Sample.Ui16Value);
#else /* ATRBLE1_VERBOSE_OUTPUT */
          ATRBLE1_PRINTF("\tSave LPS22DF\r\n");
#endif /* ATRBLE1_VERBOSE_OUTPUT */

          DataBuf32 = LPS22DF_VS_ID |
                      (((uint32_t)AllVirtualSensorsArray[LPS22DF_VS_ID].SampleDeltaDateTime) << 3);
          /* Save the data in Flash */
          WriteLogData(DataBuf32);

          DataBuf32 = AllVirtualSensorsArray[LPS22DF_VS_ID].Sample.Ui16Value;
          /* Save the data in Flash */
          WriteLogData(DataBuf32);

          NumberOfSamplesSavedInFlash++;

          AllVirtualSensorsArray[LPS22DF_VS_ID].SampleDeltaDateTime = 0;
        }

        if (AllVirtualSensorsArray[LPS22DF_VS_ID].MinDeltaDateTime != 0)
        {
#ifdef ATRBLE1_VERBOSE_OUTPUT
          ATRBLE1_PRINTF("\tSave Min Value for LPS22DF=%f\r\n",
                         LPS22DF_CODED_TO_SAMPLE(AllVirtualSensorsArray[LPS22DF_VS_ID].MinValue.Ui16Value));
#else /* ATRBLE1_VERBOSE_OUTPUT */
          ATRBLE1_PRINTF("\tSave Min Value for LPS22DF\r\n");
#endif /* ATRBLE1_VERBOSE_OUTPUT */

          DataBuf32 = STNFC_ToShortDeltaDateTime(AllVirtualSensorsArray[LPS22DF_VS_ID].MinDeltaDateTime) |
                      (((uint32_t)AllVirtualSensorsArray[LPS22DF_VS_ID].MinValue.Ui16Value) << 20);
          if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32,
                                   AllVirtualSensorsArray[LPS22DF_VS_ID].MinPositionPointer, 4) != NFCTAG_OK)
          {
            Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
          }
          AllVirtualSensorsArray[LPS22DF_VS_ID].MinDeltaDateTime = 0;
        }

        if (AllVirtualSensorsArray[LPS22DF_VS_ID].MaxDeltaDateTime != 0)
        {
#ifdef ATRBLE1_VERBOSE_OUTPUT
          ATRBLE1_PRINTF("\tSave Max Value for LPS22DF=%f\r\n",
                         LPS22DF_CODED_TO_SAMPLE(AllVirtualSensorsArray[LPS22DF_VS_ID].MaxValue.Ui16Value));
#else /* ATRBLE1_VERBOSE_OUTPUT */
          ATRBLE1_PRINTF("\tSave Max Value for LPS22DF\r\n");
#endif /* ATRBLE1_VERBOSE_OUTPUT */

          DataBuf32 = STNFC_ToShortDeltaDateTime(AllVirtualSensorsArray[LPS22DF_VS_ID].MaxDeltaDateTime) |
                      (((uint32_t)AllVirtualSensorsArray[LPS22DF_VS_ID].MaxValue.Ui16Value) << 20);
          if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32,
                                   AllVirtualSensorsArray[LPS22DF_VS_ID].MaxPositionPointer, 4) != NFCTAG_OK)
          {
            Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
          }
          AllVirtualSensorsArray[LPS22DF_VS_ID].MaxDeltaDateTime = 0;
        }

      }

      if (AllVirtualSensorsArray[STTS22H_VS_ID].Enable)
      {
        float Value;
        uint16_t ValueToCheck;
        uint8_t Status;
        uint32_t DataBuf32;
        do
        {
          HAL_Delay(10);
          BSP_ENV_SENSOR_Get_One_Shot_Status(STTS22H_0, &Status);
        } while (Status != 1);

        BSP_ENV_SENSOR_GetValue(STTS22H_0, ENV_TEMPERATURE, &Value);
        ATRBLE1_PRINTF("\t\tSTTS22H:\tTemp= %f\r\n", Value);
        ValueToCheck = STTS22H_SAMPLE_TO_CODED(Value);
        /* Check the Value respect Min and Max Limit Values*/
        MCR_STNFC_COMPARE_WITH_LIMITS(Ui16, AllVirtualSensorsArray[STTS22H_VS_ID], ValueToCheck);
        /* Compare with Ths and Update the Max/Min Sample Value */
        STNFC_ComputeMaxMinCompareTHsUi16t(&AllVirtualSensorsArray[STTS22H_VS_ID], &LogDefinition, &hrtc);

        if (AllVirtualSensorsArray[STTS22H_VS_ID].SampleDeltaDateTime != 0)
        {
#ifdef ATRBLE1_VERBOSE_OUTPUT
          ATRBLE1_PRINTF("\tSave STS22H=%d\r\n", AllVirtualSensorsArray[STTS22H_VS_ID].Sample.Ui16Value);
#else /* ATRBLE1_VERBOSE_OUTPUT */
          ATRBLE1_PRINTF("\tSave STS22H\r\n");
#endif /* ATRBLE1_VERBOSE_OUTPUT */

          DataBuf32 = STTS22H_VS_ID |
                      (((uint32_t)AllVirtualSensorsArray[STTS22H_VS_ID].SampleDeltaDateTime) << 3);
          /* Save the data in Flash */
          WriteLogData(DataBuf32);

          DataBuf32 = AllVirtualSensorsArray[STTS22H_VS_ID].Sample.Ui16Value;
          /* Save the data in Flash */
          WriteLogData(DataBuf32);

          NumberOfSamplesSavedInFlash++;

          AllVirtualSensorsArray[STTS22H_VS_ID].SampleDeltaDateTime = 0;
        }

        if (AllVirtualSensorsArray[STTS22H_VS_ID].MinDeltaDateTime != 0)
        {
#ifdef ATRBLE1_VERBOSE_OUTPUT
          ATRBLE1_PRINTF("\tSave Min Value for STS22H=%f\r\n",
                         STTS22H_CODED_TO_SAMPLE(AllVirtualSensorsArray[STTS22H_VS_ID].MinValue.Ui16Value));
#else /* ATRBLE1_VERBOSE_OUTPUT */
          ATRBLE1_PRINTF("\tSave Min Value for STS22H\r\n");
#endif /* ATRBLE1_VERBOSE_OUTPUT */

          DataBuf32 = STNFC_ToShortDeltaDateTime(AllVirtualSensorsArray[STTS22H_VS_ID].MinDeltaDateTime) |
                      (((uint32_t)AllVirtualSensorsArray[STTS22H_VS_ID].MinValue.Ui16Value) << 20);
          if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32,
                                   AllVirtualSensorsArray[STTS22H_VS_ID].MinPositionPointer, 4) != NFCTAG_OK)
          {
            Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
          }
          AllVirtualSensorsArray[STTS22H_VS_ID].MinDeltaDateTime = 0;
        }

        if (AllVirtualSensorsArray[STTS22H_VS_ID].MaxDeltaDateTime != 0)
        {
#ifdef ATRBLE1_VERBOSE_OUTPUT
          ATRBLE1_PRINTF("\tSave Max Value for STS22H=%f\r\n",
                         STTS22H_CODED_TO_SAMPLE(AllVirtualSensorsArray[STTS22H_VS_ID].MaxValue.Ui16Value));
#else /* ATRBLE1_VERBOSE_OUTPUT */
          ATRBLE1_PRINTF("\tSave Max Value for STS22H\r\n");
#endif /* ATRBLE1_VERBOSE_OUTPUT */

          DataBuf32 = STNFC_ToShortDeltaDateTime(AllVirtualSensorsArray[STTS22H_VS_ID].MaxDeltaDateTime) |
                      (((uint32_t)AllVirtualSensorsArray[STTS22H_VS_ID].MaxValue.Ui16Value) << 20);
          if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32,
                                   AllVirtualSensorsArray[STTS22H_VS_ID].MaxPositionPointer, 4) != NFCTAG_OK)
          {
            Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
          }
          AllVirtualSensorsArray[STTS22H_VS_ID].MaxDeltaDateTime = 0;
        }

      }
    }

    ST25_RETRY(BSP_NFCTAG_ResetRFSleep_Dyn(BSP_NFCTAG_INSTANCE));
#ifdef ATRBLE1_VERBOSE_OUTPUT
    ATRBLE1_PRINTF("\t-->WakeUp RF\r\n");
#endif /* ATRBLE1_VERBOSE_OUTPUT */

  }
}



/**
  * @brief Understanding the MEMS interrupt Event
  * @param None
  * @retval None
  */
void AppDetectMemsEvent(void)
{
  if (AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable | AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Enable)
  {
    BSP_MOTION_SENSOR_Event_Status_t EventStatus;
    BSP_MOTION_SENSOR_Get_Event_Status(LSM6DSV16X_0, &EventStatus);

    if ((AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable != 0) &
        (EventStatus.WakeUpStatus != 0))
    {
      int32_t AccEventVmax = DetectorValueForAccEvent();
      if (AccEventVmax > AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Th1.Ui16Value)
      {
        uint16_t ValueToCheck;
        ReadSensorAndLog |= ASYNC_EVENT;
        ValueToCheck = AccEventVmax;
        ATRBLE1_PRINTF("WakeUp=%d\r\n", ValueToCheck);
        /* Check the Value respect Min and Max Limit Values*/
        MCR_STNFC_COMPARE_WITH_LIMITS(Ui16, AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID], ValueToCheck);
        /* Compare with Ths and Update the Max/Min Sample Value */
        STNFC_ComputeMaxMinCompareTHsUi16t(&AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID], &LogDefinition, &hrtc);
      }
    }

    if (!FirstEventNotApplicable)
    {
      if ((AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Enable != 0) &
          (EventStatus.D6DOrientationStatus != 0))
      {
        static uint8_t LastSendOrientation = ORIENTATION_UNDEF;
        uint8_t STBoxPosition = Understand6DOrientation();
        uint8_t ValueToCheck = STBoxPosition;

        ATRBLE1_PRINTF("6D Orientation=%d\r\n", STBoxPosition);
        if (STBoxPosition != LastSendOrientation)
        {
          ReadSensorAndLog |= ASYNC_EVENT;
          LastSendOrientation = STBoxPosition;
          /* Check the Value respect Min and Max Limit Values*/
          MCR_STNFC_COMPARE_WITH_LIMITS(Ui8, AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID], ValueToCheck);
          /* Compare with Ths and Update the Max/Min Sample Value */
          STNFC_ComputeMaxMinCompareTHsUi8t(&AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID], &LogDefinition, &hrtc);
        }
      }
    }

    FirstEventNotApplicable = 0;

    /* Reset FIFO by setting FIFO mode to Bypass */
    BSP_MOTION_SENSOR_FIFO_Set_Mode(LSM6DSV16X_0, LSM6DSV16X_BYPASS_MODE);

    /* Set again FIFO in Continuous to FIFO mode */
    BSP_MOTION_SENSOR_FIFO_Set_Mode(LSM6DSV16X_0, LSM6DSV16X_STREAM_TO_FIFO_MODE);
  }

  if (AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable)
  {

    lsm6dsv16x_all_sources_t      status;
    lsm6dsv16x_all_sources_get(LSM6DSV16X_CONTEX, &status);

    if (status.mlc1)
    {
      lsm6dsv16x_mlc_out_t mlc_out;
      uint16_t Angle;
      lsm6dsv16x_mlc_out_get(LSM6DSV16X_CONTEX, &mlc_out);
      Angle = ((uint16_t)mlc_out.mlc1_src) * 6;
#ifdef ATRBLE1_VERBOSE_OUTPUT
      ATRBLE1_PRINTF("MLC Tilt =%d'\r\n", Angle);
#endif /* ATRBLE1_VERBOSE_OUTPUT */

      /* Check the Value respect Min and Max Limit Values*/
      MCR_STNFC_COMPARE_WITH_LIMITS(Ui8, AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID], Angle);

      /* Compare with Ths and Update the Max/Min Sample Value */
      STNFC_ComputeMaxMinCompareTHsUi8t(&AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID], &LogDefinition, &hrtc);
      if (AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].SampleDeltaDateTime != 0)
      {
        ReadSensorAndLog |= ASYNC_EVENT;
      }
    }
  }
}

/**
  * @brief  Detects actual accelerometer 6D orientation
  * @param  None
  * @retval uint8_t Orientation
  */
static uint8_t Understand6DOrientation(void)
{
  uint8_t xl;
  uint8_t yl;
  uint8_t zl;
  uint8_t xh;
  uint8_t yh;
  uint8_t zh;
  uint8_t STBoxPosition;

  /* Read the Orientation registers */
  BSP_MOTION_SENSOR_Get_6D_Orientation_XL(LSM6DSV16X_0, &xl);
  BSP_MOTION_SENSOR_Get_6D_Orientation_YL(LSM6DSV16X_0, &yl);
  BSP_MOTION_SENSOR_Get_6D_Orientation_ZL(LSM6DSV16X_0, &zl);
  BSP_MOTION_SENSOR_Get_6D_Orientation_XH(LSM6DSV16X_0, &xh);
  BSP_MOTION_SENSOR_Get_6D_Orientation_YH(LSM6DSV16X_0, &yh);
  BSP_MOTION_SENSOR_Get_6D_Orientation_ZH(LSM6DSV16X_0, &zh);

  /* Understand the Orientation */
  if (xl == 1 && yl == 0 && zl == 0 && xh == 0 && yh == 0 && zh == 0)
  {
    STBoxPosition = ORIENTATION_UP;
  }
  else if (xl == 0 && yl == 1 && zl == 0 && xh == 0 && yh == 0 && zh == 0)
  {
    STBoxPosition = ORIENTATION_RIGHT;
  }
  else if (xl == 0 && yl == 0 && zl == 0 && xh == 0 && yh == 1 && zh == 0)
  {
    STBoxPosition = ORIENTATION_LEFT;
  }
  else if (xl == 0 && yl == 0 && zl == 0 && xh == 1 && yh == 0 && zh == 0)
  {
    STBoxPosition = ORIENTATION_DOWN;
  }
  else if (xl == 0 && yl == 0 && zl == 0 && xh == 0 && yh == 0 && zh == 1)
  {
    STBoxPosition = ORIENTATION_TOP;
  }
  else if (xl == 0 && yl == 0 && zl == 1 && xh == 0 && yh == 0 && zh == 0)
  {
    STBoxPosition = ORIENTATION_BOTTOM;
  }
  else
  {
    STBoxPosition = ORIENTATION_UNDEF;
  }

  return STBoxPosition;
}


/**
  * @brief  Detector of the vector norm approximation in mg value for accelerometer event
  * @param  None
  * @retval return new Max Acc Value
  */
static int32_t DetectorValueForAccEvent(void)
{
  int i;
  BSP_MOTION_SENSOR_Axes_t AccValue_XYZ_mg;
  int32_t AccNorm;
  int32_t AccEventVmax = 0;

  uint16_t NumSamples;
  {
    uint8_t fifo_full_status = 0;
    while (fifo_full_status == 0)
    {
      BSP_MOTION_SENSOR_FIFO_Get_Full_Status(LSM6DSV16X_0, &fifo_full_status);
      HAL_Delay(1);
    }
  }

  /* Reads the FIFO status */
  BSP_MOTION_SENSOR_FIFO_Get_Num_Samples(LSM6DSV16X_0, &NumSamples);

  /* ATRBLE1_PRINTF("NumSamples=%d\r\n",NumSamples); */

  /* Read all the FIFO samples (they should be 32 at this point) */
  for (i = 0; i < NumSamples; i++)
  {
    BSP_MOTION_SENSOR_FIFO_Get_Axes(LSM6DSV16X_0, MOTION_ACCELERO, &AccValue_XYZ_mg);
    /* ATRBLE1_PRINTF("\t%d %d %d\r\n",AccValue_XYZ_mg.x,AccValue_XYZ_mg.y,AccValue_XYZ_mg.z); */

    AccNorm = AccNormVectorApproxEvaluator(AccValue_XYZ_mg);

    if (AccNorm > AccEventVmax)
    {
      AccEventVmax = AccNorm;
    }
  }

  ATRBLE1_PRINTF("AccEventVmax=%d\r\n", AccEventVmax);

  return AccEventVmax;
}

/**
  * @brief  Evaluates accelerometer normalization vector approximation in mg value
  * @param  BSP_MOTION_SENSOR_Axes_t Value_XYZ_mg
  * @retval int32_t Normalization value
  */
static int32_t AccNormVectorApproxEvaluator(BSP_MOTION_SENSOR_Axes_t Value_XYZ_mg)
{
#define DATA_ACC_MIN     0
#define DATA_ACC_MAX 16000

  int32_t Ax, Ay, Az, SumABS, MaxABS;
  int32_t AccNormVector;

  Ax =  Value_XYZ_mg.x;
  if (Ax < 0)
  {
    Ax = -Ax;
  }
  MaxABS = Ax;

  Ay =  Value_XYZ_mg.y;
  if (Ay < 0)
  {
    Ay = -Ay;
  }
  if (Ay > MaxABS)
  {
    MaxABS = Ay;
  }

  Az =  Value_XYZ_mg.z;
  if (Az < 0)
  {
    Az = -Az;
  }
  if (Az > MaxABS)
  {
    MaxABS = Az;
  }

  SumABS = Ax + Ay + Az;

  /* Vector norm approximation in mg (error +/-7.5%) */
  AccNormVector = (45 * SumABS + 77 * MaxABS) >> 7;

  if (AccNormVector < (DATA_ACC_MIN))
  {
    AccNormVector = DATA_ACC_MIN;
  }

  if (AccNormVector > (DATA_ACC_MAX))
  {
    AccNormVector = DATA_ACC_MAX;
  }

  return AccNormVector;
}

/**
  * @brief  Set and Save the Default log configuration
  * @param  None
  * @retval None
  */
static void SaveDefaultConfiguration(void)
{
  uint32_t DataBuf32;
  uint8_t *DataBuf8 = (uint8_t *)&DataBuf32;
  uint16_t *DataBuf16 = (uint16_t *)&DataBuf32;

  ATRBLE1_PRINTF("SaveDefaultConfiguration\r\n");

  /* Initialize the Protocol header */
  SmarTNFCHeader.ProtVersion  = ATRBLE1_RECORD_VERSION;
  SmarTNFCHeader.ProtRevision = ATRBLE1_RECORD_REVISION;
  if (FinishGood == FINISHA)
  {
    SmarTNFCHeader.BoardId      = ATRBLE1A_NFC_BOARD_ID;
    SmarTNFCHeader.FirmwareId   = ATRBLE1A_NFC_FW_ID;
  }
  else 
  {
    SmarTNFCHeader.BoardId      = ATRBLE1B_NFC_BOARD_ID;
    SmarTNFCHeader.FirmwareId   = ATRBLE1B_NFC_FW_ID;
  }

  DataBuf8[0] = SmarTNFCHeader.ProtVersion;
  DataBuf8[1] = SmarTNFCHeader.ProtRevision;
  DataBuf8[2] = SmarTNFCHeader.BoardId;
  DataBuf8[3] = SmarTNFCHeader.FirmwareId;

  /* Save Protocol/Board and Fw Id */
  LogDefinition.LastSamplePointer = ATRBLE1_START_ADDR_OFFSET;
  if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
  {
    Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
  }
  /* Update Last Sample Pointer (pointer to next Sample...) */
  LogDefinition.LastSamplePointer += 4;


  /* Initialize the Log Header */
  LogDefinition.SampleTime = DATA_DEFAULT_SAMPLE_INT; /* seconds */
  LogDefinition.StartDateTime = ATRBLE1_DEFAULT_EPOCH_START_TIME; /* No Date/Time Present */
  LogDefinition.RFU = 0; /* Not Used */
  LogDefinition.VirtualSensorsNum = 4;

  /* Write Virtual Sensor Number and Polling Sample Time */
  DataBuf8[0] = LogDefinition.RFU;
  DataBuf8[1] = LogDefinition.VirtualSensorsNum;
  DataBuf16[1] = LogDefinition.SampleTime;
  if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
  {
    Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
  }
  LogDefinition.LastSamplePointer += 4;

  /* Write TimeStamp==0 meaning Default Configuration */
  DataBuf32 = LogDefinition.StartDateTime;
  if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
  {
    Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
  }
  LogDefinition.LastSamplePointer += 4;

  /* Initialize the Default virtual sensors */
  /* Number 0 */
  AllVirtualSensorsArray[STTS22H_VS_ID].Enable = 1;
  ConfiguratedVirtualSensorsArray[0] = &AllVirtualSensorsArray[STTS22H_VS_ID];

  /* Number 1 */
  AllVirtualSensorsArray[LPS22DF_VS_ID].Enable = 1;
  ConfiguratedVirtualSensorsArray[1] = &AllVirtualSensorsArray[LPS22DF_VS_ID];

  /* Number 2 */
  AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Enable = 1;
  ConfiguratedVirtualSensorsArray[2] = &AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID];

  /* Number 3 */
  /* AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable=1;
    ConfiguratedVirtualSensorsArray[3] = &AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID]; */

  /* Number 3 */
  AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable = 1;
  ConfiguratedVirtualSensorsArray[3] = &AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID];

  /* Write Virtual Sensors Configuration */
  SaveVirtualSensorsConfiguration();

  /* Reset the Max/Min For each Sensor */
  ResetMaxMinValuesAllVirtualSensors();

  /* Save the Max/Min for each Sensor */
  SaveMaxMinValuesForVirtualSensors();

  /* Write Sample Counter and Last Sample Pointer*/
  LogDefinition.SampleCounterAddress = LogDefinition.LastSamplePointer;
  LogDefinition.SampleCounter = 0;
  UpdateLastSamplePointerAndSampleCounter(&LogDefinition);
  LogDefinition.LastSamplePointer += 8; /* We use 4bytes for each one */

  /* Set the Date&Time */
  if (STNFC_SetDateTime(LogDefinition.StartDateTime, &hrtc, &LogDefinition) == 0)
  {
    ATRBLE1_PRINTF("Error: Setting RTC\r\n");
  }
  else
  {
    ATRBLE1_PRINTF("Set RTC Date&Time\r\n");
  }

  NfcType5_SetInitialNDEFPayLoadLengthValue(LogDefinition.LastSamplePointer);
  NfcType5_ComputeNDEFPayLoadSize(LogDefinition.LastSamplePointer, LogDefinition.SampleCounter);

  /* Reset Log Area */
  DeleteLogArea();
  NumberOfSamplesSavedInFlash = 0;

  /* lsm6dsv16x WakeUp/6D Rec or MLC: Init/DeInit */
  InitDeInitAccEventThreshold();

  CurrentLogStatus = STNFC_LOG_RUNNING;
  LogStartPeriodicallyTimer();
}

/**
  * @brief  Set all Avaialbles Virtual Sensor Configuration
  * @param  None
  * @retval None
  */
static void SetAllAvailableVirtualSensors(void)
{
  /* Initialize the virtual sensors */
  ATRBLE1_PRINTF("Set All Available Virtual Sensors\r\n");

  /* Number 0 */
  AllVirtualSensorsArray[STTS22H_VS_ID].VirtualSensorId = STTS22H_VS_ID;
  AllVirtualSensorsArray[STTS22H_VS_ID].Enable = 0;
  AllVirtualSensorsArray[STTS22H_VS_ID].SensorType = VST_UI16;
  AllVirtualSensorsArray[STTS22H_VS_ID].ThsUsageType = TH_INT;
  /* 'C */
  AllVirtualSensorsArray[STTS22H_VS_ID].Th1.Ui16Value = STTS22H_SAMPLE_TO_CODED(22);
  AllVirtualSensorsArray[STTS22H_VS_ID].Th2.Ui16Value = STTS22H_SAMPLE_TO_CODED(43.2);
  AllVirtualSensorsArray[STTS22H_VS_ID].MaxLimit.Ui16Value = STTS22H_SAMPLE_TO_CODED(60);
  AllVirtualSensorsArray[STTS22H_VS_ID].MinLimit.Ui16Value = STTS22H_SAMPLE_TO_CODED(-10);

  /* Number 1 */
  AllVirtualSensorsArray[LPS22DF_VS_ID].VirtualSensorId = LPS22DF_VS_ID;
  AllVirtualSensorsArray[LPS22DF_VS_ID].Enable = 0;
  AllVirtualSensorsArray[LPS22DF_VS_ID].SensorType = VST_UI16;
  AllVirtualSensorsArray[LPS22DF_VS_ID].ThsUsageType = TH_BIGGER;
  /* hPa */
  AllVirtualSensorsArray[LPS22DF_VS_ID].Th1.Ui16Value = LPS22DF_SAMPLE_TO_CODED(960);
  AllVirtualSensorsArray[LPS22DF_VS_ID].MaxLimit.Ui16Value = LPS22DF_SAMPLE_TO_CODED(1260);
  AllVirtualSensorsArray[LPS22DF_VS_ID].MinLimit.Ui16Value = LPS22DF_SAMPLE_TO_CODED(260);

  /* Number 2 */
  AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].VirtualSensorId = LSM6DSV16X_WU_VS_ID;
  AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable = 0;
  AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].SensorType = VST_UI16;
  AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].ThsUsageType = TH_BIGGER;
  /* mg */
  AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Th1.Ui16Value = 4096;
  AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxLimit.Ui16Value = 16000;
  AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MinLimit.Ui16Value = 0;

  /* Number 3 */
  AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].VirtualSensorId = LSM6DSV16X_6D_VS_ID;
  AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Enable = 0;
  AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].SensorType = VST_UI8;
  /* In this way we save the events when the board position is != from UP */
  AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].ThsUsageType = TH_EXT;
  /* position */
  AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Th1.Ui8Value = ORIENTATION_RIGHT;
  AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Th2.Ui8Value = ORIENTATION_LEFT;
  AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].MaxLimit.Ui8Value = ORIENTATION_DOWN;
  AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].MinLimit.Ui8Value = ORIENTATION_UNDEF;

  /* Number 4 */
  AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].VirtualSensorId = LSM6DSV16X_MLC_VS_ID;
  AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable = 0;
  AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].SensorType = VST_UI8;
  AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].ThsUsageType = TH_INT;
  /* Degree Angle */
  AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Th1.Ui8Value = 30;
  AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Th2.Ui8Value = 50;
  AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].MaxLimit.Ui8Value = 90;
  AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].MinLimit.Ui8Value = 0;
}

/**
  * @brief  Save the Virtual Sensors Configurations
  * @param  None
  * @retval None
  */
static void SaveVirtualSensorsConfiguration(void)
{
  int32_t SensorNum;
  uint32_t DataBuf32;

  ATRBLE1_PRINTF("SaveVirtualSensorsConfiguration\r\n");

  for (SensorNum = 0; SensorNum < LogDefinition.VirtualSensorsNum; SensorNum++)
  {

    if (ConfiguratedVirtualSensorsArray[SensorNum] == &AllVirtualSensorsArray[STTS22H_VS_ID])
    {

      DataBuf32 = STTS22H_VS_ID |
                  (((uint32_t)AllVirtualSensorsArray[STTS22H_VS_ID].ThsUsageType) << 3) |
                  (((uint32_t)AllVirtualSensorsArray[STTS22H_VS_ID].Th1.Ui16Value) << (3 + 2)) |
                  (((uint32_t)AllVirtualSensorsArray[STTS22H_VS_ID].Th2.Ui16Value) << (3 + 2 + 9));
      if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
      {
        Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
      }
      LogDefinition.LastSamplePointer += 4;

      ATRBLE1_PRINTF("Save STTS22H_VS_ID:\r\n");
      ATRBLE1_PRINTF("\tThsUsageType=%s\r\n", ThresholdsUsageName[ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType]);
      ATRBLE1_PRINTF("\tTh1.Ui16Value=%f\r\n",
                     STTS22H_CODED_TO_SAMPLE(ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui16Value));
      if (ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType < TH_LESS)
      {
        ATRBLE1_PRINTF("\tTh2.Ui16Value=%f\r\n",
                       STTS22H_CODED_TO_SAMPLE(ConfiguratedVirtualSensorsArray[SensorNum]->Th2.Ui16Value));
      }

    }
    else if (ConfiguratedVirtualSensorsArray[SensorNum] == &AllVirtualSensorsArray[LPS22DF_VS_ID])
    {

      DataBuf32 = LPS22DF_VS_ID |
                  (((uint32_t)AllVirtualSensorsArray[LPS22DF_VS_ID].ThsUsageType) << 3) |
                  (((uint32_t)AllVirtualSensorsArray[LPS22DF_VS_ID].Th1.Ui16Value) << (3 + 2)) |
                  (((uint32_t)AllVirtualSensorsArray[LPS22DF_VS_ID].Th2.Ui16Value) << (3 + 2 + 11));
      if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
      {
        Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
      }
      LogDefinition.LastSamplePointer += 4;

      ATRBLE1_PRINTF("Save LPS22DF_VS_ID:\r\n");
      ATRBLE1_PRINTF("\tThsUsageType=%s\r\n", ThresholdsUsageName[ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType]);
      ATRBLE1_PRINTF("\tTh1.Ui16Value=%f\r\n",
                     LPS22DF_CODED_TO_SAMPLE(ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui16Value));
      if (ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType < TH_LESS)
      {
        ATRBLE1_PRINTF("\tTh2.Ui16Value=%f\r\n",
                       LPS22DF_CODED_TO_SAMPLE(ConfiguratedVirtualSensorsArray[SensorNum]->Th2.Ui16Value));
      }

    }
    else if (ConfiguratedVirtualSensorsArray[SensorNum] == &AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID])
    {

      DataBuf32 = LSM6DSV16X_WU_VS_ID |
                  (((uint32_t)AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].ThsUsageType) << 3) |
                  (((uint32_t)AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Th1.Ui16Value) << (3 + 2));
      if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
      {
        Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
      }
      LogDefinition.LastSamplePointer += 4;

      ATRBLE1_PRINTF("Save LSM6DSV16X_WU_VS_ID:\r\n");
      ATRBLE1_PRINTF("\tThsUsageType=%s\r\n", ThresholdsUsageName[ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType]);
      ATRBLE1_PRINTF("\tTh1.Ui16Value=%d\r\n", ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui16Value);

    }
    else if (ConfiguratedVirtualSensorsArray[SensorNum] == &AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID])
    {

      DataBuf32 = LSM6DSV16X_6D_VS_ID |
                  (((uint32_t)AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].ThsUsageType) << 3) |
                  (((uint32_t)AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Th1.Ui8Value) << (3 + 2)) |
                  (((uint32_t)AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Th2.Ui8Value) << (3 + 2 + 8));
      if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
      {
        Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
      }
      LogDefinition.LastSamplePointer += 4;

      ATRBLE1_PRINTF("Save LSM6DSV16X_6D_VS_ID:\r\n");
      ATRBLE1_PRINTF("\tThsUsageType=%s\r\n", ThresholdsUsageName[ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType]);
      ATRBLE1_PRINTF("\tTh1.Ui8Value=%d\r\n", ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui8Value);
      if (ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType < TH_LESS)
      {
        ATRBLE1_PRINTF("\tTh2.Ui8Value=%d\r\n", ConfiguratedVirtualSensorsArray[SensorNum]->Th2.Ui8Value);
      }

    }
    else if (ConfiguratedVirtualSensorsArray[SensorNum] == &AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID])
    {

      DataBuf32 = LSM6DSV16X_MLC_VS_ID |
                  (((uint32_t)AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].ThsUsageType) << 3) |
                  (((uint32_t)AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Th1.Ui8Value) << (3 + 2)) |
                  (((uint32_t)AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Th2.Ui8Value) << (3 + 2 + 8));
      if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
      {
        Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
      }
      LogDefinition.LastSamplePointer += 4;

      ATRBLE1_PRINTF("Save LSM6DSV16X_MLC_VS_ID:\r\n");
      ATRBLE1_PRINTF("\tThsUsageType=%s\r\n", ThresholdsUsageName[ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType]);
      ATRBLE1_PRINTF("\tTh1.Ui8Value=%d\r\n", ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui8Value);
      if (ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType < TH_LESS)
      {
        ATRBLE1_PRINTF("\tTh2.Ui8Value=%d\r\n", ConfiguratedVirtualSensorsArray[SensorNum]->Th2.Ui8Value);
      }
    }
  }
}


/**
  * @brief  Reset the Max/Min Values saved in NFC after a Delete Log Command from BLE PnP-L command
  * @param  None
  * @retval None
  */
static void BleResetSaveMinValuesForVirtualSensors(void)
{
  uint32_t DataBuf32;
  ATRBLE1_PRINTF("BleResetSaveMinValuesForVirtualSensors\r\n");

  if (AllVirtualSensorsArray[STTS22H_VS_ID].Enable)
  {
    AllVirtualSensorsArray[STTS22H_VS_ID].MinValue.Ui16Value = STTS22H_SAMPLE_TO_CODED(60); /* 'C */
    AllVirtualSensorsArray[STTS22H_VS_ID].MaxValue.Ui16Value = STTS22H_SAMPLE_TO_CODED(-10); /* 'C */
    AllVirtualSensorsArray[STTS22H_VS_ID].MaxDeltaDateTime = 0;
    AllVirtualSensorsArray[STTS22H_VS_ID].MinDeltaDateTime = 0;

    DataBuf32 = (0 /* ShortDeltaTime */) | (((uint32_t)AllVirtualSensorsArray[STTS22H_VS_ID].MinValue.Ui16Value) << 20);
    if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32,
                             AllVirtualSensorsArray[STTS22H_VS_ID].MinPositionPointer, 4) != NFCTAG_OK)
    {
      Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
    }

    DataBuf32 = (0 /* ShortDeltaTime */) | (((uint32_t)AllVirtualSensorsArray[STTS22H_VS_ID].MaxValue.Ui16Value) << 20);
    if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32,
                             AllVirtualSensorsArray[STTS22H_VS_ID].MaxPositionPointer, 4) != NFCTAG_OK)
    {
      Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
    }
  }

  if (AllVirtualSensorsArray[LPS22DF_VS_ID].Enable)
  {
    AllVirtualSensorsArray[LPS22DF_VS_ID].MinValue.Ui16Value = LPS22DF_SAMPLE_TO_CODED(1260); /* hPa */
    AllVirtualSensorsArray[LPS22DF_VS_ID].MaxValue.Ui16Value = LPS22DF_SAMPLE_TO_CODED(260); /* hPa */
    AllVirtualSensorsArray[LPS22DF_VS_ID].MaxDeltaDateTime = 0;
    AllVirtualSensorsArray[LPS22DF_VS_ID].MinDeltaDateTime = 0;

    DataBuf32 = (0 /* ShortDeltaTime */) | (((uint32_t)AllVirtualSensorsArray[LPS22DF_VS_ID].MinValue.Ui16Value) << 20);
    if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32,
                             AllVirtualSensorsArray[LPS22DF_VS_ID].MinPositionPointer, 4) != NFCTAG_OK)
    {
      Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
    }

    DataBuf32 = (0 /* ShortDeltaTime */) | (((uint32_t)AllVirtualSensorsArray[LPS22DF_VS_ID].MaxValue.Ui16Value) << 20);
    if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32,
                             AllVirtualSensorsArray[LPS22DF_VS_ID].MaxPositionPointer, 4) != NFCTAG_OK)
    {
      Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
    }
  }

  if (AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable)
  {
    AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxValue.Ui16Value = 0; /* mg */
    AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxDeltaDateTime = 0;
    AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MinDeltaDateTime = 0;

    DataBuf32 = (0 /* ShortDeltaTime */) ;
    if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32,
                             AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxPositionPointer, 4) != NFCTAG_OK)
    {
      Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
    }
    DataBuf32 = AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxValue.Ui16Value;
    if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32,
                             AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxPositionPointer + 4, 4) != NFCTAG_OK)
    {
      Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
    }
  }

  /* 6D Orientation and MLC Tilt have not Max/Min Section */
}

/**
  * @brief  Reset the Max/Min Values
  * @param  None
  * @retval None
  */
static void ResetMaxMinValuesAllVirtualSensors(void)
{
  ATRBLE1_PRINTF("ResetMaxMinValuesAllVirtualSensors\r\n");

  if (AllVirtualSensorsArray[STTS22H_VS_ID].Enable)
  {
    AllVirtualSensorsArray[STTS22H_VS_ID].MinValue.Ui16Value = STTS22H_SAMPLE_TO_CODED(60); /* 'C */
    AllVirtualSensorsArray[STTS22H_VS_ID].MaxValue.Ui16Value = STTS22H_SAMPLE_TO_CODED(-10); /* 'C */
    AllVirtualSensorsArray[STTS22H_VS_ID].MaxDeltaDateTime = 0;
    AllVirtualSensorsArray[STTS22H_VS_ID].MinDeltaDateTime = 0;
  }

  if (AllVirtualSensorsArray[LPS22DF_VS_ID].Enable)
  {
    AllVirtualSensorsArray[LPS22DF_VS_ID].MinValue.Ui16Value = LPS22DF_SAMPLE_TO_CODED(1260); /* hPa */
    AllVirtualSensorsArray[LPS22DF_VS_ID].MaxValue.Ui16Value = LPS22DF_SAMPLE_TO_CODED(260); /* hPa */
    AllVirtualSensorsArray[LPS22DF_VS_ID].MaxDeltaDateTime = 0;
    AllVirtualSensorsArray[LPS22DF_VS_ID].MinDeltaDateTime = 0;
  }

  if (AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable)
  {
    AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxValue.Ui16Value = 0; /* mg */
    AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxDeltaDateTime = 0;
    AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MinDeltaDateTime = 0;
  }

  /* 6D Orientation and MLC Tilt have not Max/Min Section */
}


/**
  * @brief  Save the Max/Min Values
  * @param  None
  * @retval None
  */
static void SaveMaxMinValuesForVirtualSensors(void)
{
  int32_t SensorNum;
  uint32_t DataBuf32;

  ATRBLE1_PRINTF("SaveMaxMinValuesForVirtualSensors\r\n");

  for (SensorNum = 0; SensorNum < LogDefinition.VirtualSensorsNum; SensorNum++)
  {

    if (ConfiguratedVirtualSensorsArray[SensorNum] == &AllVirtualSensorsArray[STTS22H_VS_ID])
    {

      AllVirtualSensorsArray[STTS22H_VS_ID].MinPositionPointer = LogDefinition.LastSamplePointer;
      DataBuf32 = (0 /* ShortDeltaTime */) | (((uint32_t)AllVirtualSensorsArray[STTS22H_VS_ID].MinValue.Ui16Value) << 20);
      if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
      {
        Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
      }
      LogDefinition.LastSamplePointer += 4;

      AllVirtualSensorsArray[STTS22H_VS_ID].MaxPositionPointer = LogDefinition.LastSamplePointer;
      DataBuf32 = (0 /* ShortDeltaTime */) | (((uint32_t)AllVirtualSensorsArray[STTS22H_VS_ID].MaxValue.Ui16Value) << 20);
      if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
      {
        Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
      }
      LogDefinition.LastSamplePointer += 4;

    }
    else if (ConfiguratedVirtualSensorsArray[SensorNum] == &AllVirtualSensorsArray[LPS22DF_VS_ID])
    {

      AllVirtualSensorsArray[LPS22DF_VS_ID].MinPositionPointer = LogDefinition.LastSamplePointer;
      DataBuf32 = (0 /* ShortDeltaTime */) | (((uint32_t)AllVirtualSensorsArray[LPS22DF_VS_ID].MinValue.Ui16Value) << 20);
      if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
      {
        Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
      }
      LogDefinition.LastSamplePointer += 4;

      AllVirtualSensorsArray[LPS22DF_VS_ID].MaxPositionPointer = LogDefinition.LastSamplePointer;
      DataBuf32 = (0 /* ShortDeltaTime */) | (((uint32_t)AllVirtualSensorsArray[LPS22DF_VS_ID].MaxValue.Ui16Value) << 20);
      if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
      {
        Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
      }
      LogDefinition.LastSamplePointer += 4;

    }
    else if (ConfiguratedVirtualSensorsArray[SensorNum] == &AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID])
    {

      AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxPositionPointer = LogDefinition.LastSamplePointer;
      DataBuf32 = (0 /* ShortDeltaTime */) ;
      if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
      {
        Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
      }
      LogDefinition.LastSamplePointer += 4;

      DataBuf32 = AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxValue.Ui16Value;
      if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
      {
        Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
      }
      LogDefinition.LastSamplePointer += 4;
    }

    /* 6D Orientation and MLC Tilt have not Max/Min Section */

  }
}

/**
  * @brief  Save the Last Sample Pointer and Sample Counter
  * @param  SNFC_LogDefinition_t *LogDefinition Pointer to Log definition structure
  * @retval None
  */
static void UpdateLastSamplePointerAndSampleCounter(SNFC_LogDefinition_t *LogDefinition)
{
  uint32_t DataBuf32;

  /* Write Sample Counter */
  DataBuf32 = LogDefinition->SampleCounter;
  if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition->SampleCounterAddress,
                           4) != NFCTAG_OK)
  {
    Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
  }

  /*  Write LastSamplePointer */
  DataBuf32 = LogDefinition->LastSamplePointer;
  if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition->SampleCounterAddress + 4,
                           4) != NFCTAG_OK)
  {
    Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
  }
}



/**
  * @brief  Accelerometer events initialization
  * @retval None
  */
static int32_t InitDeInitAccEventThreshold(void)
{
  int32_t Success = BSP_ERROR_NONE;

  static uint8_t FirstTimeLSM6DSV16X = 0;

  if (AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable |
      AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Enable |
      AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable)
  {
    ATRBLE1_PRINTF("Enable Accelerometer Events\r\n");
    EnableInterruptFromMems();

    if (FirstTimeLSM6DSV16X)
    {
      /* This in order to be sure to restart from a clean situation */
      BSP_MOTION_SENSOR_DeInit(LSM6DSV16X_0);
      BSP_MOTION_SENSOR_Init(LSM6DSV16X_0, MOTION_ACCELERO);
    }
    FirstTimeLSM6DSV16X = 1;

  }
  else
  {
    ATRBLE1_PRINTF("Disable Accelerometer Events\r\n");
    DisableInterruptFromMems();
  }

  /* Enable 6D orientation */
  if (AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Enable)
  {

    FirstEventNotApplicable = 1;

    /* Enable 6D Orientation */
    ATRBLE1_PRINTF("6D On\r\n");
    Success = BSP_MOTION_SENSOR_Enable_6D_Orientation(LSM6DSV16X_0, BSP_MOTION_SENSOR_INT1_PIN);
    if (Success != BSP_ERROR_NONE)
    {
      ATRBLE1_PRINTF("\r\nError enabling 6D\r\n");
      return Success;
    }
  }
  else
  {
    /* Disable 6D Orientation */
    ATRBLE1_PRINTF("6D Off\r\n");
    Success = BSP_MOTION_SENSOR_Disable_6D_Orientation(LSM6DSV16X_0);
    if (Success != BSP_ERROR_NONE)
    {
      ATRBLE1_PRINTF("\r\nError Disabling 6D\r\n");
      return Success;
    }
  }


  /* Enable wake up events */
  if (AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable)
  {
    /* Enable Wake Up */
    uint32_t Th_Max;
    ATRBLE1_PRINTF("WakeUp On\r\n");
    Success = BSP_MOTION_SENSOR_Enable_Wake_Up_Detection(LSM6DSV16X_0, BSP_MOTION_SENSOR_INT1_PIN);

    if (Success != BSP_ERROR_NONE)
    {
      ATRBLE1_PRINTF("\r\nError enabling WakeUp\r\n");
      return Success;
    }

    /* FullScale 16G */
    Success = BSP_MOTION_SENSOR_SetFullScale(LSM6DSV16X_0, MOTION_ACCELERO, 16);
    if (Success != BSP_ERROR_NONE)
    {
      ATRBLE1_PRINTF("\r\nError Setting Full Scale\r\n");
      return Success;
    }

    /* Set the Threshold */
    Th_Max = AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Th1.Ui16Value;

#ifdef ATRBLE1_VERBOSE_OUTPUT
    ATRBLE1_PRINTF("Acc_Th_Max= %ld\r\n", Th_Max);
#endif /* ATRBLE1_VERBOSE_OUTPUT */

    Success = BSP_MOTION_SENSOR_Set_Wake_Up_Threshold(LSM6DSV16X_0, Th_Max);
    if (Success != BSP_ERROR_NONE)
    {
      ATRBLE1_PRINTF("\r\nError Setting WakeUp Threshold\r\n");
      return Success;
    }
  }
  else
  {
    /* Disable Wake Up */
    ATRBLE1_PRINTF("WakeUp Off\r\n");
    Success = BSP_MOTION_SENSOR_Disable_Wake_Up_Detection(LSM6DSV16X_0);
    if (Success != BSP_ERROR_NONE)
    {
      ATRBLE1_PRINTF("\r\nError disabling WakeUp\r\n");
      return Success;
    }
  }

  ATRBLE1_PRINTF("\r\n");

  if ((AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable) |
      (AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Enable))
  {
    /* Set the FIFO in Continuous to FIFO mode */
    float Bdr;
    Success = BSP_MOTION_SENSOR_GetOutputDataRate(LSM6DSV16X_0, MOTION_ACCELERO, &Bdr);
    if (Success != BSP_ERROR_NONE)
    {
      ATRBLE1_PRINTF("\r\nError Reading ODR\r\n");
      return Success;
    }
    Success = BSP_MOTION_SENSOR_FIFO_Set_BDR(LSM6DSV16X_0, MOTION_ACCELERO, Bdr);
    if (Success != BSP_ERROR_NONE)
    {
      ATRBLE1_PRINTF("\r\nError Setting FIFO BDR\r\n");
      return Success;
    }

    /* Reset FIFO */
    Success = BSP_MOTION_SENSOR_FIFO_Set_Mode(LSM6DSV16X_0, LSM6DSV16X_BYPASS_MODE);
    if (Success != BSP_ERROR_NONE)
    {
      ATRBLE1_PRINTF("\r\nError Setting FIFO Mode\r\n");
    }

    /* Set FIFO in Continuous to FIFO mode */
    Success = BSP_MOTION_SENSOR_FIFO_Set_Mode(LSM6DSV16X_0, LSM6DSV16X_STREAM_TO_FIFO_MODE);
    if (Success != BSP_ERROR_NONE)
    {
      ATRBLE1_PRINTF("\r\nError Setting FIFO Mode\r\n");
    }
  }
  else
  {
    /* Set the FIFO Bypass mode */
    Success = BSP_MOTION_SENSOR_FIFO_Set_Mode(LSM6DSV16X_0, LSM6DSV16X_BYPASS_MODE);
    if (Success != BSP_ERROR_NONE)
    {
      ATRBLE1_PRINTF("\r\nError Setting FIFO Mode\r\n");
    }
  }



  /* Enable MCL */
  if (AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable)
  {
    ucf_line_t *ProgramPointer;
    int32_t LineCounter;
    int32_t TotalNumberOfLine;
    int32_t RetValue;

    ProgramPointer = (ucf_line_t *)lsm6dsv16x_tilt_angle_mode0;
    TotalNumberOfLine = sizeof(lsm6dsv16x_tilt_angle_mode0) / sizeof(ucf_line_t);
    for (LineCounter = 0; LineCounter < TotalNumberOfLine; LineCounter++)
    {
      RetValue = BSP_MOTION_SENSOR_Write_Register(LSM6DSV16X_0,
                                                  ProgramPointer[LineCounter].address,
                                                  ProgramPointer[LineCounter].data);
      if (RetValue != BSP_ERROR_NONE)
      {
        ATRBLE1_PRINTF("Error loading the Program to LSM6DSV16X [%ld]->%lx\n\r", LineCounter, RetValue);
        Error_Handler(ATRBLE1_ERROR_HW_INIT, __FILE__, __LINE__);
      }
    }
    ATRBLE1_PRINTF("MLC program Loaded on LSM6DSV16X [%d]\r\n", TotalNumberOfLine);
    ATRBLE1_PRINTF("    Detect angles from 0 to 90 degrees\r\n");
  }

  return Success;
}

/**
  * @brief  Set the NFC Security Level and GPO behavior
  * @param  None
  * @retval None
  */
static void SetNFCBehavior(void)
{
  /* Enable the NFC GPIO Interrupt */
  if (BSP_NFCTAG_GPIO_Init() != BSP_ERROR_NONE)
  {
    ATRBLE1_PRINTF("Error NFC Initializing GPIO\r\n");
    Error_Handler(ATRBLE1_ERROR_HW_INIT, __FILE__, __LINE__);
  }
  else
  {
    ATRBLE1_PRINTF("NFC NFC GPIO Initialized\r\n");
  }

  /* Setting the New Password for I2C protection */
  if (BSP_NFCTAG_ChangeI2CPassword(0x90ABCDEF, 0x12345678) != NFCTAG_OK)
  {
    Error_Handler(ATRBLE1_ERROR_HW_INIT, __FILE__, __LINE__);
  }

  /* GPO sensible to RF Field change */
  if (BSP_NFCTAG_WriteConfigIT(0x90ABCDEF, 0x12345678,
                               ST25DV_GPO_ENABLE_MASK | ST25DV_GPO_FIELDCHANGE_MASK) != NFCTAG_OK)
  {
    Error_Handler(ATRBLE1_ERROR_HW_INIT, __FILE__, __LINE__);
  }
}


/**
  * @brief  Understanding the Interrupt from ST25DV
  * @param  None
  * @retval None
  */
void AppLogDetectRFActivity(void)
{
  uint8_t ITStatus;
  static ST25DV_FIELD_STATUS RFfield = ST25DV_FIELD_OFF;
  uint8_t FieldOn = 0;
  uint8_t FieldOff = 0;

  /* Read the IT status register */
  ST25_RETRY(BSP_NFCTAG_ReadITSTStatus_Dyn(BSP_NFCTAG_INSTANCE, &ITStatus));

  /* Check the Falling Bit */
  if ((ITStatus & ST25DV_ITSTS_DYN_FIELDFALLING_MASK) == ST25DV_ITSTS_DYN_FIELDFALLING_MASK)
  {
    FieldOff = 1;
  }

  /* Check the Rising Bit */
  if ((ITStatus & ST25DV_ITSTS_DYN_FIELDRISING_MASK) == ST25DV_ITSTS_DYN_FIELDRISING_MASK)
  {
    FieldOn = 1;
  }

  /* Make the decision */
  if (((FieldOff == 1) & (FieldOn == 1)) |
      ((FieldOff == 0) & (FieldOn == 0) & (RFfield == ST25DV_FIELD_OFF)))
  {
    /* can't decide, need to read the register to get actual state */
    static ST25DV_FIELD_STATUS field = ST25DV_FIELD_OFF;
    int32_t status = BSP_NFCTAG_GetRFField_Dyn(BSP_NFCTAG_INSTANCE, &field);
    if ((field == ST25DV_FIELD_ON) || (status == NFCTAG_NACK))
    {
      if (FieldOn || FieldOff)
      {
        /* Off->On */
        ATRBLE1_PRINTF("\r\nDetected NFC FIELD Off->On\r\n");
        RFActivityStatus = FIELD_RISING;
        RFfield = ST25DV_FIELD_ON;
      }
    }
    else
    {
      if (FieldOn || FieldOff)
      {
        /* On->Off */
        ATRBLE1_PRINTF("\r\nDetected NFC FIELD On->Off\r\n");
        RFfield = ST25DV_FIELD_OFF;
        RFActivityStatus = FIELD_FALLING;
      }
    }
  }
  else
  {
    if ((FieldOff == 0) & (FieldOn == 1))
    {
      /* On */
      ATRBLE1_PRINTF("\r\nDetected NFC FIELD On\r\n");
      RFActivityStatus = FIELD_RISING;
    }
    else if ((FieldOff == 1) & (FieldOn == 0))
    {
      /* Off */
      ATRBLE1_PRINTF("\r\nDetected NFC FIELD Off\r\n");
      RFActivityStatus = FIELD_FALLING;
    }
  }

  /* When the Phone is no more under the Tag */
  if (RFActivityStatus == FIELD_FALLING)
  {

    /* Control if there is a new configuration */
    ST25_RETRY(BSP_NFCTAG_SetRFSleep_Dyn(BSP_NFCTAG_INSTANCE));
#ifdef ATRBLE1_VERBOSE_OUTPUT
    ATRBLE1_PRINTF("\t-->Sleep RF\r\n");
#endif /* ATRBLE1_VERBOSE_OUTPUT */

    CheckIfNewConfiguration();

    ST25_RETRY(BSP_NFCTAG_ResetRFSleep_Dyn(BSP_NFCTAG_INSTANCE));
#ifdef ATRBLE1_VERBOSE_OUTPUT
    ATRBLE1_PRINTF("\t-->WakeUp RF\r\n");
#endif /* ATRBLE1_VERBOSE_OUTPUT */
  }
}

/**
  * @brief  Checks if there is a new configuration.
  * @param  None
  * @retval None
  */
static void CheckIfNewConfiguration(void)
{
  SNFC_LogDefinition_t LocalLogDefinition;

  ATRBLE1_PRINTF("Check if there is a new Configuration\r\n");
  ReadConfiguration(&LocalLogDefinition, LogDefinition.StartDateTime, 1);

  if (LocalLogDefinition.StartDateTime != 0)
  {
    StartNewLog();
  }
}

/**
  * @brief  Start a New Log
  * @param  None
  * @retval None
  */
void StartNewLog(void)
{
  int32_t SensorNum;
  ATRBLE1_PRINTF("Start a new Log\r\n");

  NumberOfSamplesSavedInFlash = 0;

  /* Disable all the Previous Enabled Virtual Sensors */
  for (SensorNum = 0; SensorNum < LogDefinition.VirtualSensorsNum; SensorNum++)
  {
    ConfiguratedVirtualSensorsArray[SensorNum]->Enable = 0;
    ConfiguratedVirtualSensorsArray[SensorNum]->SampleDeltaDateTime = 0;
    ConfiguratedVirtualSensorsArray[SensorNum]->MinDeltaDateTime = 0;
    ConfiguratedVirtualSensorsArray[SensorNum]->MaxDeltaDateTime = 0;
  }

  /* Read again and update the configuration restarting the log */
  ReadConfiguration(&LogDefinition, 0, 0);

  /* Set the Date&Time */
  if (STNFC_SetDateTime(LogDefinition.StartDateTime, &hrtc, &LogDefinition) == 0)
  {
    ATRBLE1_PRINTF("Error: Setting RTC\r\n");
  }
  else
  {
    ATRBLE1_PRINTF("Set RTC Date&Time\r\n");
  }

  /* Reset the Max/Min For each Sensor */
  ResetMaxMinValuesAllVirtualSensors();

  /* Save the Max/Min for each Sensor */
  SaveMaxMinValuesForVirtualSensors();

  /* Write Sample Counter and Last Sample Pointer*/
  LogDefinition.SampleCounterAddress = LogDefinition.LastSamplePointer;
  LogDefinition.SampleCounter = 0;
  UpdateLastSamplePointerAndSampleCounter(&LogDefinition);
  LogDefinition.LastSamplePointer += 8;

  NfcType5_SetInitialNDEFPayLoadLengthValue(LogDefinition.LastSamplePointer);

  NfcType5_ComputeNDEFPayLoadSize(LogDefinition.LastSamplePointer, LogDefinition.SampleCounter);

  /* Reset Log Area */
  DeleteLogArea();
  NumberOfSamplesSavedInFlash = 0;

  /* lsm6dsv16x WakeUp/6D Rec or MLC: Init/DeInit */
  InitDeInitAccEventThreshold();

  CurrentLogStatus = STNFC_LOG_RUNNING;
  LogStartPeriodicallyTimer();
}

/**
  * @brief  Stop Current Log
  * @param  None
  * @retval None
  */
void StopCurrentLog(void)
{
  ATRBLE1_PRINTF("Stop Current Log\r\n");

  DisableInterruptFromMems();

  /* Stop periodically Timer */
  LogStopPeriodicallyTimer();

  /* Dump latest Values on Flash */
  DumpLogData();

  CurrentLogStatus = STNFC_LOG_STOP;
}

/**
  * @brief  API controlled by BLE for Deleting the Log Saved
  * @param  None
  * @retval None
  */
void BleDeleteCurrentLog(void)
{
  if (CurrentLogStatus == STNFC_LOG_STOP)
  {
    ATRBLE1_PRINTF("Delete Log saved on Flash\r\n");
    /* Reset Log Area */
    DeleteLogArea();
    NumberOfSamplesSavedInFlash = 0;

    /* Reset and Save the Max/Min For each Sensor */
    BleResetSaveMinValuesForVirtualSensors();
  }
}

/**
  * @brief  API controlled by BLE for Pausing the Log
  * @param  None
  * @retval None
  */
void BlePauseCurrentLog(void)
{
  if (CurrentLogStatus == STNFC_LOG_RUNNING)
  {
    ATRBLE1_PRINTF("Put the Log on Pause\r\n");

    DisableInterruptFromMems();

    /* Stop periodically Timer */
    LogStopPeriodicallyTimer();

    CurrentLogStatus = STNFC_LOG_PAUSED;
  }
}

/**
  * @brief  API controlled by BLE for Resuming a paused Log
  * @param  None
  * @retval None
  */
void BleResumeCurrentLog(void)
{
  if (CurrentLogStatus == STNFC_LOG_PAUSED)
  {
    ATRBLE1_PRINTF("Resume the log paused\r\n");

    EnableInterruptFromMems();

    /* Start periodically Timer */
    LogStartPeriodicallyTimer();

    CurrentLogStatus = STNFC_LOG_RUNNING;
  }
}

/**
  * @brief  API controlled by BLE for Reading the Log Saved
  * @param  None
  * @retval None
  */
void BleReadCurrentLog(void)
{
  /* #define DEBUG_BINARY_CONTENT */

#ifdef DEBUG_BINARY_CONTENT
#define READ_NFC_DATA(pointer) {             \
  if(BSP_NFCTAG_ReadData( BSP_NFCTAG_INSTANCE, (uint8_t *) (DataToSend+WritingPointer), pointer, 4 )!=NFCTAG_OK) {\
    Error_Handler(ATRBLE1_ERROR_NFC_READING,__FILE__,__LINE__); \
  }                                         \
   ATRBLE1_PRINTF("| %08X",DataToSend[WritingPointer]);\
   pointer+=4;                               \
   WritingPointer++;                         \
}

#define READ_FLASH_DATA(pointer) {           \
  DataToSend[WritingPointer] = *pointer;    \
  ATRBLE1_PRINTF("| %08X",DataToSend[WritingPointer]);\
  pointer++;                                \
  WritingPointer++;                         \
}

#define WRITE_VALUE(ValueToSend) {           \
  DataToSend[WritingPointer] = ValueToSend; \
  ATRBLE1_PRINTF("| %08X",DataToSend[WritingPointer]);\
  WritingPointer++;                         \
}
#else /* DEBUG_BINARY_CONTENT */
#define READ_NFC_DATA(pointer) {            \
  if(BSP_NFCTAG_ReadData( BSP_NFCTAG_INSTANCE, (uint8_t *) (DataToSend+WritingPointer), pointer, 4 )!=NFCTAG_OK) {\
    Error_Handler(ATRBLE1_ERROR_NFC_READING,__FILE__,__LINE__); \
  }                                         \
  pointer+=4;                               \
  WritingPointer++;                         \
}

#define READ_FLASH_DATA(pointer) {          \
  DataToSend[WritingPointer] = *pointer;    \
  pointer++;                                \
  WritingPointer++;                         \
}

#define WRITE_VALUE(ValueToSend) {          \
  DataToSend[WritingPointer] = ValueToSend; \
  WritingPointer++;                         \
}
#endif /* DEBUG_BINARY_CONTENT */

  if (CurrentLogStatus == STNFC_LOG_STOP)
  {
    int32_t NumberOfBytesNecessary = 0;
    int32_t SensorNum;
    int32_t SampleNum;
    /* Fake value that could be used by BLE Application
       for understaning the starting samples point */
    uint32_t FakeLastSamplePointer = -1;
    uint32_t NFCReadingPointer = (ATRBLE1_START_ADDR_OFFSET); /* Start From NFC Content */
    uint32_t *FlashReadingPointer = (uint32_t *)LOG_FIRST_ADDRESS; /* Start from Flash Content */
    uint32_t *DataToSend;
    uint32_t WritingPointer = 0;

    ATRBLE1_PRINTF("Read Log saved\r\n");

    /* 1) Compute the Total Number of bytes necessary
     *  for sending the Log with BLE */


    /*  Protocol Header from NFC */
    /*  DataBuf8[0] = SmarTNFCHeader.ProtVersion;
        DataBuf8[1] = SmarTNFCHeader.ProtRevision;
        DataBuf8[2] = SmarTNFCHeader.BoardId;
        DataBuf8[3] = SmarTNFCHeader.FirmwareId; */

    NumberOfBytesNecessary += 4;

    /* Log Header from NFC */
    /*  DataBuf8[0] = LogDefinition.RFU;
        DataBuf8[1] = LogDefinition.VirtualSensorsNum;
        DataBuf16[1] = LogDefinition.SampleTime; */
    NumberOfBytesNecessary += 4;

    /* start epoc time from NFC */
    /*  DataBuf32 = LogDefinition.StartDateTime; */
    NumberOfBytesNecessary += 4;

    /* VS configuration */
    for (SensorNum = 0; SensorNum < LogDefinition.VirtualSensorsNum; SensorNum++)
    {
      /* We have saved the configuration for each Vs in 4-bytes */
      NumberOfBytesNecessary += 4;
    }

    /* Read the Max/Min Section from NFC */
    for (SensorNum = 0; SensorNum < LogDefinition.VirtualSensorsNum; SensorNum++)
    {

      if (ConfiguratedVirtualSensorsArray[SensorNum] == &AllVirtualSensorsArray[STTS22H_VS_ID])
      {

        /* DataBuf32 = (ShortDeltaTime ) | (((uint32_t)AllVirtualSensorsArray[STTS22H_VS_ID].MinValue.Ui16Value)<<20); */
        NumberOfBytesNecessary += 4;

        /* DataBuf32 = (ShortDeltaTime ) | (((uint32_t)AllVirtualSensorsArray[STTS22H_VS_ID].MaxValue.Ui16Value)<<20); */
        NumberOfBytesNecessary += 4;

      }
      else if (ConfiguratedVirtualSensorsArray[SensorNum] == &AllVirtualSensorsArray[LPS22DF_VS_ID])
      {

        /* DataBuf32 = (ShortDeltaTime ) | (((uint32_t)AllVirtualSensorsArray[LPS22DF_VS_ID].MinValue.Ui16Value)<<20); */
        NumberOfBytesNecessary += 4;

        /* DataBuf32 = (ShortDeltaTime) | (((uint32_t)AllVirtualSensorsArray[LPS22DF_VS_ID].MaxValue.Ui16Value)<<20); */
        NumberOfBytesNecessary += 4;

      }
      else if (ConfiguratedVirtualSensorsArray[SensorNum] == &AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID])
      {

        /* DataBuf32 = (ShortDeltaTime) ; */
        NumberOfBytesNecessary += 4;

        /* DataBuf32 = AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxValue.Ui16Value; */
        NumberOfBytesNecessary += 4;
      }

      /* 6D Orientation and MLC Tilt have not Max/Min Section */

    }

    /* Number Of Samples Saved In Flash */
    NumberOfBytesNecessary += 4;

    /* Fake Last Sample Pointer */
    NumberOfBytesNecessary += 4;

    /* Samples Log from Flash
       We have saved 2 32-bits words for any sample:
       Vs id + ShortDeltaTime
       Sample value */
    NumberOfBytesNecessary += (4 + 4) * NumberOfSamplesSavedInFlash;


    /* 2) Allocate and Fill the Buffer before to Send */
    ATRBLE1_PRINTF("\tBuffer Allocation of %dBytes\r\n", NumberOfBytesNecessary);

    DataToSend = (uint32_t *) malloc(NumberOfBytesNecessary);
    if (DataToSend == NULL)
    {
      ATRBLE1_PRINTF("ERROR allocating the Buffer\r\n");
      return;
    }

    /* Restart from the beginning */
    NFCReadingPointer = (ATRBLE1_START_ADDR_OFFSET); /* Start From NFC Content */

    ATRBLE1_PRINTF("\tNFC Content\r\n");
    /* Read the Protocol Header from NFC */
    /*  DataBuf8[0] = SmarTNFCHeader.ProtVersion;
        DataBuf8[1] = SmarTNFCHeader.ProtRevision;
        DataBuf8[2] = SmarTNFCHeader.BoardId;
        DataBuf8[3] = SmarTNFCHeader.FirmwareId; */

    READ_NFC_DATA(NFCReadingPointer)

    /* Read the Log Header from NFC */
    /*  DataBuf8[0] = LogDefinition.RFU;
        DataBuf8[1] = LogDefinition.VirtualSensorsNum;
        DataBuf16[1] = LogDefinition.SampleTime; */
    READ_NFC_DATA(NFCReadingPointer)

    /* Read the start epoc time from NFC */
    /*   DataBuf32 = LogDefinition.StartDateTime; */
    READ_NFC_DATA(NFCReadingPointer)

    /* Read the VS configuration */
    for (SensorNum = 0; SensorNum < LogDefinition.VirtualSensorsNum; SensorNum++)
    {
      /* We have saved the configuration for each Vs in 4-bytes */
      READ_NFC_DATA(NFCReadingPointer)
    }

    /* Read the Max/Min Section from NFC */
    for (SensorNum = 0; SensorNum < LogDefinition.VirtualSensorsNum; SensorNum++)
    {

      if (ConfiguratedVirtualSensorsArray[SensorNum] == &AllVirtualSensorsArray[STTS22H_VS_ID])
      {

        /* DataBuf32 = (ShortDeltaTime ) | (((uint32_t)AllVirtualSensorsArray[STTS22H_VS_ID].MinValue.Ui16Value)<<20); */
        READ_NFC_DATA(NFCReadingPointer)

        /* DataBuf32 = (ShortDeltaTime ) | (((uint32_t)AllVirtualSensorsArray[STTS22H_VS_ID].MaxValue.Ui16Value)<<20); */
        READ_NFC_DATA(NFCReadingPointer)

      }
      else if (ConfiguratedVirtualSensorsArray[SensorNum] == &AllVirtualSensorsArray[LPS22DF_VS_ID])
      {

        /* DataBuf32 = (ShortDeltaTime ) | (((uint32_t)AllVirtualSensorsArray[LPS22DF_VS_ID].MinValue.Ui16Value)<<20); */
        READ_NFC_DATA(NFCReadingPointer)

        /* DataBuf32 = (ShortDeltaTime) | (((uint32_t)AllVirtualSensorsArray[LPS22DF_VS_ID].MaxValue.Ui16Value)<<20); */
        READ_NFC_DATA(NFCReadingPointer)

      }
      else if (ConfiguratedVirtualSensorsArray[SensorNum] == &AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID])
      {

        /* DataBuf32 = (ShortDeltaTime) ; */
        READ_NFC_DATA(NFCReadingPointer)

        /* DataBuf32 = AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].MaxValue.Ui16Value; */
        READ_NFC_DATA(NFCReadingPointer)
      }

      /* 6D Orientation and MLC Tilt have not Max/Min Section */

    }

    /* Send Number Of Samples Saved In Flash */
    WRITE_VALUE(NumberOfSamplesSavedInFlash)

    /* Send a Fake Last Sample Pointer */
    WRITE_VALUE(FakeLastSamplePointer)

#ifdef DEBUG_BINARY_CONTENT
    ATRBLE1_PRINTF("\r\n\tFlash Content %d Samples\r\n", NumberOfSamplesSavedInFlash);
#else /* DEBUG_BINARY_CONTENT */
    ATRBLE1_PRINTF("\tFlash Content %d Samples\r\n", NumberOfSamplesSavedInFlash);
#endif /* DEBUG_BINARY_CONTENT */

    /* Read the samples Log from Flash */
    for (SampleNum = 0; SampleNum < NumberOfSamplesSavedInFlash; SampleNum++)
    {
      /* We have saved 2 32-bits words for any sample */

      /* Vs id + ShortDeltaTime */
      READ_FLASH_DATA(FlashReadingPointer)

      /* Sample value */
      READ_FLASH_DATA(FlashReadingPointer)
    }

    /* 3) Send the buffer with BLE */
    BinaryContentEncapsulateAndSend((uint8_t *)DataToSend, NumberOfBytesNecessary);

    ATRBLE1_PRINTF("\tDone\r\n");

    /* Free the Buffer */
    free(DataToSend);
  }
#undef READ_NFC_DATA
#undef READ_FLASH_DATA
#undef WRITE_VALUE
#undef DEBUG_BINARY_CONTENT
}

/**
  * @brief  SAI controlled by BLE for Starting a new Log
  * @param  None
  * @retval None
  */
void BleStartNewLog(void)
{
  if (CurrentLogStatus == STNFC_LOG_STOP)
  {
    uint32_t DataBuf32;
    uint8_t *DataBuf8 = (uint8_t *)&DataBuf32;
    uint16_t *DataBuf16 = (uint16_t *)&DataBuf32;

    ATRBLE1_PRINTF("Start a new Log\r\n");
    NumberOfSamplesSavedInFlash = 0;

    /* Initialize the Protocol header */
    SmarTNFCHeader.ProtVersion  = ATRBLE1_RECORD_VERSION;
    SmarTNFCHeader.ProtRevision = ATRBLE1_RECORD_REVISION;
    if (FinishGood == FINISHA)
    {
      SmarTNFCHeader.BoardId      = ATRBLE1A_NFC_BOARD_ID;
      SmarTNFCHeader.FirmwareId   = ATRBLE1A_NFC_FW_ID;
    } 
    else 
    {
      SmarTNFCHeader.BoardId      = ATRBLE1B_NFC_BOARD_ID;
      SmarTNFCHeader.FirmwareId   = ATRBLE1B_NFC_FW_ID;
    }

    DataBuf8[0] = SmarTNFCHeader.ProtVersion;
    DataBuf8[1] = SmarTNFCHeader.ProtRevision;
    DataBuf8[2] = SmarTNFCHeader.BoardId;
    DataBuf8[3] = SmarTNFCHeader.FirmwareId;

    /* Save Protocol/Board and Fw Id */
    LogDefinition.LastSamplePointer = ATRBLE1_START_ADDR_OFFSET;
    if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
    {
      Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
    }
    /* Update Last Sample Pointer (pointer to next Sample...) */
    LogDefinition.LastSamplePointer += 4;

    /* Log Header already initialized with BLE Application */

    /* Update only the table of active Virtual Sensors... */
    {
      uint8_t VirtualSensorsNum = 0;
      int32_t count;
      for (count = 0; count < ATRBLE1_VIRTUAL_SENSORS_NUM; count++)
      {

        if (AllVirtualSensorsArray[count].Enable)
        {
          ConfiguratedVirtualSensorsArray[VirtualSensorsNum] = &AllVirtualSensorsArray[count];
          VirtualSensorsNum++;
        }
      }

      /* Update the number of configurated Virtual Sensors */
      LogDefinition.VirtualSensorsNum = VirtualSensorsNum;
    }

    /* Write Virtual Sensor Number and Polling Sample Time */
    DataBuf8[0] = LogDefinition.RFU;
    DataBuf8[1] = LogDefinition.VirtualSensorsNum;
    DataBuf16[1] = LogDefinition.SampleTime;
    if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
    {
      Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
    }
    LogDefinition.LastSamplePointer += 4;

    /* Write TimeStamp==0 meaning Default Configuration */
    DataBuf32 = LogDefinition.StartDateTime;
    if (BSP_NFCTAG_WriteData(BSP_NFCTAG_INSTANCE, (uint8_t *)&DataBuf32, LogDefinition.LastSamplePointer, 4) != NFCTAG_OK)
    {
      Error_Handler(ATRBLE1_ERROR_NFC_WRITING, __FILE__, __LINE__);
    }
    LogDefinition.LastSamplePointer += 4;

    /* Write Virtual Sensors Configuration */
    SaveVirtualSensorsConfiguration();

    /* Reset the Max/Min For each Sensor */
    ResetMaxMinValuesAllVirtualSensors();

    /* Save the Max/Min for each Sensor */
    SaveMaxMinValuesForVirtualSensors();

    /* Write Sample Counter and Last Sample Pointer*/
    LogDefinition.SampleCounterAddress = LogDefinition.LastSamplePointer;
    LogDefinition.SampleCounter = 0;
    UpdateLastSamplePointerAndSampleCounter(&LogDefinition);
    LogDefinition.LastSamplePointer += 8; /* We use 4bytes for each one */

    /* Set the Date&Time */
    if (STNFC_SetDateTime(LogDefinition.StartDateTime, &hrtc, &LogDefinition) == 0)
    {
      ATRBLE1_PRINTF("Error: Setting RTC\r\n");
    }
    else
    {
      ATRBLE1_PRINTF("Set RTC Date&Time\r\n");
    }

    NfcType5_SetInitialNDEFPayLoadLengthValue(LogDefinition.LastSamplePointer);
    NfcType5_ComputeNDEFPayLoadSize(LogDefinition.LastSamplePointer, LogDefinition.SampleCounter);

    /* Reset Log Area */
    DeleteLogArea();
    NumberOfSamplesSavedInFlash = 0;

    /* lsm6dsv16x WakeUp/6D Rec or MLC: Init/DeInit */
    InitDeInitAccEventThreshold();

    CurrentLogStatus = STNFC_LOG_RUNNING;
    LogStartPeriodicallyTimer();
  }
}


/**
  * @brief  Reads the active log configuration
  * @param  SNFC_LogDefinition_t *LogDefinition
  * @param  uint32_t CurrentStartDateTime  Actual Start Date&Time Value
  * @param  int32_t OnlyChecks if ==1 it controls only if the configuration is valid
  * @retval None
  */
static void ReadConfiguration(SNFC_LogDefinition_t *LogDefinition, uint32_t CurrentStartDateTime, int32_t OnlyChecks)
{
  uint32_t DataBuf32;
  uint8_t *DataBuf8 = (uint8_t *)&DataBuf32;
  uint16_t *DataBuf16 = (uint16_t *)&DataBuf32;
  SNFC_ValidConfiguration_t ValidConf = STNFC_VALID_CONFIG;
  SNFC_CodeDefinition_t *LocalSmarTNFCodeHeader = (SNFC_CodeDefinition_t *)&DataBuf32;

  LogDefinition->LastSamplePointer = ATRBLE1_START_ADDR_OFFSET;
  if (BSP_NFCTAG_ReadData(BSP_NFCTAG_INSTANCE, DataBuf8, LogDefinition->LastSamplePointer, 4) != NFCTAG_OK)
  {
    Error_Handler(ATRBLE1_ERROR_NFC_READING, __FILE__, __LINE__);
    ValidConf = STNFC_ERROR_READING_CONFIG;
  }

  if (ValidConf == STNFC_VALID_CONFIG)
  {
    uint8_t BoardId,FirmwareId;
    
    LogDefinition->LastSamplePointer += 4;
      
    if (FinishGood == FINISHA)
    {
      BoardId      = ATRBLE1A_NFC_BOARD_ID;
      FirmwareId   = ATRBLE1A_NFC_FW_ID;
    }
    else 
    {
      BoardId      = ATRBLE1B_NFC_BOARD_ID;
      FirmwareId   = ATRBLE1B_NFC_FW_ID;
    }
  
    /* Check the protocol header */
    if ((LocalSmarTNFCodeHeader->ProtVersion  != ATRBLE1_RECORD_VERSION) |
        (LocalSmarTNFCodeHeader->ProtRevision != ATRBLE1_RECORD_REVISION) |
        (LocalSmarTNFCodeHeader->BoardId      != BoardId) |
        (LocalSmarTNFCodeHeader->FirmwareId   != FirmwareId))
    {
      ATRBLE1_PRINTF("Error: Protocol Header not valid\r\n");
      ValidConf = STNFC_NOT_VALID_CONFIG;
    }
    else if (OnlyChecks == 0)
    {
      /* If we are not making only the check... update the configuration */
      memcpy(&SmarTNFCHeader, LocalSmarTNFCodeHeader, sizeof(SNFC_CodeDefinition_t));
    }
  }

  if (ValidConf == STNFC_VALID_CONFIG)
  {
    if (BSP_NFCTAG_ReadData(BSP_NFCTAG_INSTANCE, DataBuf8, LogDefinition->LastSamplePointer, 4) != NFCTAG_OK)
    {
      Error_Handler(ATRBLE1_ERROR_NFC_READING, __FILE__, __LINE__);
      ValidConf = STNFC_ERROR_READING_CONFIG;
    }
    if (ValidConf == STNFC_VALID_CONFIG)
    {
      LogDefinition->LastSamplePointer += 4;

      LogDefinition->RFU = DataBuf8[0];
      LogDefinition->VirtualSensorsNum = DataBuf8[1];
      LogDefinition->SampleTime = DataBuf16[1];

      ATRBLE1_PRINTF("\tVn=%d SampleTime=%d\r\n", LogDefinition->VirtualSensorsNum, LogDefinition->SampleTime);

      if (LogDefinition->VirtualSensorsNum == 0)
      {
        ATRBLE1_PRINTF("\tError: VirtualSensorsNum==0\r\n");
        ValidConf = STNFC_NOT_VALID_CONFIG;
      }

      if (LogDefinition->VirtualSensorsNum > ATRBLE1_VIRTUAL_SENSORS_NUM)
      {
        ATRBLE1_PRINTF("\tError: VirtualSensorsNum >%d\r\n", ATRBLE1_VIRTUAL_SENSORS_NUM);
        ValidConf = STNFC_NOT_VALID_CONFIG;
      }

      if ((LogDefinition->SampleTime < 1) | (LogDefinition->SampleTime > (UINT16_MAX - 1)))
      {
        ATRBLE1_PRINTF("\tError: SampleTime =%d Not Valid\r\n", LogDefinition->SampleTime);
        ValidConf = STNFC_NOT_VALID_CONFIG;
      }
    }
  }

  if (ValidConf == STNFC_VALID_CONFIG)
  {
    if (BSP_NFCTAG_ReadData(BSP_NFCTAG_INSTANCE, DataBuf8, LogDefinition->LastSamplePointer, 4) != NFCTAG_OK)
    {
      Error_Handler(ATRBLE1_ERROR_NFC_READING, __FILE__, __LINE__);
      ValidConf = STNFC_ERROR_READING_CONFIG;
    }
    if (ValidConf == STNFC_VALID_CONFIG)
    {
      LogDefinition->LastSamplePointer += 4;

      LogDefinition->StartDateTime = DataBuf32;
      /* If we have a valid not null Start Date&Time and different respect the
         Current one used...-> New Configuration */
      if (LogDefinition->StartDateTime != 0)
      {
        if (LogDefinition->StartDateTime == CurrentStartDateTime)
        {
          ATRBLE1_PRINTF("Start Time not Changed\r\n");
          ValidConf = STNFC_NOT_CHANGED_CONFIG;
        }
      }
      else
      {
        ATRBLE1_PRINTF("Start Time ==0\r\n");
        ValidConf = STNFC_NOT_VALID_CONFIG;
      }
    }
  }

  if (ValidConf == STNFC_VALID_CONFIG)
  {
    /* We need to read the Virtual Sensor Configuration */
    int32_t SensorNum;
    for (SensorNum = 0; ((SensorNum < LogDefinition->VirtualSensorsNum) & (ValidConf == STNFC_VALID_CONFIG)); SensorNum++)
    {
      if (BSP_NFCTAG_ReadData(BSP_NFCTAG_INSTANCE, DataBuf8, LogDefinition->LastSamplePointer, 4) != NFCTAG_OK)
      {
        Error_Handler(ATRBLE1_ERROR_NFC_READING, __FILE__, __LINE__);
        ValidConf = STNFC_ERROR_READING_CONFIG;
      }
      if (ValidConf == STNFC_VALID_CONFIG)
      {
        LogDefinition->LastSamplePointer += 4;
        switch (DataBuf32 & 0x7)
        {
          case STTS22H_VS_ID:
            ATRBLE1_PRINTF("\tFound STTS22H_VS_ID:\r\n");
            if (OnlyChecks == 0)
            {
              ConfiguratedVirtualSensorsArray[SensorNum] = &AllVirtualSensorsArray[STTS22H_VS_ID];
              ConfiguratedVirtualSensorsArray[SensorNum]->Enable = 1;
              ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType = (SNFC_ThresholdsUsage_t)((DataBuf32 >> 3) & 0x3);
              ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui16Value = (DataBuf32 >> (3 + 2)) & 0x1FF;
              ATRBLE1_PRINTF("\tThsUsageType=%s\r\n",
                             ThresholdsUsageName[ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType]);
              ATRBLE1_PRINTF("\tTh1.Ui16Value=%f\r\n",
                             STTS22H_CODED_TO_SAMPLE(ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui16Value));
              if (ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType < TH_LESS)
              {
                ConfiguratedVirtualSensorsArray[SensorNum]->Th2.Ui16Value = (DataBuf32 >> (3 + 2 + 9)) & 0x1FF;
                ATRBLE1_PRINTF("\tTh2.Ui16Value=%f\r\n",
                               STTS22H_CODED_TO_SAMPLE(ConfiguratedVirtualSensorsArray[SensorNum]->Th2.Ui16Value));
              }
            }
            break;

          case LPS22DF_VS_ID:
            ATRBLE1_PRINTF("\tFound LPS22DF_VS_ID:\r\n");
            if (OnlyChecks == 0)
            {
              ConfiguratedVirtualSensorsArray[SensorNum] = &AllVirtualSensorsArray[LPS22DF_VS_ID];
              ConfiguratedVirtualSensorsArray[SensorNum]->Enable = 1;
              ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType = (SNFC_ThresholdsUsage_t)((DataBuf32 >> 3) & 0x3);
              ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui16Value = (DataBuf32 >> (3 + 2)) & 0x7FF;
              ATRBLE1_PRINTF("\tThsUsageType=%s\r\n",
                             ThresholdsUsageName[ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType]);
              ATRBLE1_PRINTF("\tTh1.Ui16Value=%f\r\n",
                             LPS22DF_CODED_TO_SAMPLE(ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui16Value));
              if (ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType < TH_LESS)
              {
                ConfiguratedVirtualSensorsArray[SensorNum]->Th2.Ui16Value = (DataBuf32 >> (3 + 2 + 11)) & 0x7FF;
                ATRBLE1_PRINTF("\tTh2.Ui16Value=%f\r\n",
                               LPS22DF_CODED_TO_SAMPLE(ConfiguratedVirtualSensorsArray[SensorNum]->Th2.Ui16Value));
              }
            }
            break;

          case LSM6DSV16X_WU_VS_ID:
            ATRBLE1_PRINTF("\tFound LSM6DSV16X_WU_VS_ID:\r\n");
            if (OnlyChecks == 0)
            {
              ConfiguratedVirtualSensorsArray[SensorNum] = &AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID];
              ConfiguratedVirtualSensorsArray[SensorNum]->Enable = 1;
              ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType = (SNFC_ThresholdsUsage_t)((DataBuf32 >> 3) & 0x3);
              ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui16Value = (DataBuf32 >> (3 + 2)) & 0x7FFFF;
              ATRBLE1_PRINTF("\tThsUsageType=%s\r\n",
                             ThresholdsUsageName[ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType]);
              ATRBLE1_PRINTF("\tTh1.Ui16Value=%d\r\n", ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui16Value);

              /* Check Virtual Sensors Incompatibility */
              if (AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable)
              {
                ATRBLE1_PRINTF("\tWarning Incompatibility with MLC\r\n\tDisable MLC\r\n");
                AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable = 0;
              }
            }
            break;

          case LSM6DSV16X_6D_VS_ID:
            ATRBLE1_PRINTF("\tFound LSM6DSV16X_6D_VS_ID:\r\n");
            if (OnlyChecks == 0)
            {
              ConfiguratedVirtualSensorsArray[SensorNum] = &AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID];
              ConfiguratedVirtualSensorsArray[SensorNum]->Enable = 1;
              ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType = (SNFC_ThresholdsUsage_t)((DataBuf32 >> 3) & 0x3);
              ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui8Value = (DataBuf32 >> (3 + 2)) & 0xFF;
              ATRBLE1_PRINTF("\tThsUsageType=%s\r\n",
                             ThresholdsUsageName[ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType]);
              ATRBLE1_PRINTF("\tTh1.Ui8Value=%d\r\n", ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui8Value);
              if (ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType < TH_LESS)
              {
                ConfiguratedVirtualSensorsArray[SensorNum]->Th2.Ui8Value = (DataBuf32 >> (3 + 2 + 8)) & 0xFF;
                ATRBLE1_PRINTF("\tTh2.Ui8Value=%d\r\n", ConfiguratedVirtualSensorsArray[SensorNum]->Th2.Ui8Value);
              }

              /* Check Virtual Sensors Incompatibility */
              if (AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable)
              {
                ATRBLE1_PRINTF("\tWarning Incompatibility with MLC\r\n\tDisable MLC\r\n");
                AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable = 0;
              }
            }
            break;

          case LSM6DSV16X_MLC_VS_ID:
            ATRBLE1_PRINTF("\tFound LSM6DSV16X_MLC_VS_ID:\r\n");
            if (OnlyChecks == 0)
            {
              ConfiguratedVirtualSensorsArray[SensorNum] = &AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID];
              ConfiguratedVirtualSensorsArray[SensorNum]->Enable = 1;
              ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType = (SNFC_ThresholdsUsage_t)((DataBuf32 >> 3) & 0x3);
              ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui8Value = (DataBuf32 >> (3 + 2)) & 0xFF;
              ATRBLE1_PRINTF("\tThsUsageType=%s\r\n",
                             ThresholdsUsageName[ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType]);
              ATRBLE1_PRINTF("\tTh1.Ui8Value=%d\r\n", ConfiguratedVirtualSensorsArray[SensorNum]->Th1.Ui8Value);
              if (ConfiguratedVirtualSensorsArray[SensorNum]->ThsUsageType < TH_LESS)
              {
                ConfiguratedVirtualSensorsArray[SensorNum]->Th2.Ui8Value = (DataBuf32 >> (3 + 2 + 8)) & 0xFF;
                ATRBLE1_PRINTF("\tTh2.Ui8Value=%d\r\n", ConfiguratedVirtualSensorsArray[SensorNum]->Th2.Ui8Value);
              }

              /* Check Virtual Sensors Incompatibility */
              if (AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable)
              {
                ATRBLE1_PRINTF("\tWarning: Incompatibility with WakeUp\r\n\tDisable WakeUp\r\n");
                AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable = 0;
              }
              if (AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Enable)
              {
                ATRBLE1_PRINTF("\tWarning Incompatibility with 6D Orientation\r\n\tDisable 6D Orientation\r\n");
                AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Enable = 0;
              }
            }
            break;

          default:
            ValidConf = STNFC_NOT_VALID_CONFIG;
            ATRBLE1_PRINTF("Error: Not recognized VirtualSensorID=%d\r\n", DataBuf32 & 0x7);
        }
      }
    }
  }

  switch (ValidConf)
  {
    case STNFC_NOT_VALID_CONFIG:
      ATRBLE1_PRINTF("Not Valid Configuration present on NFC Skip it\r\n");
      LogDefinition->StartDateTime = 0;
      break;
    case STNFC_ERROR_READING_CONFIG:
      ATRBLE1_PRINTF("Error Reading Configuration present on NFC Skip it\r\n");
      ATRBLE1_PRINTF("Try again to write the new configuration\r\n");
      LogDefinition->StartDateTime = 0;
      break;
    case STNFC_VALID_CONFIG:
      if (OnlyChecks)
      {
        ATRBLE1_PRINTF("Valid Configuration present on NFC\r\n");
      }
      break;
    case STNFC_NOT_CHANGED_CONFIG:
      if (OnlyChecks)
      {
        ATRBLE1_PRINTF("Going on with the current Configuration\r\n");
      }
      LogDefinition->StartDateTime = 0;
      break;
  }
}
