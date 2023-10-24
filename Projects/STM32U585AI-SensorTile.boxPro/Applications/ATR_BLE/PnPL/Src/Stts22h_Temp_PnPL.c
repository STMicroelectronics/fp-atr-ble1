/**
  ******************************************************************************
  * @file    Stts22h_Temp_PnPL.c
  * @author  SRA
  * @brief   Stts22h_Temp PnPL Component Manager
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
  * This file has been auto generated from the following DTDL Component:
  * dtmi:appconfig:steval_mkboxpro:fp_atr_ble1:sensors:stts22h_temp;1
  *
  * Created by: DTDL2PnPL_cGen version 1.1.0
  *
  * WARNING! All changes made to this file will be lost if this is regenerated
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "App_model.h"
#include "IPnPLComponent.h"
#include "IPnPLComponent_vtbl.h"
#include "PnPLCompManager.h"

#include "Stts22h_Temp_PnPL.h"
#include "Stts22h_Temp_PnPL_vtbl.h"

static const IPnPLComponent_vtbl sStts22h_Temp_PnPL_CompIF_vtbl =
{
  Stts22h_Temp_PnPL_vtblGetKey,
  Stts22h_Temp_PnPL_vtblGetNCommands,
  Stts22h_Temp_PnPL_vtblGetCommandKey,
  Stts22h_Temp_PnPL_vtblGetStatus,
  Stts22h_Temp_PnPL_vtblSetProperty,
  Stts22h_Temp_PnPL_vtblExecuteFunction
};

/**
  *  Stts22h_Temp_PnPL internal structure.
  */
struct _Stts22h_Temp_PnPL
{
  /**
    * Implements the IPnPLComponent interface.
    */
  IPnPLComponent_t component_if;

};

/* Objects instance ----------------------------------------------------------*/
static Stts22h_Temp_PnPL sStts22h_Temp_PnPL;

/* Public API definition -----------------------------------------------------*/
IPnPLComponent_t *Stts22h_Temp_PnPLAlloc()
{
  IPnPLComponent_t *pxObj = (IPnPLComponent_t *) &sStts22h_Temp_PnPL;
  if (pxObj != NULL)
  {
    pxObj->vptr = &sStts22h_Temp_PnPL_CompIF_vtbl;
  }
  return pxObj;
}

uint8_t Stts22h_Temp_PnPLInit(IPnPLComponent_t *_this)
{
  IPnPLComponent_t *component_if = _this;
  PnPLAddComponent(component_if);
  stts22h_temp_comp_init();
  return 0;
}


/* IPnPLComponent virtual functions definition -------------------------------*/
char *Stts22h_Temp_PnPL_vtblGetKey(IPnPLComponent_t *_this)
{
  return stts22h_temp_get_key();
}

uint8_t Stts22h_Temp_PnPL_vtblGetNCommands(IPnPLComponent_t *_this)
{
  return 0;
}

char *Stts22h_Temp_PnPL_vtblGetCommandKey(IPnPLComponent_t *_this, uint8_t id)
{
  return "";
}

uint8_t Stts22h_Temp_PnPL_vtblGetStatus(IPnPLComponent_t *_this, char **serializedJSON, uint32_t *size, uint8_t pretty)
{
  JSON_Value *tempJSON;
  JSON_Object *JSON_Status;

  tempJSON = json_value_init_object();
  JSON_Status = json_value_get_object(tempJSON);

  bool temp_b = 0;
  stts22h_temp_get_enable(&temp_b);
  json_object_dotset_boolean(JSON_Status, "stts22h_temp.enable", temp_b);
  float temp_f = 0;
  stts22h_temp_get_thresholds_usage(&temp_f);
  uint8_t enum_id = 0;
  if (temp_f == external_range)
  {
    enum_id = 0;
  }
  else if (temp_f == internal_range)
  {
    enum_id = 1;
  }
  else if (temp_f == less_than)
  {
    enum_id = 2;
  }
  else if (temp_f == bigger_than)
  {
    enum_id = 3;
  }
  json_object_dotset_number(JSON_Status, "stts22h_temp.thresholds_usage", enum_id);
  stts22h_temp_get_threshold_low(&temp_f);
  json_object_dotset_number(JSON_Status, "stts22h_temp.threshold_low", temp_f);
  stts22h_temp_get_threshold_high(&temp_f);
  json_object_dotset_number(JSON_Status, "stts22h_temp.threshold_high", temp_f);
  /* Next fields are not in DTDL model but added looking @ the component schema
  field (this is :sensors). ONLY for Sensors, Algorithms and Actuators*/
  json_object_dotset_number(JSON_Status, "stts22h_temp.c_type", COMP_TYPE_SENSOR);
  int8_t temp_int8 = 0;
  stts22h_temp_get_stream_id(&temp_int8);
  json_object_dotset_number(JSON_Status, "stts22h_temp.stream_id", temp_int8);
  stts22h_temp_get_ep_id(&temp_int8);
  json_object_dotset_number(JSON_Status, "stts22h_temp.ep_id", temp_int8);

  if (pretty == 1)
  {
    *serializedJSON = json_serialize_to_string_pretty(tempJSON);
    *size = json_serialization_size_pretty(tempJSON);
  }
  else
  {
    *serializedJSON = json_serialize_to_string(tempJSON);
    *size = json_serialization_size(tempJSON);
  }

  /* No need to free temp_j as it is part of tempJSON */
  json_value_free(tempJSON);

  return 0;
}

uint8_t Stts22h_Temp_PnPL_vtblSetProperty(IPnPLComponent_t *_this, char *serializedJSON)
{
  JSON_Value *tempJSON = json_parse_string(serializedJSON);
  JSON_Object *tempJSONObject = json_value_get_object(tempJSON);

  uint8_t ret = 0;
  if (json_object_dothas_value(tempJSONObject, "stts22h_temp.enable"))
  {
    bool enable = json_object_dotget_boolean(tempJSONObject, "stts22h_temp.enable");
    stts22h_temp_set_enable(enable);
  }
  if (json_object_dothas_value(tempJSONObject, "stts22h_temp.thresholds_usage"))
  {
    int thresholds_usage = (int)json_object_dotget_number(tempJSONObject, "stts22h_temp.thresholds_usage");
    switch (thresholds_usage)
    {
      case 0:
        stts22h_temp_set_thresholds_usage(external_range);
        break;
      case 1:
        stts22h_temp_set_thresholds_usage(internal_range);
        break;
      case 2:
        stts22h_temp_set_thresholds_usage(less_than);
        break;
      case 3:
        stts22h_temp_set_thresholds_usage(bigger_than);
        break;
    }
  }
  if (json_object_dothas_value(tempJSONObject, "stts22h_temp.threshold_low"))
  {
    float threshold_low = (float) json_object_dotget_number(tempJSONObject, "stts22h_temp.threshold_low");
    stts22h_temp_set_threshold_low(threshold_low);
  }
  if (json_object_dothas_value(tempJSONObject, "stts22h_temp.threshold_high"))
  {
    float threshold_high = (float) json_object_dotget_number(tempJSONObject, "stts22h_temp.threshold_high");
    stts22h_temp_set_threshold_high(threshold_high);
  }
  json_value_free(tempJSON);
  return ret;
}

uint8_t Stts22h_Temp_PnPL_vtblExecuteFunction(IPnPLComponent_t *_this, char *serializedJSON)
{
  return 1;
}
