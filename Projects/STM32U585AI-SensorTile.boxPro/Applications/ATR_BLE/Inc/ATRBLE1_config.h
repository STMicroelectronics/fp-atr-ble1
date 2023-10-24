/**
  ******************************************************************************
  * @file    ATRBLE1_config.h
  * @author  System Research & Applications Team - Agrate Lab.
  * @version V2.1.0
  * @date    25-July-2023
  * @brief   FP-ATR-BLE1 configuration
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
#ifndef __ATRBLE1_CONFIG_H
#define __ATRBLE1_CONFIG_H

/* Exported define ------------------------------------------------------------*/

/* For enabling the printf */
#define ATRBLE1_ENABLE_PRINTF

/* Wake up every 1 seconds */
#define ATRBLE1_WAKE_UP_PERIOD 10000

/* Blink the Green Led every 1 seconds */
#define ATRBLE1_LED_BLINING_PERIOD 10000

/* Default Sample value time in seconds */
#define DATA_DEFAULT_SAMPLE_INT 60

/* Default Epoch Start time if there is not a Valid Configuration:
 *     GMT: Monday 4 July 2022 07:28:53
 */
#define ATRBLE1_DEFAULT_EPOCH_START_TIME 1656919733

/**************************************
  *  Lab/Experimental section defines  *
  ***************************************/

/**************************************
  * Don't Change the following defines *
  ***************************************/

/* Virtual Sensor Configuration */
#define ATRBLE1_VIRTUAL_SENSORS_NUM 5
#define STTS22H_VS_ID         0
#define LPS22DF_VS_ID         1
#define LSM6DSV16X_WU_VS_ID   2
#define LSM6DSV16X_6D_VS_ID   3
#define LSM6DSV16X_MLC_VS_ID  4

/* Accelerometer 6D orientation */
#define ORIENTATION_UNDEF  0x00
#define ORIENTATION_RIGHT  0x01
#define ORIENTATION_TOP    0x02
#define ORIENTATION_LEFT   0x03
#define ORIENTATION_BOTTOM 0x04
#define ORIENTATION_UP     0x05
#define ORIENTATION_DOWN   0x06

/* Package Version only numbers 0->9 */
#define ATRBLE1_VERSION_MAJOR '2'
#define ATRBLE1_VERSION_MINOR '1'
#define ATRBLE1_VERSION_PATCH '0'

/* Package Name */
#define ATRBLE1_PACKAGENAME "ATRBLE1"

/* Bluetooth Advertise Time in seconds */
#define ATRBLE1_DEFAULT_BLE_ADV_TIME 60

/* BlueST-SDK FirmwareIds */
#define ATRBLE1A_BLUEST_SDK_FW_ID 0x0C
#define ATRBLE1B_BLUEST_SDK_FW_ID 0x01

/* NFC Protocol Version and Revision */
#define ATRBLE1_RECORD_VERSION 2
#define ATRBLE1_RECORD_REVISION 1

/* NFC Protocol  BoardId and FirmwareId for STEVAL$MKBOXPROA */
#define ATRBLE1A_NFC_BOARD_ID 3
#define ATRBLE1A_NFC_FW_ID 2

/* NFC Protocol  BoardId and FirmwareId for STEVAL$MKBOXPROB */
#define ATRBLE1B_NFC_BOARD_ID 4
#define ATRBLE1B_NFC_FW_ID 1

#ifdef ATRBLE1_ENABLE_PRINTF
  #define ATRBLE1_PRINTF(...) printf(__VA_ARGS__)
#else /* ATRBLE1_ENABLE_PRINTF */
  #define ATRBLE1_PRINTF(...)
#endif /* ATRBLE1_ENABLE_PRINTF */


#endif /* __ATRBLE1_CONFIG_H */

