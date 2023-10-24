/**
  ******************************************************************************
  * @file    PnPL_Init.c
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

#include "PnPL_Init.h"

static IPnPLComponent_t *pStts22h_Temp_PnPLObj = NULL;
static IPnPLComponent_t *pLps22df_Press_PnPLObj = NULL;
static IPnPLComponent_t *pLsm6dsv16x_Mlc_PnPLObj = NULL;
static IPnPLComponent_t *pLsm6dsv16x6d_Acc_PnPLObj = NULL;
static IPnPLComponent_t *pLsm6dsv16xwu_Acc_PnPLObj = NULL;
static IPnPLComponent_t *pControl_PnPLObj = NULL;
static IPnPLComponent_t *pDeviceinformation_PnPLObj = NULL;

uint8_t PnPL_Components_Alloc()
{
  /* PnPL Components Allocation */
  pStts22h_Temp_PnPLObj = Stts22h_Temp_PnPLAlloc();
  pLps22df_Press_PnPLObj = Lps22df_Press_PnPLAlloc();
  pLsm6dsv16x_Mlc_PnPLObj = Lsm6dsv16x_Mlc_PnPLAlloc();
  pLsm6dsv16x6d_Acc_PnPLObj = Lsm6dsv16x6d_Acc_PnPLAlloc();
  pLsm6dsv16xwu_Acc_PnPLObj = Lsm6dsv16xwu_Acc_PnPLAlloc();
  pControl_PnPLObj = Control_PnPLAlloc();
  pDeviceinformation_PnPLObj = Deviceinformation_PnPLAlloc();
  return 0;
}

uint8_t PnPL_Components_Init(IControl_t iControl)
{
  /* Init&Add PnPL Components */
  Stts22h_Temp_PnPLInit(pStts22h_Temp_PnPLObj);
  Lps22df_Press_PnPLInit(pLps22df_Press_PnPLObj);
  Lsm6dsv16x_Mlc_PnPLInit(pLsm6dsv16x_Mlc_PnPLObj);
  Lsm6dsv16x6d_Acc_PnPLInit(pLsm6dsv16x6d_Acc_PnPLObj);
  Lsm6dsv16xwu_Acc_PnPLInit(pLsm6dsv16xwu_Acc_PnPLObj);
  Control_PnPLInit(pControl_PnPLObj, &iControl);
  Deviceinformation_PnPLInit(pDeviceinformation_PnPLObj);
  return 0;
}