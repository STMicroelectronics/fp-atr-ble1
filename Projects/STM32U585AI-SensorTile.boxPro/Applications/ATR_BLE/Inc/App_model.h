/**
  ******************************************************************************
  * @file    App_model.h
  * @author  SRA
  * @brief   App Application Model and PnPL Components APIs
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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

/**
  ******************************************************************************
  * Component APIs *************************************************************
  * - Component init function
  *    <comp_name>_comp_init(void)
  * - Component get_key function
  *    <comp_name>_get_key(void)
  * - Component GET/SET Properties APIs ****************************************
  *  - GET Functions
  *    uint8_t <comp_name>_get_<prop_name>(prop_type *value)
  *      if prop_type == char --> (char **value)
  *  - SET Functions
  *    uint8_t <comp_name>_set_<prop_name>(prop_type value)
  *      if prop_type == char --> (char *value)
  *  - Component COMMAND Reaction Functions
  *      uint8_t <comp_name>_<command_name>(I<Compname>_t * ifn,
  *                     field1_type field1_name, field2_type field2_name, ...,
  *                     fieldN_type fieldN_name); //ifn: Interface Functions
  *  - Component TELEMETRY Send Functions
  *      uint8_t <comp_name>_create_telemetry(tel1_type tel1_name,
  *                     tel2_type tel2_name, ..., telN_type telN_name,
  *                     char **telemetry, uint32_t *size)
  ******************************************************************************
  */

#ifndef APP_MODEL_H_
#define APP_MODEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"
#include "PnPLCompManager.h"
#include "IControl.h"
#include "IControl_vtbl.h"
/* USER includes -------------------------------------------------------------*/

#define COMP_TYPE_SENSOR          0x00
#define COMP_TYPE_ALGORITHM       0x01
#define COMP_TYPE_OTHER           0x02
#define COMP_TYPE_ACTUATOR        0x03

#define LOG_CTRL_MODE_SD          0x00
#define LOG_CTRL_MODE_USB         0x01
#define LOG_CTRL_MODE_BLE         0x02

#define SENSOR_NUMBER             5
#define ALGORITHM_NUMBER          0
#define ACTUATOR_NUMBER           0
#define OTHER_COMP_NUMBER         1

typedef struct _StreamParams_t
{
  int8_t stream_id;
  int8_t usb_ep;
  uint16_t spts;
  uint32_t usb_dps;
  uint32_t sd_dps;
  float ioffset;
  float bandwidth;
  /* Stream Parameters Model USER code */
} StreamParams_t;

typedef struct _SensorModel_t
{
  /* E.g. IIS3DWB Component is a sensor (look @ schema field)
     so, its model has the following structure */
  uint8_t id;
  char *comp_name;
  StreamParams_t streamParams;
  /* Sensor Components Model USER code */
} SensorModel_t;

typedef struct _ControlModel_t
{
  char *comp_name;
  /* Control Component Model USER code */
} ControlModel_t;

typedef struct _AppModel_t
{
  SensorModel_t *s_models[SENSOR_NUMBER];
  ControlModel_t control_model;

  /* Insert here your custom App Model code */
} AppModel_t;

AppModel_t *getAppModel(void);

/* Device Components APIs ----------------------------------------------------*/

/* STTS22H PnPL Component ----------------------------------------------------*/
uint8_t stts22h_temp_comp_init(void);
char *stts22h_temp_get_key(void);
uint8_t stts22h_temp_get_enable(bool *value);
uint8_t stts22h_temp_get_thresholds_usage(float *value);
uint8_t stts22h_temp_get_threshold_low(float *value);
uint8_t stts22h_temp_get_threshold_high(float *value);
uint8_t stts22h_temp_get_stream_id(int8_t *value);
uint8_t stts22h_temp_get_ep_id(int8_t *value);
uint8_t stts22h_temp_set_enable(bool value);
uint8_t stts22h_temp_set_thresholds_usage(float value);
uint8_t stts22h_temp_set_threshold_low(float value);
uint8_t stts22h_temp_set_threshold_high(float value);

/* LPS22DF PnPL Component ----------------------------------------------------*/
uint8_t lps22df_press_comp_init(void);
char *lps22df_press_get_key(void);
uint8_t lps22df_press_get_enable(bool *value);
uint8_t lps22df_press_get_thresholds_usage(float *value);
uint8_t lps22df_press_get_threshold_low(float *value);
uint8_t lps22df_press_get_threshold_high(float *value);
uint8_t lps22df_press_get_stream_id(int8_t *value);
uint8_t lps22df_press_get_ep_id(int8_t *value);
uint8_t lps22df_press_set_enable(bool value);
uint8_t lps22df_press_set_thresholds_usage(float value);
uint8_t lps22df_press_set_threshold_low(float value);
uint8_t lps22df_press_set_threshold_high(float value);

/* LSM6DSV16X MLC PnPL Component ---------------------------------------------*/
uint8_t lsm6dsv16x_mlc_comp_init(void);
char *lsm6dsv16x_mlc_get_key(void);
uint8_t lsm6dsv16x_mlc_get_enable(bool *value);
uint8_t lsm6dsv16x_mlc_get_thresholds_usage(float *value);
uint8_t lsm6dsv16x_mlc_get_threshold_low(float *value);
uint8_t lsm6dsv16x_mlc_get_threshold_high(float *value);
uint8_t lsm6dsv16x_mlc_get_stream_id(int8_t *value);
uint8_t lsm6dsv16x_mlc_get_ep_id(int8_t *value);
uint8_t lsm6dsv16x_mlc_set_enable(bool value);
uint8_t lsm6dsv16x_mlc_set_thresholds_usage(float value);
uint8_t lsm6dsv16x_mlc_set_threshold_low(float value);
uint8_t lsm6dsv16x_mlc_set_threshold_high(float value);

/* LSM6DSV16X 6D PnPL Component ----------------------------------------------*/
uint8_t lsm6dsv16x6d_acc_comp_init(void);
char *lsm6dsv16x6d_acc_get_key(void);
uint8_t lsm6dsv16x6d_acc_get_enable(bool *value);
uint8_t lsm6dsv16x6d_acc_get_thresholds_usage(float *value);
uint8_t lsm6dsv16x6d_acc_get_threshold_low(float *value);
uint8_t lsm6dsv16x6d_acc_get_threshold_high(float *value);
uint8_t lsm6dsv16x6d_acc_get_stream_id(int8_t *value);
uint8_t lsm6dsv16x6d_acc_get_ep_id(int8_t *value);
uint8_t lsm6dsv16x6d_acc_set_enable(bool value);
uint8_t lsm6dsv16x6d_acc_set_thresholds_usage(float value);
uint8_t lsm6dsv16x6d_acc_set_threshold_low(float value);
uint8_t lsm6dsv16x6d_acc_set_threshold_high(float value);

/* LSM6DSV16X WakeUp PnPL Component ------------------------------------------*/
uint8_t lsm6dsv16xWu_acc_comp_init(void);
char *lsm6dsv16xWu_acc_get_key(void);
uint8_t lsm6dsv16xWu_acc_get_enable(bool *value);
uint8_t lsm6dsv16xWu_acc_get_thresholds_usage(float *value);
uint8_t lsm6dsv16xWu_acc_get_threshold_low(float *value);
uint8_t lsm6dsv16xWu_acc_get_stream_id(int8_t *value);
uint8_t lsm6dsv16xWu_acc_get_ep_id(int8_t *value);
uint8_t lsm6dsv16xWu_acc_set_enable(bool value);
uint8_t lsm6dsv16xWu_acc_set_threshold_low(float value);

/* Control PnPL Component ----------------------------------------------------*/
uint8_t control_comp_init(void);
char *control_get_key(void);
uint8_t control_get_log_status(char **value);
uint8_t control_get_SampleInterval(int32_t *value);
uint8_t control_set_SampleInterval(int32_t value);
uint8_t control_start_log(IControl_t *ifn, int32_t epoc_time);
uint8_t control_stop_log(IControl_t *ifn);
uint8_t control_read_log(IControl_t *ifn);
uint8_t control_delete_log(IControl_t *ifn);
uint8_t control_pause_resume_log(IControl_t *ifn);

/* Device Information PnPL Component -----------------------------------------*/
uint8_t DeviceInformation_comp_init(void);
char *DeviceInformation_get_key(void);
uint8_t DeviceInformation_get_manufacturer(char **value);
uint8_t DeviceInformation_get_model(char **value);
uint8_t DeviceInformation_get_swVersion(char **value);
uint8_t DeviceInformation_get_osName(char **value);
uint8_t DeviceInformation_get_processorArchitecture(char **value);
uint8_t DeviceInformation_get_processorManufacturer(char **value);
uint8_t DeviceInformation_get_totalStorage(float *value);
uint8_t DeviceInformation_get_totalMemory(float *value);

#ifdef __cplusplus
}
#endif

#endif /* APP_MODEL_H_ */
