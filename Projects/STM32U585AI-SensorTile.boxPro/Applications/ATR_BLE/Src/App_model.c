/**
  ******************************************************************************
  * @file    App_Model.c
  * @author  SRA
  * @brief   App Application Model and PnPL Components APIs
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  *
  ******************************************************************************
  */

/**
  ******************************************************************************
  * This file has been auto generated from the following Device Template Model:
  * dtmi:appconfig:steval_mkboxpro:fp_atr_ble1;2
  *
  * Created by: DTDL2PnPL_cGen version 1.1.0
  *
  * WARNING! All changes made to this file will be lost if this is regenerated
  ******************************************************************************
  */

#include "App_model.h"
#include <string.h>
#include <stdio.h>

/* USER includes -------------------------------------------------------------*/
#include "SmartNFC.h"
#include "LogControl.h"
#include "main.h"

/* USER private function prototypes ------------------------------------------*/
static void UpdateStatus(void);

/* USER defines --------------------------------------------------------------*/

static uint8_t sensors_cnt = 0;

AppModel_t app_model;

AppModel_t *getAppModel(void)
{
  return &app_model;
}

/* Device Components APIs ----------------------------------------------------*/

/* STTS22H PnPL Component ----------------------------------------------------*/
static SensorModel_t stts22h_temp_model;

uint8_t stts22h_temp_comp_init(void)
{
  stts22h_temp_model.comp_name = stts22h_temp_get_key();

  uint16_t id = sensors_cnt;
  sensors_cnt += 1;
  app_model.s_models[id] = &stts22h_temp_model;

  /* USER Component initialization code */
  return 0;
}
char *stts22h_temp_get_key(void)
{
  return "stts22h_temp";
}

uint8_t stts22h_temp_get_enable(bool *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[STTS22H_VS_ID].Enable;
  return 0;
}
uint8_t stts22h_temp_get_thresholds_usage(float *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[STTS22H_VS_ID].ThsUsageType;
  return 0;
}
uint8_t stts22h_temp_get_threshold_low(float *value)
{
  /* USER Code */
  *value = STTS22H_CODED_TO_SAMPLE(AllVirtualSensorsArray[STTS22H_VS_ID].Th1.Ui16Value);
  return 0;
}
uint8_t stts22h_temp_get_threshold_high(float *value)
{
  /* USER Code */
  *value = STTS22H_CODED_TO_SAMPLE(AllVirtualSensorsArray[STTS22H_VS_ID].Th2.Ui16Value);
  return 0;
}
uint8_t stts22h_temp_get_stream_id(int8_t *value)
{
  /* USER Code */
  return 0;
}
uint8_t stts22h_temp_get_ep_id(int8_t *value)
{
  /* USER Code */
  return 0;
}
uint8_t stts22h_temp_set_enable(bool value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[STTS22H_VS_ID].Enable = value;

  UpdateStatus();
  return 0;
}
uint8_t stts22h_temp_set_thresholds_usage(float value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[STTS22H_VS_ID].ThsUsageType = (SNFC_ThresholdsUsage_t) value;

  UpdateStatus();
  return 0;
}
uint8_t stts22h_temp_set_threshold_low(float value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[STTS22H_VS_ID].Th1.Ui16Value =  STTS22H_SAMPLE_TO_CODED(value);

  UpdateStatus();
  return 0;
}
uint8_t stts22h_temp_set_threshold_high(float value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[STTS22H_VS_ID].Th2.Ui16Value =  STTS22H_SAMPLE_TO_CODED(value);

  UpdateStatus();
  return 0;
}

/* LPS22DF PnPL Component ----------------------------------------------------*/
static SensorModel_t lps22df_press_model;

uint8_t lps22df_press_comp_init(void)
{
  lps22df_press_model.comp_name = lps22df_press_get_key();

  uint16_t id = sensors_cnt;
  sensors_cnt += 1;
  app_model.s_models[id] = &lps22df_press_model;

  /* USER Component initialization code */
  return 0;
}
char *lps22df_press_get_key(void)
{
  return "lps22df_press";
}

uint8_t lps22df_press_get_enable(bool *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[LPS22DF_VS_ID].Enable;
  return 0;
}
uint8_t lps22df_press_get_thresholds_usage(float *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[LPS22DF_VS_ID].ThsUsageType;
  return 0;
}
uint8_t lps22df_press_get_threshold_low(float *value)
{
  /* USER Code */
  *value = LPS22DF_CODED_TO_SAMPLE(AllVirtualSensorsArray[LPS22DF_VS_ID].Th1.Ui16Value);
  return 0;
}
uint8_t lps22df_press_get_threshold_high(float *value)
{
  /* USER Code */
  *value = LPS22DF_CODED_TO_SAMPLE(AllVirtualSensorsArray[LPS22DF_VS_ID].Th2.Ui16Value);
  return 0;
}
uint8_t lps22df_press_get_stream_id(int8_t *value)
{
  /* USER Code */
  return 0;
}
uint8_t lps22df_press_get_ep_id(int8_t *value)
{
  /* USER Code */
  return 0;
}
uint8_t lps22df_press_set_enable(bool value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[LPS22DF_VS_ID].Enable = value;

  UpdateStatus();
  return 0;
}
uint8_t lps22df_press_set_thresholds_usage(float value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[LPS22DF_VS_ID].ThsUsageType = (SNFC_ThresholdsUsage_t) value;

  UpdateStatus();
  return 0;
}
uint8_t lps22df_press_set_threshold_low(float value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[LPS22DF_VS_ID].Th1.Ui16Value =  LPS22DF_SAMPLE_TO_CODED(value);

  UpdateStatus();
  return 0;
}
uint8_t lps22df_press_set_threshold_high(float value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[LPS22DF_VS_ID].Th2.Ui16Value =  LPS22DF_SAMPLE_TO_CODED(value);

  UpdateStatus();
  return 0;
}

/* LSM6DSV16X MLC PnPL Component ---------------------------------------------*/
static SensorModel_t lsm6dsv16x_mlc_model;

uint8_t lsm6dsv16x_mlc_comp_init(void)
{
  lsm6dsv16x_mlc_model.comp_name = lsm6dsv16x_mlc_get_key();

  uint16_t id = sensors_cnt;
  sensors_cnt += 1;
  app_model.s_models[id] = &lsm6dsv16x_mlc_model;

  /* USER Component initialization code */
  return 0;
}
char *lsm6dsv16x_mlc_get_key(void)
{
  return "lsm6dsv16x_mlc";
}

uint8_t lsm6dsv16x_mlc_get_enable(bool *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable;
  return 0;
}
uint8_t lsm6dsv16x_mlc_get_thresholds_usage(float *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].ThsUsageType;
  return 0;
}
uint8_t lsm6dsv16x_mlc_get_threshold_low(float *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Th1.Ui8Value;
  return 0;
}
uint8_t lsm6dsv16x_mlc_get_threshold_high(float *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Th2.Ui8Value;
  return 0;
}
uint8_t lsm6dsv16x_mlc_get_stream_id(int8_t *value)
{
  /* USER Code */
  return 0;
}
uint8_t lsm6dsv16x_mlc_get_ep_id(int8_t *value)
{
  /* USER Code */
  return 0;
}
uint8_t lsm6dsv16x_mlc_set_enable(bool value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable = value;


  /* Check Virtual Sensors Incompatibility */
  if (AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable)
  {
    if (AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Enable)
    {
      ATRBLE1_PRINTF("\tWarning Incompatibility with 6D\r\n\tDisable 6D\r\n");
      AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Enable = 0;
    }

    if (AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable)
    {
      ATRBLE1_PRINTF("\tWarning Incompatibility with WU\r\n\tDisable WU\r\n");
      AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable = 0;
    }
  }

  UpdateStatus();
  return 0;
}
uint8_t lsm6dsv16x_mlc_set_thresholds_usage(float value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].ThsUsageType = (SNFC_ThresholdsUsage_t) value;

  UpdateStatus();
  return 0;
}
uint8_t lsm6dsv16x_mlc_set_threshold_low(float value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Th1.Ui8Value = (uint8_t) value;

  UpdateStatus();
  return 0;
}
uint8_t lsm6dsv16x_mlc_set_threshold_high(float value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Th2.Ui8Value = (uint8_t) value;

  UpdateStatus();
  return 0;
}

/* LSM6DSV16X 6D PnPL Component ----------------------------------------------*/
static SensorModel_t lsm6dsv16x6d_acc_model;

uint8_t lsm6dsv16x6d_acc_comp_init(void)
{
  lsm6dsv16x6d_acc_model.comp_name = lsm6dsv16x6d_acc_get_key();

  uint16_t id = sensors_cnt;
  sensors_cnt += 1;
  app_model.s_models[id] = &lsm6dsv16x6d_acc_model;

  /* USER Component initialization code */
  return 0;
}
char *lsm6dsv16x6d_acc_get_key(void)
{
  return "lsm6dsv16x6d_acc";
}

uint8_t lsm6dsv16x6d_acc_get_enable(bool *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Enable;
  return 0;
}
uint8_t lsm6dsv16x6d_acc_get_thresholds_usage(float *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].ThsUsageType;
  return 0;
}
uint8_t lsm6dsv16x6d_acc_get_threshold_low(float *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Th1.Ui8Value;
  return 0;
}
uint8_t lsm6dsv16x6d_acc_get_threshold_high(float *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Th2.Ui8Value;
  return 0;
}
uint8_t lsm6dsv16x6d_acc_get_stream_id(int8_t *value)
{
  /* USER Code */
  return 0;
}
uint8_t lsm6dsv16x6d_acc_get_ep_id(int8_t *value)
{
  /* USER Code */
  return 0;
}
uint8_t lsm6dsv16x6d_acc_set_enable(bool value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Enable = value;

  /* Check Virtual Sensors Incompatibility */
  if (AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Enable)
  {
    if (AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable)
    {
      ATRBLE1_PRINTF("\tWarning Incompatibility with MLC\r\n\tDisable MLC\r\n");
      AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable = 0;
    }
  }

  UpdateStatus();
  return 0;
}
uint8_t lsm6dsv16x6d_acc_set_thresholds_usage(float value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].ThsUsageType = (SNFC_ThresholdsUsage_t) value;

  UpdateStatus();
  return 0;
}
uint8_t lsm6dsv16x6d_acc_set_threshold_low(float value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Th1.Ui8Value = (uint8_t) value;
  return 0;
}
uint8_t lsm6dsv16x6d_acc_set_threshold_high(float value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[LSM6DSV16X_6D_VS_ID].Th2.Ui8Value = (uint8_t) value;
  return 0;
}

/* LSM6DSV16X WakeUp PnPL Component ------------------------------------------*/
static SensorModel_t lsm6dsv16xWu_acc_model;

uint8_t lsm6dsv16xWu_acc_comp_init(void)
{
  lsm6dsv16xWu_acc_model.comp_name = lsm6dsv16xWu_acc_get_key();

  uint16_t id = sensors_cnt;
  sensors_cnt += 1;
  app_model.s_models[id] = &lsm6dsv16xWu_acc_model;

  /* USER Component initialization code */
  return 0;
}
char *lsm6dsv16xWu_acc_get_key(void)
{
  return "lsm6dsv16xWu_acc";
}

uint8_t lsm6dsv16xWu_acc_get_enable(bool *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable;
  return 0;
}
uint8_t lsm6dsv16xWu_acc_get_thresholds_usage(float *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].ThsUsageType;
  return 0;
}
uint8_t lsm6dsv16xWu_acc_get_threshold_low(float *value)
{
  /* USER Code */
  *value = AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Th1.Ui16Value;
  return 0;
}
uint8_t lsm6dsv16xWu_acc_get_stream_id(int8_t *value)
{
  /* USER Code */
  return 0;
}
uint8_t lsm6dsv16xWu_acc_get_ep_id(int8_t *value)
{
  /* USER Code */
  return 0;
}
uint8_t lsm6dsv16xWu_acc_set_enable(bool value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable = value;

  /* Check Virtual Sensors Incompatibility */
  if (AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Enable)
  {
    if (AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable)
    {
      ATRBLE1_PRINTF("\tWarning Incompatibility with MLC\r\n\tDisable MLC\r\n");
      AllVirtualSensorsArray[LSM6DSV16X_MLC_VS_ID].Enable = 0;
    }
  }

  UpdateStatus();
  return 0;
}
uint8_t lsm6dsv16xWu_acc_set_threshold_low(float value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  AllVirtualSensorsArray[LSM6DSV16X_WU_VS_ID].Th1.Ui16Value = (uint16_t) value;
  return 0;
}

/* Control PnPL Component ----------------------------------------------------*/
uint8_t control_comp_init(void)
{
  app_model.control_model.comp_name = control_get_key();

  /* USER Component initialization code */
  return 0;
}
char *control_get_key(void)
{
  return "control";
}

uint8_t control_get_log_status(char **value)
{
  /* USER Code */
  static char Status[16];

  switch (CurrentLogStatus)
  {
    case STNFC_LOG_STOP:
      sprintf(Status, "Stopped");
      break;
    case STNFC_LOG_RUNNING:
      sprintf(Status, "Logging");
      break;
    case STNFC_LOG_PAUSED:
      sprintf(Status, "Paused");
      break;
  }
  *value = Status;
  return 0;
}
uint8_t control_get_SampleInterval(int32_t *value)
{
  /* USER Code */
  *value = LogDefinition.SampleTime;
  return 0;
}
uint8_t control_set_SampleInterval(int32_t value)
{
  /* USER Code */
  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  LogDefinition.SampleTime = value;

  UpdateStatus();
  return 0;
}
uint8_t control_start_log(IControl_t *ifn, int32_t epoc_time)
{
  /* IControl_start_log(ifn, epoc_time); */
  ATRBLE1_PRINTF("Received Start Log Command [epoc_time=%d]\r\n", epoc_time);

  /* with int32_t max epoc time possible is
    Tuesday 19 January 2038 03:14:07 GMT+01:00
    So it should be enough... */

  switch (CurrentLogStatus)
  {
    case STNFC_LOG_STOP:
      LogDefinition.StartDateTime = epoc_time;
      BleStartNewLog();
      break;
    case STNFC_LOG_RUNNING:
      ATRBLE1_PRINTF("Log was already ongoing.. Skip Command\r\n");
      break;
    case STNFC_LOG_PAUSED:
      /* Stop before the Log on Paused state */
      BleStopCurrentLog();

      LogDefinition.StartDateTime = epoc_time;
      BleStartNewLog();
      break;
  }

  UpdateStatus();
  return 0;
}
uint8_t control_stop_log(IControl_t *ifn)
{
  /* IControl_stop_log(ifn); */
  ATRBLE1_PRINTF("Received Stop Log Command\r\n");
  switch (CurrentLogStatus)
  {
    case STNFC_LOG_STOP:
      ATRBLE1_PRINTF("Log was not running.. Skip Command\r\n");
      break;
    case STNFC_LOG_RUNNING:
    case STNFC_LOG_PAUSED:
      BleStopCurrentLog();
      break;
  }

  UpdateStatus();
  return 0;
}
uint8_t control_read_log(IControl_t *ifn)
{
// IControl_read_log(ifn);
  ATRBLE1_PRINTF("Received Read Log Command\r\n");

  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  BleReadCurrentLog();

  UpdateStatus();
  return 0;
}
uint8_t control_delete_log(IControl_t *ifn)
{
// IControl_delete_log(ifn);
  ATRBLE1_PRINTF("Received Delete Log Command\r\n");

  /* Check if we need to stop the current Log */
  if (CurrentLogStatus != STNFC_LOG_STOP)
  {
    BleStopCurrentLog();
  }

  BleDeleteCurrentLog();
  return 0;
}
uint8_t control_pause_resume_log(IControl_t *ifn)
{
// IControl_pause_resume_log(ifn);
  ATRBLE1_PRINTF("Received Pause/Resume Log Command\r\n");

  switch (CurrentLogStatus)
  {
    case STNFC_LOG_STOP:
      ATRBLE1_PRINTF("Log was not running.. Skip Command\r\n");
      break;
    case STNFC_LOG_RUNNING:
      BlePauseCurrentLog();
      break;
    case STNFC_LOG_PAUSED:
      BleResumeCurrentLog();
      break;
  }

  UpdateStatus();
  return 0;
}

/* Device Information PnPL Component -----------------------------------------*/
uint8_t DeviceInformation_comp_init(void)
{

  /* USER Component initialization code */
  return 0;
}
char *DeviceInformation_get_key(void)
{
  return "DeviceInformation";
}

uint8_t DeviceInformation_get_manufacturer(char **value)
{
  /* USER Code */
  static char StringValue[24];
  sprintf(StringValue, "STMicroelectronics");
  *value = StringValue;
  return 0;
}
uint8_t DeviceInformation_get_model(char **value)
{
  /* USER Code */
  static char StringValue[24];
  if (FinishGood == FINISHA)
  {
    sprintf(StringValue, "steval_stbox_pro (A)");
  }
  else
  {
    sprintf(StringValue, "steval_stbox_pro (B)");
  }
  *value = StringValue;
  return 0;
}
uint8_t DeviceInformation_get_swVersion(char **value)
{
  /* USER Code */
  static char FwName[32];
  sprintf(FwName, "%s_%s_%c.%c.%c",
          "U585",
          ATRBLE1_PACKAGENAME,
          ATRBLE1_VERSION_MAJOR,
          ATRBLE1_VERSION_MINOR,
          ATRBLE1_VERSION_PATCH);
  *value = FwName;
  return 0;
}
uint8_t DeviceInformation_get_osName(char **value)
{
  /* USER Code */
  static char StringValue[8];
  sprintf(StringValue, "None");
  *value = StringValue;
  return 0;
}
uint8_t DeviceInformation_get_processorArchitecture(char **value)
{
  /* USER Code */
  static char StringValue[4];
  sprintf(StringValue, "ARM");
  *value = StringValue;
  return 0;
}
uint8_t DeviceInformation_get_processorManufacturer(char **value)
{
  /* USER Code */
  static char StringValue[4];
  sprintf(StringValue, "ARM");
  *value = StringValue;
  return 0;
}
uint8_t DeviceInformation_get_totalStorage(float *value)
{
  /* USER Code */
  *value = 2097151;
  return 0;
}
uint8_t DeviceInformation_get_totalMemory(float *value)
{
  /* USER Code */
  *value = 786431;
  return 0;
}

static void UpdateStatus(void)
{
  PnPLCommand_t PnPLCommand;
  char *SerializedJSON;
  uint32_t size;

  sprintf(PnPLCommand.comp_name, "%s", "all");
  PnPLCommand.comm_type = PNPL_CMD_GET;

  PnPLSerializeResponse(&PnPLCommand, &SerializedJSON, &size, 0);

  ATRBLE1_PRINTF("--> <%.*s>\r\n", size, SerializedJSON);

  PnPLikeEncapsulateAndSend((uint8_t *) SerializedJSON, size);
  free(SerializedJSON);
}
