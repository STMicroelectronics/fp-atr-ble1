/**
  ******************************************************************************
  * @file    PnPL_Init.h
  * @author  SRA
  * @brief   PnPL Components initialization functions
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

#ifndef PNPL_INIT_H_
#define PNPL_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "PnPLCompManager.h"
#include "Stts22h_Temp_PnPL.h"
#include "Lps22df_Press_PnPL.h"
#include "Lsm6dsv16x_Mlc_PnPL.h"
#include "Lsm6dsv16x6d_Acc_PnPL.h"
#include "Lsm6dsv16xwu_Acc_PnPL.h"
#include "Control_PnPL.h"
#include "Deviceinformation_PnPL.h"

#include "IControl.h"
#include "IControl_vtbl.h"


uint8_t PnPL_Components_Alloc();
uint8_t PnPL_Components_Init(IControl_t iControl);


#ifdef __cplusplus
}
#endif

#endif /* PNPL_INIT_H_ */
