/**
  ******************************************************************************
  * @file    Lsm6dsv16xwu_Acc_PnPL.c
  * @author  SRA
  * @brief   Lsm6dsv16xwu_Acc PnPL Component Manager
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
  * dtmi:appconfig:steval_mkboxpro:fp_atr_ble1:sensors:lsm6dsv16xWu_acc;1
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

#include "Lsm6dsv16xwu_Acc_PnPL.h"
#include "Lsm6dsv16xwu_Acc_PnPL_vtbl.h"

static const IPnPLComponent_vtbl sLsm6dsv16xwu_Acc_PnPL_CompIF_vtbl =
{
  Lsm6dsv16xwu_Acc_PnPL_vtblGetKey,
  Lsm6dsv16xwu_Acc_PnPL_vtblGetNCommands,
  Lsm6dsv16xwu_Acc_PnPL_vtblGetCommandKey,
  Lsm6dsv16xwu_Acc_PnPL_vtblGetStatus,
  Lsm6dsv16xwu_Acc_PnPL_vtblSetProperty,
  Lsm6dsv16xwu_Acc_PnPL_vtblExecuteFunction
};

/**
  *  Lsm6dsv16xwu_Acc_PnPL internal structure.
  */
struct _Lsm6dsv16xwu_Acc_PnPL
{
  /**
    * Implements the IPnPLComponent interface.
    */
  IPnPLComponent_t component_if;

};

/* Objects instance ----------------------------------------------------------*/
static Lsm6dsv16xwu_Acc_PnPL sLsm6dsv16xwu_Acc_PnPL;

/* Public API definition -----------------------------------------------------*/
IPnPLComponent_t *Lsm6dsv16xwu_Acc_PnPLAlloc()
{
  IPnPLComponent_t *pxObj = (IPnPLComponent_t *) &sLsm6dsv16xwu_Acc_PnPL;
  if (pxObj != NULL)
  {
    pxObj->vptr = &sLsm6dsv16xwu_Acc_PnPL_CompIF_vtbl;
  }
  return pxObj;
}

uint8_t Lsm6dsv16xwu_Acc_PnPLInit(IPnPLComponent_t *_this)
{
  IPnPLComponent_t *component_if = _this;
  PnPLAddComponent(component_if);
  lsm6dsv16xWu_acc_comp_init();
  return 0;
}


/* IPnPLComponent virtual functions definition -------------------------------*/
char *Lsm6dsv16xwu_Acc_PnPL_vtblGetKey(IPnPLComponent_t *_this)
{
  return lsm6dsv16xWu_acc_get_key();
}

uint8_t Lsm6dsv16xwu_Acc_PnPL_vtblGetNCommands(IPnPLComponent_t *_this)
{
  return 0;
}

char *Lsm6dsv16xwu_Acc_PnPL_vtblGetCommandKey(IPnPLComponent_t *_this, uint8_t id)
{
  return "";
}

uint8_t Lsm6dsv16xwu_Acc_PnPL_vtblGetStatus(IPnPLComponent_t *_this, char **serializedJSON, uint32_t *size,
                                            uint8_t pretty)
{
  JSON_Value *tempJSON;
  JSON_Object *JSON_Status;

  tempJSON = json_value_init_object();
  JSON_Status = json_value_get_object(tempJSON);

  bool temp_b = 0;
  lsm6dsv16xWu_acc_get_enable(&temp_b);
  json_object_dotset_boolean(JSON_Status, "lsm6dsv16xWu_acc.enable", temp_b);
  float temp_f = 0;
  lsm6dsv16xWu_acc_get_thresholds_usage(&temp_f);
  uint8_t enum_id = 0;
  if (temp_f == bigger_than)
  {
    enum_id = 3;
  }
  json_object_dotset_number(JSON_Status, "lsm6dsv16xWu_acc.thresholds_usage", enum_id);

  lsm6dsv16xWu_acc_get_threshold_low(&temp_f);
  json_object_dotset_number(JSON_Status, "lsm6dsv16xWu_acc.threshold_low", temp_f);
  /* Next fields are not in DTDL model but added looking @ the component schema
  field (this is :sensors). ONLY for Sensors, Algorithms and Actuators*/
  json_object_dotset_number(JSON_Status, "lsm6dsv16xWu_acc.c_type", COMP_TYPE_SENSOR);
  int8_t temp_int8 = 0;
  lsm6dsv16xWu_acc_get_stream_id(&temp_int8);
  json_object_dotset_number(JSON_Status, "lsm6dsv16xWu_acc.stream_id", temp_int8);
  lsm6dsv16xWu_acc_get_ep_id(&temp_int8);
  json_object_dotset_number(JSON_Status, "lsm6dsv16xWu_acc.ep_id", temp_int8);

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

uint8_t Lsm6dsv16xwu_Acc_PnPL_vtblSetProperty(IPnPLComponent_t *_this, char *serializedJSON)
{
  JSON_Value *tempJSON = json_parse_string(serializedJSON);
  JSON_Object *tempJSONObject = json_value_get_object(tempJSON);

  uint8_t ret = 0;
  if (json_object_dothas_value(tempJSONObject, "lsm6dsv16xWu_acc.enable"))
  {
    bool enable = json_object_dotget_boolean(tempJSONObject, "lsm6dsv16xWu_acc.enable");
    lsm6dsv16xWu_acc_set_enable(enable);
  }
  if (json_object_dothas_value(tempJSONObject, "lsm6dsv16xWu_acc.threshold_low"))
  {
    float threshold_low = (float) json_object_dotget_number(tempJSONObject, "lsm6dsv16xWu_acc.threshold_low");
    lsm6dsv16xWu_acc_set_threshold_low(threshold_low);
  }
  json_value_free(tempJSON);
  return ret;
}

uint8_t Lsm6dsv16xwu_Acc_PnPL_vtblExecuteFunction(IPnPLComponent_t *_this, char *serializedJSON)
{
  return 1;
}
