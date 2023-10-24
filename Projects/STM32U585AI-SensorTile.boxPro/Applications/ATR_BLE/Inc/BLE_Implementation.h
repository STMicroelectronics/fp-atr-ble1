/**
  ******************************************************************************
  * @file    BLE_Implementation.h
  * @author  System Research & Applications Team - Agrate Lab.
  * @version V2.1.0
  * @date    25-July-2023
  * @brief   BLE Implementation header file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BLE_IMPLEMENTATION_H_
#define _BLE_IMPLEMENTATION_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/**
  * User can added here the header file for the selected BLE features.
  * For example:
  * #include "BLE_Environmental.h"
  * #include "BLE_Inertial.h"
  */

#include "BLE_PnPLike.h"
#include "BLE_BinaryContent.h"


/* Exported Defines --------------------------------------------------------*/

/* Package Version firmware */
#define BLE_VERSION_FW_MAJOR    ATRBLE1_VERSION_MAJOR
#define BLE_VERSION_FW_MINOR    ATRBLE1_VERSION_MINOR
#define BLE_VERSION_FW_PATCH    ATRBLE1_VERSION_PATCH

/* Firmware Package Name */
#define BLE_FW_PACKAGENAME      "AtrBle1"

/* Standard Terminal */
#define W2ST_CONNECT_STD_TERM      (1)

/* Standard Error */
#define W2ST_CONNECT_STD_ERR       (1<<1)

/* PnPLike Feature */
#define W2ST_CONNECT_PNPLIKE       (1<<2)

/* BinaryContent Feature */
#define W2ST_CONNECT_BINARYCONTENT (1<<3)

/* Exported Variables ------------------------------------------------------- */
extern uint32_t ConnectionBleStatus;
extern volatile uint8_t  connected;
extern volatile uint32_t RebootBoard;
extern volatile uint32_t SwapBanks;

/* Exported functions ------------------------------------------------------- */
extern void BLE_InitCustomService(void);
extern void InitPnPLComponent(void);
extern void BLE_SetCustomAdvertiseData(uint8_t *manuf_data);
extern void BluetoothInit(void);
extern void EnableDisableDualBoot(void);
extern void RestartBLE_Manager(void);

extern tBleStatus PnPLikeEncapsulateAndSend(uint8_t *data, uint32_t length);
extern tBleStatus BinaryContentEncapsulateAndSend(uint8_t *data, uint32_t length);

/* Exported macro ------------------------------------------------------------*/
#define W2ST_CHECK_CONNECTION(BleChar) ((ConnectionBleStatus&(BleChar)) ? 1 : 0)
#define W2ST_ON_CONNECTION(BleChar)    (ConnectionBleStatus|=(BleChar))
#define W2ST_OFF_CONNECTION(BleChar)   (ConnectionBleStatus&=(~BleChar))

#ifdef __cplusplus
}
#endif

#endif /* _BLE_IMPLEMENTATION_H_ */

