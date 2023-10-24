/**
  ******************************************************************************
  * @file    IControl_vtbl.h
  * @author  SRA
  * @brief
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

#ifndef INCLUDE_ICONTROL_VTBL_H_
#define INCLUDE_ICONTROL_VTBL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
  * Create a type name for IControl_vtbl.
  */
typedef struct _IControl_vtbl IControl_vtbl;

struct _IControl_vtbl
{
  uint8_t (*control_start_log)(IControl_t *_this, uint32_t epoc_time);
  uint8_t (*control_stop_log)(IControl_t *_this);
  uint8_t (*control_read_log)(IControl_t *_this);
  uint8_t (*control_delete_log)(IControl_t *_this);
  uint8_t (*control_pause_resume_log)(IControl_t *_this);
};

struct _IControl_t
{
  /**
    * Pointer to the virtual table for the class.
    */
  const IControl_vtbl *vptr;
};

/* Inline functions definition -----------------------------------------------*/
inline uint8_t IControl_start_log(IControl_t *_this, uint32_t epoc_time)
{
  return _this->vptr->control_start_log(_this, epoc_time);
}
inline uint8_t IControl_stop_log(IControl_t *_this)
{
  return _this->vptr->control_stop_log(_this);
}
inline uint8_t IControl_read_log(IControl_t *_this)
{
  return _this->vptr->control_read_log(_this);
}
inline uint8_t IControl_delete_log(IControl_t *_this)
{
  return _this->vptr->control_delete_log(_this);
}
inline uint8_t IControl_pause_resume_log(IControl_t *_this)
{
  return _this->vptr->control_pause_resume_log(_this);
}

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_ICONTROL_VTBL_H_ */
