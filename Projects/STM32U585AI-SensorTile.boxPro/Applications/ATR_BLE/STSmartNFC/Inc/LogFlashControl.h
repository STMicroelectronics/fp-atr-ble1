/**
  ******************************************************************************
  * @file    LogFlashControl.h
  * @author  System Research & Applications Team - Agrate Lab.
  * @version V2.1.0
  * @date    25-July-2023
  * @brief   API for controlling the log saved in flash
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
#ifndef _LOG_FLASH_CONTROL_H_
#define _LOG_FLASH_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Exported defines ----------------------------------------------------------*/

/* Max Log Size 64Kbytes */
#define MAX_LOG_SIZE 0x4000
#define LOG_FIRST_ADDRESS 0x080FA000
#define LOG_LAST_ADDRESS  0x080FE000

/* Exported functions --------------------------------------------------------*/

/* API for Deleting Log Area */
extern void DeleteLogArea(void);
/* API for writing a 32 bits sample on Log Area */
extern void WriteLogData(uint32_t Value);
/* API for Dumping the last bytes on Log Area */
extern void DumpLogData(void);

#ifdef __cplusplus
}
#endif

#endif /* _LOG_FLASH_CONTROL_H_ */

