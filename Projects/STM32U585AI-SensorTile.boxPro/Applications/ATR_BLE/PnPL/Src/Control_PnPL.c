/**
  ******************************************************************************
  * @file    Control_PnPL.c
  * @author  SRA
  * @brief   Control PnPL Component Manager
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
  * dtmi:appconfig:steval_mkboxpro:fp_atr_ble1:other:control;1
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

#include "Control_PnPL.h"
#include "Control_PnPL_vtbl.h"

static const IPnPLComponent_vtbl sControl_PnPL_CompIF_vtbl =
{
  Control_PnPL_vtblGetKey,
  Control_PnPL_vtblGetNCommands,
  Control_PnPL_vtblGetCommandKey,
  Control_PnPL_vtblGetStatus,
  Control_PnPL_vtblSetProperty,
  Control_PnPL_vtblExecuteFunction
};

/**
  *  Control_PnPL internal structure.
  */
struct _Control_PnPL
{
  /**
    * Implements the IPnPLComponent interface.
    */
  IPnPLComponent_t component_if;
  /**
    * Contains Control functions pointers.
    */
  IControl_t *cmdIF;
};

/* Objects instance ----------------------------------------------------------*/
static Control_PnPL sControl_PnPL;

/* Public API definition -----------------------------------------------------*/
IPnPLComponent_t *Control_PnPLAlloc()
{
  IPnPLComponent_t *pxObj = (IPnPLComponent_t *) &sControl_PnPL;
  if (pxObj != NULL)
  {
    pxObj->vptr = &sControl_PnPL_CompIF_vtbl;
  }
  return pxObj;
}

uint8_t Control_PnPLInit(IPnPLComponent_t *_this,  IControl_t *inf)
{
  IPnPLComponent_t *component_if = _this;
  PnPLAddComponent(component_if);
  Control_PnPL *p_if_owner = (Control_PnPL *) _this;
  p_if_owner->cmdIF = inf;
  control_comp_init();
  return 0;
}


/* IPnPLComponent virtual functions definition -------------------------------*/
char *Control_PnPL_vtblGetKey(IPnPLComponent_t *_this)
{
  return control_get_key();
}

uint8_t Control_PnPL_vtblGetNCommands(IPnPLComponent_t *_this)
{
  return 5;
}

char *Control_PnPL_vtblGetCommandKey(IPnPLComponent_t *_this, uint8_t id)
{
  switch (id)
  {
    case 0:
      return "control*start_log";
      break;
    case 1:
      return "control*stop_log";
      break;
    case 2:
      return "control*read_log";
      break;
    case 3:
      return "control*delete_log";
      break;
    case 4:
      return "control*pause_resume_log";
      break;
  }
  return 0;
}

uint8_t Control_PnPL_vtblGetStatus(IPnPLComponent_t *_this, char **serializedJSON, uint32_t *size, uint8_t pretty)
{
  JSON_Value *tempJSON;
  JSON_Object *JSON_Status;

  tempJSON = json_value_init_object();
  JSON_Status = json_value_get_object(tempJSON);

  char *temp_s = "";
  control_get_log_status(&temp_s);
  json_object_dotset_string(JSON_Status, "control.log_status", temp_s);
  int32_t temp_i = 0;
  control_get_SampleInterval(&temp_i);
  json_object_dotset_number(JSON_Status, "control.SampleInterval", temp_i);
  json_object_dotset_number(JSON_Status, "control.c_type", COMP_TYPE_OTHER);

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

uint8_t Control_PnPL_vtblSetProperty(IPnPLComponent_t *_this, char *serializedJSON)
{
  JSON_Value *tempJSON = json_parse_string(serializedJSON);
  JSON_Object *tempJSONObject = json_value_get_object(tempJSON);

  uint8_t ret = 0;
  if (json_object_dothas_value(tempJSONObject, "control.SampleInterval"))
  {
    int32_t SampleInterval = (int32_t) json_object_dotget_number(tempJSONObject, "control.SampleInterval");
    control_set_SampleInterval(SampleInterval);
  }
  json_value_free(tempJSON);
  return ret;
}

uint8_t Control_PnPL_vtblExecuteFunction(IPnPLComponent_t *_this, char *serializedJSON)
{
  Control_PnPL *p_if_owner = (Control_PnPL *) _this;
  JSON_Value *tempJSON = json_parse_string(serializedJSON);
  JSON_Object *tempJSONObject = json_value_get_object(tempJSON);
  if (json_object_dothas_value(tempJSONObject, "control*start_log.epoc_time"))
  {
    int32_t epoc_time = (int32_t) json_object_dotget_number(tempJSONObject, "control*start_log.epoc_time");
    control_start_log(p_if_owner->cmdIF, epoc_time);
  }
  if (json_object_dothas_value(tempJSONObject, "control*stop_log"))
  {
    control_stop_log(p_if_owner->cmdIF);
  }
  if (json_object_dothas_value(tempJSONObject, "control*read_log"))
  {
    control_read_log(p_if_owner->cmdIF);
  }
  if (json_object_dothas_value(tempJSONObject, "control*delete_log"))
  {
    control_delete_log(p_if_owner->cmdIF);
  }
  if (json_object_dothas_value(tempJSONObject, "control*pause_resume_log"))
  {
    control_pause_resume_log(p_if_owner->cmdIF);
  }
  json_value_free(tempJSON);
  return 0;
}
