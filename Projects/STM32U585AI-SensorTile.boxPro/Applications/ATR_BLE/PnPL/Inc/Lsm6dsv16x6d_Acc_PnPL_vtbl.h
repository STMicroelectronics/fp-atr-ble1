/**
  ******************************************************************************
  * @file    Lsm6dsv16x6d_Acc_PnPL_vtbl.h
  * @author  SRA
  * @brief   Lsm6dsv16x6d_Acc PnPL Component Manager
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
  * dtmi:appconfig:steval_mkboxpro:fp_atr_ble1:sensors:lsm6dsv16x6d_acc;1
  *
  * Created by: DTDL2PnPL_cGen version 1.1.0
  *
  * WARNING! All changes made to this file will be lost if this is regenerated
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _PNPL__H_
#define _PNPL__H_

#ifdef __cplusplus
extern "C" {
#endif

char *Lsm6dsv16x6d_Acc_PnPL_vtblGetKey(IPnPLComponent_t *_this);
uint8_t Lsm6dsv16x6d_Acc_PnPL_vtblGetNCommands(IPnPLComponent_t *_this);
char *Lsm6dsv16x6d_Acc_PnPL_vtblGetCommandKey(IPnPLComponent_t *_this, uint8_t id);
uint8_t Lsm6dsv16x6d_Acc_PnPL_vtblGetStatus(IPnPLComponent_t *_this, char **serializedJSON, uint32_t *size,
                                            uint8_t pretty);
uint8_t Lsm6dsv16x6d_Acc_PnPL_vtblSetProperty(IPnPLComponent_t *_this, char *serializedJSON);
uint8_t Lsm6dsv16x6d_Acc_PnPL_vtblExecuteFunction(IPnPLComponent_t *_this, char *serializedJSON);

#ifdef __cplusplus
}
#endif

#endif /* _PNPL__H_ */
