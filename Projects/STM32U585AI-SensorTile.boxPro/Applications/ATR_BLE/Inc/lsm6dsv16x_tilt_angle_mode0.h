/**
  ******************************************************************************
  * @file    lsm6dsv16x_tilt_angle_mode0.h
  * @author  MEMS Software Solutions Team
  * @brief   This file contains the configuration for lsm6dsv16x_tilt_angle_mode0.
  *
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
#ifndef LSM6DSV16X_TILT_ANGLE_MODE0_H
#define LSM6DSV16X_TILT_ANGLE_MODE0_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#ifndef MEMS_UCF_SHARED_TYPES
#define MEMS_UCF_SHARED_TYPES

/** Common data block definition **/
typedef struct
{
  uint8_t address;
  uint8_t data;
} ucf_line_t;

#endif /* MEMS_UCF_SHARED_TYPES */

/** Configuration array generated from Unico Tool **/
const ucf_line_t lsm6dsv16x_tilt_angle_mode0[] =
{
  {.address = 0x10, .data = 0x00,},
  {.address = 0x11, .data = 0x00,},
  {.address = 0x01, .data = 0x80,},
  {.address = 0x04, .data = 0x00,},
  {.address = 0x05, .data = 0x00,},
  {.address = 0x17, .data = 0x40,},
  {.address = 0x02, .data = 0x11,},
  {.address = 0x08, .data = 0xEA,},
  {.address = 0x09, .data = 0x1C,},
  {.address = 0x09, .data = 0x02,},
  {.address = 0x09, .data = 0x26,},
  {.address = 0x09, .data = 0x02,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x0A,},
  {.address = 0x09, .data = 0x01,},
  {.address = 0x09, .data = 0x01,},
  {.address = 0x02, .data = 0x11,},
  {.address = 0x08, .data = 0xFA,},
  {.address = 0x09, .data = 0x14,},
  {.address = 0x09, .data = 0x02,},
  {.address = 0x09, .data = 0x28,},
  {.address = 0x09, .data = 0x02,},
  {.address = 0x09, .data = 0x34,},
  {.address = 0x09, .data = 0x02,},
  {.address = 0x02, .data = 0x21,},
  {.address = 0x08, .data = 0x14,},
  {.address = 0x09, .data = 0x3F,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x04,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x1F,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x02, .data = 0x21,},
  {.address = 0x08, .data = 0x28,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x01, .data = 0x00,},
  {.address = 0x01, .data = 0x80,},
  {.address = 0x17, .data = 0x40,},
  {.address = 0x02, .data = 0x21,},
  {.address = 0x08, .data = 0x34,},
  {.address = 0x09, .data = 0x5A,},
  {.address = 0x09, .data = 0x38,},
  {.address = 0x09, .data = 0x06,},
  {.address = 0x09, .data = 0x80,},
  {.address = 0x09, .data = 0x23,},
  {.address = 0x09, .data = 0x34,},
  {.address = 0x09, .data = 0x04,},
  {.address = 0x09, .data = 0x80,},
  {.address = 0x09, .data = 0xB1,},
  {.address = 0x09, .data = 0x2A,},
  {.address = 0x09, .data = 0x30,},
  {.address = 0x09, .data = 0xC0,},
  {.address = 0x09, .data = 0x00,},
  {.address = 0x09, .data = 0x31,},
  {.address = 0x09, .data = 0x21,},
  {.address = 0x09, .data = 0xE0,},
  {.address = 0x09, .data = 0xBA,},
  {.address = 0x09, .data = 0x35,},
  {.address = 0x09, .data = 0x53,},
  {.address = 0x09, .data = 0xC0,},
  {.address = 0x09, .data = 0x41,},
  {.address = 0x09, .data = 0x37,},
  {.address = 0x09, .data = 0x54,},
  {.address = 0x09, .data = 0xE0,},
  {.address = 0x09, .data = 0x35,},
  {.address = 0x09, .data = 0x3A,},
  {.address = 0x09, .data = 0x09,},
  {.address = 0x09, .data = 0x80,},
  {.address = 0x09, .data = 0x07,},
  {.address = 0x09, .data = 0x39,},
  {.address = 0x09, .data = 0x86,},
  {.address = 0x09, .data = 0xC0,},
  {.address = 0x09, .data = 0xA6,},
  {.address = 0x09, .data = 0x39,},
  {.address = 0x09, .data = 0x87,},
  {.address = 0x09, .data = 0xE0,},
  {.address = 0x09, .data = 0x1E,},
  {.address = 0x09, .data = 0x3B,},
  {.address = 0x09, .data = 0x0B,},
  {.address = 0x09, .data = 0x80,},
  {.address = 0x09, .data = 0xB3,},
  {.address = 0x09, .data = 0x3A,},
  {.address = 0x09, .data = 0xA9,},
  {.address = 0x09, .data = 0xE0,},
  {.address = 0x09, .data = 0xB8,},
  {.address = 0x09, .data = 0x3B,},
  {.address = 0x09, .data = 0x0D,},
  {.address = 0x09, .data = 0x80,},
  {.address = 0x09, .data = 0x75,},
  {.address = 0x09, .data = 0x3B,},
  {.address = 0x09, .data = 0xCB,},
  {.address = 0x09, .data = 0xE0,},
  {.address = 0x09, .data = 0xE4,},
  {.address = 0x09, .data = 0x3B,},
  {.address = 0x09, .data = 0xED,},
  {.address = 0x09, .data = 0xC0,},
  {.address = 0x09, .data = 0xFA,},
  {.address = 0x09, .data = 0x3B,},
  {.address = 0x09, .data = 0xFE,},
  {.address = 0x09, .data = 0xE0,},
  {.address = 0x01, .data = 0x80,},
  {.address = 0x17, .data = 0x00,},
  {.address = 0x04, .data = 0x00,},
  {.address = 0x05, .data = 0x10,},
  {.address = 0x02, .data = 0x01,},
  {.address = 0x01, .data = 0x00,},
  {.address = 0x5E, .data = 0x02,},
  {.address = 0x01, .data = 0x80,},
  {.address = 0x0D, .data = 0x01,},
  {.address = 0x60, .data = 0x25,},
  {.address = 0x45, .data = 0x02,},
  {.address = 0x01, .data = 0x00,},
  {.address = 0x15, .data = 0x00,},
  {.address = 0x17, .data = 0x00,},
  {.address = 0x10, .data = 0x05,},
  {.address = 0x11, .data = 0x00,}
};

#ifdef __cplusplus
}
#endif

#endif /* LSM6DSV16X_TILT_ANGLE_MODE0_H */

