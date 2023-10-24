/**
  ******************************************************************************
  * @file    BLE_Implementation.c
  * @author  System Research & Applications Team - Agrate Lab.
  * @version V2.1.0
  * @date    25-July-2023
  * @brief   BLE Implementation template file.
  *          This file should be copied to the application folder and renamed
  *          to BLE_Implementation.c.
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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "ATRBLE1_config.h"
#include "BLE_Manager.h"
#include "OTA.h"
#include "main.h"
#include "SensorTileBoxPro.h"
#include "SensorTileBoxPro_env_sensors.h"

#include "PnPLCompManager.h"
#include "IControl.h"
#include "IControl_vtbl.h"

#include "Stts22h_Vs_PnPL.h"
#include "Lsm6dsv16x_Mlc_Vs_PnPL.h"
#include "Lsm6dsv16x_6d_Vs_PnPL.h"
#include "Lsm6dsv16x_wu_Vs_PnPL.h"
#include "Lps22df_Vs_PnPL.h"
#include "Control_PnPL.h"
#include "Deviceinformation_PnPL.h"

#include "SmartNFC.h"
#include "LogControl.h"

/* Exported Variables --------------------------------------------------------*/
volatile uint8_t  connected   = FALSE;
volatile uint32_t RebootBoard = 0;
volatile uint32_t SwapBanks   = 0;
uint32_t ConnectionBleStatus = 0;

uint32_t SizeOfUpdateBlueFW = 0;

/* Imported Variables --------------------------------------------------------*/
extern TIM_HandleTypeDef TimCCHandle;

/* Private variables ---------------------------------------------------------*/
static uint32_t NeedToRebootBoard = 0;
static uint32_t NeedToSwapBanks = 0;

static IPnPLComponent_t *pStts22hVsPnPLObj = NULL;
static IPnPLComponent_t *pLsm6dsv16xMCLVsPnPLObj = NULL;
static IPnPLComponent_t *pLsm6dsv16x6DVsPnPLObj = NULL;
static IPnPLComponent_t *pLsm6dsv16xWUVsPnPLObj = NULL;
static IPnPLComponent_t *pLps22dfVsPnPLObj = NULL;
static IPnPLComponent_t *pControlPnPLObj = NULL;
static IPnPLComponent_t *pDeviceInformationPnPLObj = NULL;

static IControl_t iControl;


/* Private functions ---------------------------------------------------------*/
static uint32_t DebugConsoleParsing(uint8_t *att_data, uint8_t data_length);
static void DisconnectionCompletedFunction(void);
static void ConnectionCompletedFunction(uint16_t ConnectionHandle, uint8_t Address_Type, uint8_t Addr[6]);
static uint32_t DebugConsoleCommandParsing(uint8_t *att_data, uint8_t data_length);

/**********************************************************************************************
  * Callback functions prototypes to manage the extended configuration characteristic commands *
  **********************************************************************************************/
static void ExtExtConfigUidCommandCallback(uint8_t **UID);
static void ExtConfigInfoCommandCallback(uint8_t *Answer);
static void ExtConfigHelpCommandCallback(uint8_t *Answer);
static void ExtConfigVersionFwCommandCallback(uint8_t *Answer);
static void ExtConfigSetNameCommandCallback(uint8_t *NewName);
static void ExtConfigReadBanksFwIdCommandCallback(uint8_t *CurBank, uint16_t *FwId1, uint16_t *FwId2);
static void ExtConfigBanksSwapCommandCallback(void);
static void MTUExcahngeRespEvent(int32_t MaxCharLength);

static void NotifyEventPnPLike(BLE_NotifyEvent_t Event);
static void WriteRequestPnPLike(uint8_t *received_msg, uint8_t msg_length);
static void WriteRequestBinaryContent(uint8_t *received_msg, uint32_t msg_length);

static void NotifyEventBinaryContent(BLE_NotifyEvent_t Event);


/** @brief Initialize the BlueNRG stack and services
  * @param  None
  * @retval None
  */
void BluetoothInit(void)
{
  /* BlueNRG stack setting */
  BLE_StackValue.ConfigValueOffsets                   = CONFIG_DATA_PUBADDR_OFFSET;
  BLE_StackValue.ConfigValuelength                    = CONFIG_DATA_PUBADDR_LEN;
  BLE_StackValue.GAP_Roles                            = GAP_PERIPHERAL_ROLE;
  BLE_StackValue.IO_capabilities                      = IO_CAP_DISPLAY_ONLY;
  BLE_StackValue.AuthenticationRequirements           = BONDING;
  BLE_StackValue.MITM_ProtectionRequirements          = MITM_PROTECTION_REQUIRED;
  BLE_StackValue.SecureConnectionSupportOptionCode    = SC_IS_SUPPORTED;
  BLE_StackValue.SecureConnectionKeypressNotification = KEYPRESS_IS_NOT_SUPPORTED;

  /* Use BLE Random Address */
  BLE_StackValue.OwnAddressType = RANDOM_ADDR;

  /* Set the Board Name */
  {
    uint8_t *BoardName = ReadFlashBoardName();

    if (BoardName != NULL)
    {
      /* If there is Saved Board Name */
      memcpy(BLE_StackValue.BoardName, BoardName, 8);
    }
    else
    {
      /* Use the Default Board Name */
      sprintf(BLE_StackValue.BoardName, "%s", BLE_FW_PACKAGENAME);
    }
  }

  /* To set the TX power level of the bluetooth device ( -2 dBm )*/
  BLE_StackValue.EnableHighPowerMode = 0; /*  Low Power */

  /* Values: 0x00 ... 0x31 - The value depends on the device */
  BLE_StackValue.PowerAmplifierOutputLevel = 25;

  /* BlueNRG services setting */
  BLE_StackValue.EnableConfig    = 0;
  BLE_StackValue.EnableConsole   = 1;
  BLE_StackValue.EnableExtConfig = 1;

  /* For Enabling the Secure Connection */
  BLE_StackValue.EnableSecureConnection = 0;

  if (BLE_StackValue.EnableSecureConnection)
  {
    /* Using the Secure Connection, the Rescan should be done by BLE chip */
    BLE_StackValue.ForceRescan = 0;
  }
  else
  {
    BLE_StackValue.ForceRescan = 1;
  }

  InitBleManager();
}


/** @brief Initialize the Components Used for PnP-L
  * @param  None
  * @retval None
  */
void InitPnPLComponent(void)
{
  /* PnP-L Components Allocation */
  pStts22hVsPnPLObj = Stts22h_Vs_PnPLAlloc();
  pControlPnPLObj = Control_PnPLAlloc();
  pLsm6dsv16xMCLVsPnPLObj = Lsm6dsv16x_Mlc_Vs_PnPLAlloc();
  pLsm6dsv16x6DVsPnPLObj = Lsm6dsv16x_6d_Vs_PnPLAlloc();
  pLsm6dsv16xWUVsPnPLObj = Lsm6dsv16x_Wu_Vs_PnPLAlloc();
  pLps22dfVsPnPLObj = Lps22df_Vs_PnPLAlloc();
  pDeviceInformationPnPLObj = Deviceinformation_PnPLAlloc();

  /* Init&Add PnP-L Components */
  Stts22h_Vs_PnPLInit(pStts22hVsPnPLObj);
  Control_PnPLInit(pControlPnPLObj, &iControl);
  Lsm6dsv16x_Mlc_Vs_PnPLInit(pLsm6dsv16xMCLVsPnPLObj);
  Lsm6dsv16x_6d_Vs_PnPLInit(pLsm6dsv16x6DVsPnPLObj);
  Lsm6dsv16x_Wu_Vs_PnPLInit(pLsm6dsv16xWUVsPnPLObj);
  Lps22df_Vs_PnPLInit(pLps22dfVsPnPLObj);
  Deviceinformation_PnPLInit(pDeviceInformationPnPLObj);
}

/** @brief Initialize the Components Used for PnP-L
  * @param  None
  * @retval None
  */
void RestartBLE_Manager(void)
{
  HAL_NVIC_DisableIRQ(EXTI11_IRQn);

  /* Stop the TIM Base generation in interrupt mode for Led Blinking */
  BlinkStopPeriodicallyTimer();
  BSP_LED_Off(LED_GREEN);

  ResetBleManager();
  BluetoothInit();
}

/**
  * @brief  Enable BlueNRG-LP Interrupt.
  * @param  None
  * @retval None
  */
void InitBLEIntForBlueNRGLP(void)
{
  HAL_EXTI_GetHandle(&hexti11, EXTI_LINE_11);
  HAL_EXTI_RegisterCallback(&hexti11, HAL_EXTI_COMMON_CB_ID, hci_tl_lowlevel_isr);
  HAL_NVIC_SetPriority(EXTI11_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI11_IRQn);
}

/**
  * @brief  Custom Service Initialization.
  * @param  None
  * @retval None
  */
void BLE_InitCustomService(void)
{
  /* Custom Function for Debug Console Command parsing */
  CustomDebugConsoleParsingCallback = DebugConsoleParsing;

  /* Custom Function for Connection Completed */
  CustomConnectionCompleted = ConnectionCompletedFunction;

  /* Custom Function for Disconnection Completed */
  CustomDisconnectionCompleted = DisconnectionCompletedFunction;

  /* Characteristic allocation for PnPLike feature */
  CustomWriteRequestPnPLike = WriteRequestPnPLike;
  if (BleManagerAddChar(BLE_InitPnPLikeService()) == 0)
  {
    ATRBLE1_PRINTF("Error adding PnPLike Service\r\n");
  }
  CustomNotifyEventPnPLike = NotifyEventPnPLike;

  /* Characteristic allocation for BinaryContent feature */
  CustomWriteRequestBinaryContent = WriteRequestBinaryContent;
  if (BleManagerAddChar(BLE_InitBinaryContentService()) == 0)
  {
    ATRBLE1_PRINTF("Error adding BinaryContent Service\r\n");
  }
  CustomNotifyEventBinaryContent = NotifyEventBinaryContent;

  /* For Receiving information on Response Event for a MTU Exchange Event */
  CustomMTUExchangeRespEvent = MTUExcahngeRespEvent;

  /***********************************************************************************
    * Callback functions to manage the extended configuration characteristic commands *
    ***********************************************************************************/
  CustomExtConfigUidCommandCallback       = ExtExtConfigUidCommandCallback;
  CustomExtConfigInfoCommandCallback      = ExtConfigInfoCommandCallback;
  CustomExtConfigHelpCommandCallback      = ExtConfigHelpCommandCallback;
  CustomExtConfigVersionFwCommandCallback = ExtConfigVersionFwCommandCallback;
  CustomExtConfigSetNameCommandCallback   = ExtConfigSetNameCommandCallback;

  CustomExtConfigReadBanksFwIdCommandCallback       = ExtConfigReadBanksFwIdCommandCallback;
  {
    uint16_t FwId1, FwId2;

    ReadFlashBanksFwId(&FwId1, &FwId2);
    if (FwId2 != OTA_OTA_FW_ID_NOT_VALID)
    {
      /* Enable the Banks Swap only if there is a valid fw on second bank */
      CustomExtConfigBanksSwapCommandCallback           = ExtConfigBanksSwapCommandCallback;
    }
  }
}

/**
  * @brief  Callback Called after a MTU Exchange Event
  * @param  int32_t MaxCharLength
  * @retval none
  */
static void MTUExcahngeRespEvent(int32_t MaxCharLength)
{
  BLE_BinaryContentSetMaxCharLength(MaxCharLength);
  ATRBLE1_PRINTF("BLE_BinaryContentSetMaxCharLength ->%d\r\n", MaxCharLength);
}

/**
  * @brief  Set Custom Advertise Data.
  * @param  uint8_t *manuf_data: Advertise Data
  * @retval None
  */
void BLE_SetCustomAdvertiseData(uint8_t *manuf_data)
{
  manuf_data[BLE_MANAGER_CUSTOM_FIELD1] = ATRBLE1_BLUEST_SDK_FW_ID;
  manuf_data[BLE_MANAGER_CUSTOM_FIELD2] = CurrentActiveBank;

  switch (CurrentLogStatus)
  {
    case STNFC_LOG_STOP:
      manuf_data[BLE_MANAGER_CUSTOM_FIELD3] = 0; /* Stop */
      break;
    case STNFC_LOG_RUNNING:
      manuf_data[BLE_MANAGER_CUSTOM_FIELD3] = 1; /* Logging */
      break;
    case STNFC_LOG_PAUSED:
      manuf_data[BLE_MANAGER_CUSTOM_FIELD3] = 2; /* Paused */
      break;
  }
  manuf_data[BLE_MANAGER_CUSTOM_FIELD4] = 0x00; /* Not Used */
}

/**
  * @brief  Callback Function for Un/Subscription PnPLike Feature
  * @param  BLE_NotifyEvent_t Event Sub/Unsub
  * @retval None
  */
static void NotifyEventPnPLike(BLE_NotifyEvent_t Event)
{
  if (Event == BLE_NOTIFY_SUB)
  {
    W2ST_ON_CONNECTION(W2ST_CONNECT_PNPLIKE);
    ATRBLE1_PRINTF("PnPLike Subscribe\r\n");
  }
  else if (Event == BLE_NOTIFY_UNSUB)
  {
    W2ST_OFF_CONNECTION(W2ST_CONNECT_PNPLIKE);
    ATRBLE1_PRINTF("PnPLike Unsubscribe\r\n");
  }
}

/**
  * @brief  Callback called when there is a write on PnPLike feature
  * @param  uint8_t* received_msg received message
  * @param  uint8_t msg_length message length
  * @retval None
  */
static void WriteRequestPnPLike(uint8_t *received_msg, uint8_t msg_length)
{
  PnPLCommand_t PnPLCommand;
  ATRBLE1_PRINTF("PnPMessage Received\r\n");
  ATRBLE1_PRINTF("\t<%.*s>\r\n", msg_length, received_msg);

  PnPLParseCommand((char *)received_msg, &PnPLCommand);

  if (PnPLCommand.comm_type == PNPL_CMD_GET)
  {
    char *SerializedJSON;
    uint32_t size;

    PnPLSerializeResponse(&PnPLCommand, &SerializedJSON, &size, 0);

    ATRBLE1_PRINTF("--> <%.*s>\r\n", size, SerializedJSON);

    PnPLikeEncapsulateAndSend((uint8_t *) SerializedJSON, size);
    free(SerializedJSON);
  }
}

/**
  * @brief  Callback called when there is a write on Binary Content feature
  * @param  uint8_t* received_msg received message
  * @param  uint32_t msg_length message length
  * @retval None
  */
static void WriteRequestBinaryContent(uint8_t *received_msg, uint32_t msg_length)
{
  int32_t Counter;
  ATRBLE1_PRINTF("\r\n");
  ATRBLE1_PRINTF("BinaryContent Received msg_length=%d\r\n", msg_length);
  for (Counter = 0; Counter < msg_length; Counter += 4)
  {
    ATRBLE1_PRINTF("| %02X", received_msg[Counter]);
    ATRBLE1_PRINTF("%02X", received_msg[Counter + 1]);
    ATRBLE1_PRINTF("%02X", received_msg[Counter + 2]);
    ATRBLE1_PRINTF("%02X", received_msg[Counter + 3]);
  }
  ATRBLE1_PRINTF("\r\n");
}

/**
  * @brief  Callback Function for Un/Subscription BinaryContent Feature
  * @param  BLE_NotifyEvent_t Event Sub/Unsub
  * @retval None
  */
static void NotifyEventBinaryContent(BLE_NotifyEvent_t Event)
{
  if (Event == BLE_NOTIFY_SUB)
  {
    W2ST_ON_CONNECTION(W2ST_CONNECT_BINARYCONTENT);
    ATRBLE1_PRINTF("BinaryContent Subscribe\r\n");
  }
  else if (Event == BLE_NOTIFY_UNSUB)
  {
    W2ST_OFF_CONNECTION(W2ST_CONNECT_BINARYCONTENT);
    ATRBLE1_PRINTF("BinaryContent Unsubscribe\r\n");
  }
}

/**
  * @brief  Encapsulate and Send data to PnPLike Feature
  * @param  uint8_t *data string to write
  * @param  uint32_t length length of string to write
  * @retval tBleStatus      Status
  */
tBleStatus PnPLikeEncapsulateAndSend(uint8_t *data, uint32_t length)
{
  uint32_t tot_len, j;
  uint8_t *JSON_string_command_wTP;
  uint32_t length_wTP;
  uint32_t len;

  if ((length % 19U) == 0U)
  {
    length_wTP = (length / 19U) + length;
  }
  else
  {
    length_wTP = (length / 19U) + 1U + length;
  }

  JSON_string_command_wTP = malloc(sizeof(uint8_t) * length_wTP);

  if (JSON_string_command_wTP == NULL)
  {
    ATRBLE1_PRINTF("Error: Mem calloc error [%lu]: %d@%s\r\n", length, __LINE__, __FILE__);
    return BLE_STATUS_ERROR;
  }
  else
  {
    tot_len = BLE_Command_TP_Encapsulate(JSON_string_command_wTP, data, length, 20);

    /* Data are sent as notifications*/
    j = 0;
    while (j < tot_len)
    {
      len = MIN(20U, (tot_len - j));
      if (BLE_PnPLikeUpdate(JSON_string_command_wTP + j, (uint8_t)len) != (tBleStatus)BLE_STATUS_SUCCESS)
      {
        return BLE_STATUS_ERROR;
      }
      HAL_Delay(20);
      j += len;
    }
    free(JSON_string_command_wTP);
    return BLE_STATUS_SUCCESS;
  }
}

/**
  * @brief  Encapsulate and Send data to BinaryContent Feature
  * @param  uint8_t *data string to write
  * @param  uint32_t length length of string to write
  * @retval tBleStatus      Status
  */
tBleStatus BinaryContentEncapsulateAndSend(uint8_t *data, uint32_t length)
{
  uint32_t tot_len, j;
  uint8_t *JSON_string_command_wTP;
  uint32_t length_wTP;
  uint32_t len;

  int32_t MaxBinaryContentUpdate = BLE_BinaryContentGetMaxCharLength();
  int32_t MaxBinaryContentUpdateMinus1 = MaxBinaryContentUpdate - 1;

  if ((length % MaxBinaryContentUpdateMinus1) == 0U)
  {
    length_wTP = (length / MaxBinaryContentUpdateMinus1) + length;
  }
  else
  {
    length_wTP = (length / MaxBinaryContentUpdateMinus1) + 1U + length;
  }

  JSON_string_command_wTP = malloc(sizeof(uint8_t) * length_wTP);

  if (JSON_string_command_wTP == NULL)
  {
    ATRBLE1_PRINTF("Error: Mem calloc error [%lu]: %d@%s\r\n", length, __LINE__, __FILE__);
    return BLE_STATUS_ERROR;
  }
  else
  {
    tot_len = BLE_Command_TP_Encapsulate(JSON_string_command_wTP, data, length, MaxBinaryContentUpdate);

    /* Data are sent as notifications*/
    j = 0;
    while (j < tot_len)
    {
      len = MIN(MaxBinaryContentUpdate, (tot_len - j));
      if (BLE_BinaryContentUpdate(JSON_string_command_wTP + j, (uint8_t)len) != (tBleStatus)BLE_STATUS_SUCCESS)
      {
        return BLE_STATUS_ERROR;
      }
      HAL_Delay(20);
      j += len;
    }
    free(JSON_string_command_wTP);
    return BLE_STATUS_SUCCESS;
  }
}

void Write_Request_BinaryContent(void *BleCharPointer, uint16_t handle, uint16_t Offset, uint8_t data_length,
                                 uint8_t *att_data)
{
  ATRBLE1_PRINTF("Ciao\r\n");
}

/**
  * @brief  This function makes the parsing of the Debug Console
  * @param  uint8_t *att_data attribute data
  * @param  uint8_t data_length length of the data
  * @retval uint32_t SendBackData true/false
  */
static uint32_t DebugConsoleParsing(uint8_t *att_data, uint8_t data_length)
{
  /* By default Answer with the same message received */
  uint32_t SendBackData = 1;

  if (SizeOfUpdateBlueFW != 0)
  {
    /* Firmware update */
    int8_t RetValue = UpdateFWBlueMS(&SizeOfUpdateBlueFW, att_data, data_length, 1);
    if (RetValue != 0)
    {
      Term_Update(((uint8_t *)&RetValue), 1);
      if (RetValue == 1)
      {
        /* if OTA checked */
        ATRBLE1_PRINTF("%s will restart after the disconnection\r\n", ATRBLE1_PACKAGENAME);
        HAL_Delay(1000);
        NeedToSwapBanks = 1;
      }
    }
    SendBackData = 0;
  }
  else
  {
    /* Received one write from Client on Terminal characteristc */
    SendBackData = DebugConsoleCommandParsing(att_data, data_length);
  }

  return SendBackData;
}

/**
  * @brief  This function makes the parsing of the Debug Console Commands
  * @param  uint8_t *att_data attribute data
  * @param  uint8_t data_length length of the data
  * @retval uint32_t SendBackData true/false
  */
static uint32_t DebugConsoleCommandParsing(uint8_t *att_data, uint8_t data_length)
{
  uint32_t SendBackData = 1;

  /* Help Command */
  if (!strncmp("help", (char *)(att_data), 4))
  {
    /* Print Legend */
    SendBackData = 0;

    BytesToWrite = sprintf((char *)BufferToWrite,
                           "info\n");
    Term_Update(BufferToWrite, BytesToWrite);
  }
  else if (!strncmp("versionFw", (char *)(att_data), 9))
  {
    BytesToWrite = sprintf((char *)BufferToWrite, "%s_%s_%c.%c.%c\r\n",
                           "U585",
                           ATRBLE1_PACKAGENAME,
                           ATRBLE1_VERSION_MAJOR,
                           ATRBLE1_VERSION_MINOR,
                           ATRBLE1_VERSION_PATCH);
    Term_Update(BufferToWrite, BytesToWrite);
    SendBackData = 0;
  }
  else if (!strncmp("info", (char *)(att_data), 4))
  {
    SendBackData = 0;

    BytesToWrite = sprintf((char *)BufferToWrite, "\r\nSTMicroelectronics %s:\n"
                           "\tVersion %c.%c.%c\n"
                           "\tTM32U585AI-SensorTile.box-Pro board"
                           "\n",
                           ATRBLE1_PACKAGENAME,
                           ATRBLE1_VERSION_MAJOR, ATRBLE1_VERSION_MINOR, ATRBLE1_VERSION_PATCH);
    Term_Update(BufferToWrite, BytesToWrite);

    BytesToWrite = sprintf((char *)BufferToWrite, "\t(HAL %ld.%ld.%ld_%ld)\n"
                           "\tCompiled %s %s"
#if defined (__IAR_SYSTEMS_ICC__)
                           " (IAR)\n",
#elif defined (__CC_ARM)
                           " (KEIL)\n",
#elif defined (__GNUC__)
                           " (STM32CubeIDE)\n",
#endif
                           HAL_GetHalVersion() >> 24,
                           (HAL_GetHalVersion() >> 16) & 0xFF,
                           (HAL_GetHalVersion() >> 8) & 0xFF,
                           HAL_GetHalVersion()      & 0xFF,
                           __DATE__, __TIME__);
    Term_Update(BufferToWrite, BytesToWrite);
    BytesToWrite = sprintf((char *)BufferToWrite, "Current Bank =%ld\n", CurrentActiveBank);
    Term_Update(BufferToWrite, BytesToWrite);
  }
  else if (!strncmp("upgradeFw", (char *)(att_data), 9))
  {
    uint32_t uwCRCValue;
    uint8_t *PointerByte = (uint8_t *) &SizeOfUpdateBlueFW;

    PointerByte[0] = att_data[ 9];
    PointerByte[1] = att_data[10];
    PointerByte[2] = att_data[11];
    PointerByte[3] = att_data[12];

    /* Check the Maximum Possible OTA size */
    if (SizeOfUpdateBlueFW > OTA_MAX_PROG_SIZE)
    {
      ATRBLE1_PRINTF("OTA %s SIZE=%ld > %d Max Allowed\r\n", ATRBLE1_PACKAGENAME, SizeOfUpdateBlueFW, OTA_MAX_PROG_SIZE);
      /* Answer with a wrong CRC value for signaling the problem to BlueMS application */
      BufferToWrite[0] = att_data[13];
      BufferToWrite[1] = (att_data[14] != 0) ? 0 : 1; /* In order to be sure to have a wrong CRC */
      BufferToWrite[2] = att_data[15];
      BufferToWrite[3] = att_data[16];
      BytesToWrite = 4;
      Term_Update(BufferToWrite, BytesToWrite);
    }
    else
    {
      PointerByte = (uint8_t *) &uwCRCValue;
      PointerByte[0] = att_data[13];
      PointerByte[1] = att_data[14];
      PointerByte[2] = att_data[15];
      PointerByte[3] = att_data[16];

      ATRBLE1_PRINTF("OTA %s SIZE=%ld uwCRCValue=%lx\r\n", ATRBLE1_PACKAGENAME, SizeOfUpdateBlueFW, uwCRCValue);

      /* Reset the Flash */
      StartUpdateFWBlueMS(SizeOfUpdateBlueFW, uwCRCValue);

#if 0
      /* Reduce the connection interval */
      {
        tBleStatus ret = aci_l2cap_connection_parameter_update_req(
                           ConnectionHandle,
                           6 /* interval_min*/,
                           6 /* interval_max */,
                           0   /* slave_latency */,
                           400 /*timeout_multiplier*/);
                           /* Go to infinite loop if there is one error */
                           if (ret != BLE_STATUS_SUCCESS)
      {
        while (1)
          {
            ATRBLE1_PRINTF("Problem Changing the connection interval\r\n");
          }
        }
      }
#endif

      /* Signal that we are ready sending back the CRV value*/
      BufferToWrite[0] = PointerByte[0];
      BufferToWrite[1] = PointerByte[1];
      BufferToWrite[2] = PointerByte[2];
      BufferToWrite[3] = PointerByte[3];
      BytesToWrite = 4;
      Term_Update(BufferToWrite, BytesToWrite);
    }

    SendBackData = 0;
  }
  else if (!strncmp("uid", (char *)(att_data), 3))
  {
    /* Write back the STM32 UID */
    uint8_t *uid = (uint8_t *)STM32_UUID;
    uint32_t MCU_ID = STM32_MCU_ID[0] & 0xFFF;
    BytesToWrite = sprintf((char *)BufferToWrite, "%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X_%.3lX\n",
                           uid[ 3], uid[ 2], uid[ 1], uid[ 0],
                           uid[ 7], uid[ 6], uid[ 5], uid[ 4],
                           uid[11], uid[ 10], uid[9], uid[8],
                           MCU_ID);
    Term_Update(BufferToWrite, BytesToWrite);
    SendBackData = 0;
  }

  return SendBackData;
}

/**
  * @brief  This function is called when the peer device get disconnected.
  * @param  None
  * @retval None
  */
static void DisconnectionCompletedFunction(void)
{
  connected = FALSE;

  /* Reset for any problem during FOTA update */
  SizeOfUpdateBlueFW = 0;

  /* Reset the BLE Connection Variable */
  ConnectionBleStatus = 0;

  /* Avoid to make the device connectable again. */
  set_connectable = FALSE;

  /* Disable BLE Chip */
  SwitchOffBle = 1;

  if (NeedToRebootBoard)
  {
    NeedToRebootBoard = 0;
    RebootBoard = 1;
  }

  if (NeedToSwapBanks)
  {
    NeedToSwapBanks = 0;
    SwapBanks = 1;
  }
}

/**
  * @brief  This function is called when there is a LE Connection Complete event.
  * @param  None
  * @retval None
  */
static void ConnectionCompletedFunction(uint16_t ConnectionHandle, uint8_t Address_Type, uint8_t Addr[6])
{
  connected = TRUE;
  ConnectionBleStatus = 0;

  BlinkStopPeriodicallyTimer();

  BSP_LED_Off(LED_GREEN);

  HAL_Delay(100);
}


/**
  * @brief  Enable Disable the jump to second flash bank and reboot board
  * @param  None
  * @retval None
  */
void EnableDisableDualBoot(void)
{
  FLASH_OBProgramInitTypeDef    OBInit;
  /* Set BFB2 bit to enable boot from Flash Bank2 */
  /* Allow Access to Flash control registers and user Flash */
  HAL_FLASH_Unlock();

  /* Allow Access to option bytes sector */
  HAL_FLASH_OB_Unlock();

  /* Get the Dual boot configuration status */
  HAL_FLASHEx_OBGetConfig(&OBInit);

  /* Enable/Disable dual boot feature */
  OBInit.OptionType = OPTIONBYTE_USER;
  OBInit.USERType   = OB_USER_SWAP_BANK;

  if (((OBInit.USERConfig) & (FLASH_OPTR_SWAP_BANK)) == FLASH_OPTR_SWAP_BANK)
  {
    OBInit.USERConfig &= ~FLASH_OPTR_SWAP_BANK;
    ATRBLE1_PRINTF("->Disable DualBoot\r\n");
  }
  else
  {
    OBInit.USERConfig = FLASH_OPTR_SWAP_BANK;
    ATRBLE1_PRINTF("->Enable DualBoot\r\n");
  }

  if (HAL_FLASHEx_OBProgram(&OBInit) != HAL_OK)
  {
    /*
    Error occurred while setting option bytes configuration.
    User can add here some code to deal with this error.
    To know the code error, user can call function 'HAL_FLASH_GetError()'
    */
    Error_Handler(ATRBLE1_ERROR_FLASH, __FILE__, __LINE__);
  }

  /* Start the Option Bytes programming process */
  if (HAL_FLASH_OB_Launch() != HAL_OK)
  {
    /*
    Error occurred while reloading option bytes configuration.
    User can add here some code to deal with this error.
    To know the code error, user can call function 'HAL_FLASH_GetError()'
    */
    Error_Handler(ATRBLE1_ERROR_FLASH, __FILE__, __LINE__);
  }
  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();
}


/***********************************************************************************
  * Callback functions to manage the extended configuration characteristic commands *
  ***********************************************************************************/

/**
  * @brief  Callback Function for answering to the UID command
  * @param  uint8_t **UID STM32 UID Return value
  * @retval None
  */
static void ExtExtConfigUidCommandCallback(uint8_t **UID)
{
  *UID = (uint8_t *)STM32_UUID;
}

/**
  * @brief  Callback Function for answering to Info command
  * @param  uint8_t *Answer Return String
  * @retval None
  */
static void ExtConfigInfoCommandCallback(uint8_t *Answer)
{
  sprintf((char *)Answer, "\r\nSTMicroelectronics %s:\n"
          "\tVersion %c.%c.%c\n"
          "\tSTM32U585AI-SensorTile.box-Pro board"
          "\n\t(HAL %ld.%ld.%ld_%ld)\n"
          "\tCompiled %s %s"
#if defined (__IAR_SYSTEMS_ICC__)
          " (IAR)\n"
#elif defined (__CC_ARM)
          " (KEIL)\n"
#elif defined (__GNUC__)
          " (STM32CubeIDE)\n"
#endif
          "\tCurrent Bank =%ld\n",
          ATRBLE1_PACKAGENAME,
          ATRBLE1_VERSION_MAJOR, ATRBLE1_VERSION_MINOR, ATRBLE1_VERSION_PATCH,
          HAL_GetHalVersion() >> 24,
          (HAL_GetHalVersion() >> 16) & 0xFF,
          (HAL_GetHalVersion() >> 8) & 0xFF,
          HAL_GetHalVersion()      & 0xFF,
          __DATE__, __TIME__,
          CurrentActiveBank);
}

/**
  * @brief  Callback Function for answering to SetName command
  * @param  uint8_t *NewName New Name
  * @retval None
  */
static void ExtConfigSetNameCommandCallback(uint8_t *NewName)
{
  ATRBLE1_PRINTF("Received a new Board's Name=%s\r\n", NewName);
  /* Update the Board's name in flash */
  UpdateCurrFlashBankFwIdBoardName(ATRBLE1_BLUEST_SDK_FW_ID, NewName);

  /* Update the Name for BLE Advertise */
  sprintf(BLE_StackValue.BoardName, "%s", NewName);
}

/**
  * @brief  Callback Function for answering to Help command
  * @param  uint8_t *Answer Return String
  * @retval None
  */
static void ExtConfigHelpCommandCallback(uint8_t *Answer)
{
  sprintf((char *)Answer, "Help Message.....");
}

/**
  * @brief  Callback Function for answering to VersionFw command
  * @param  uint8_t *Answer Return String
  * @retval None
  */
static void ExtConfigVersionFwCommandCallback(uint8_t *Answer)
{
  sprintf((char *)Answer, "%s_%s_%c.%c.%c\r\n",
          "U585",
          ATRBLE1_PACKAGENAME,
          ATRBLE1_VERSION_MAJOR,
          ATRBLE1_VERSION_MINOR,
          ATRBLE1_VERSION_PATCH);
}

/**
  * @brief  Callback Function for answering to ReadBanksFwId command
  * @param  uint8_t *CurBank Number Current Bank
  * @param  uint16_t *FwId1 Bank1 Firmware Id
  * @param  uint16_t *FwId2 Bank2 Firmware Id
  * @retval None
  */
static void ExtConfigReadBanksFwIdCommandCallback(uint8_t *CurBank, uint16_t *FwId1, uint16_t *FwId2)
{
  ReadFlashBanksFwId(FwId1, FwId2);
  *CurBank = CurrentActiveBank;
}

/**
  * @brief  Callback Function for answering to BanksSwap command
  * @param  None
  * @retval None
  */
static void ExtConfigBanksSwapCommandCallback(void)
{
  uint16_t FwId1, FwId2;

  ReadFlashBanksFwId(&FwId1, &FwId2);
  if (FwId2 != OTA_OTA_FW_ID_NOT_VALID)
  {
    ATRBLE1_PRINTF("Swapping to Bank%d\n", (CurrentActiveBank == 1) ? 0 : 1);
    ATRBLE1_PRINTF("%s will restart after the disconnection\r\n", ATRBLE1_PACKAGENAME);
    NeedToSwapBanks = 1;
  }
  else
  {
    ATRBLE1_PRINTF("Not Valid fw on Bank%d\n\tCommand Rejected\n", (CurrentActiveBank == 1) ? 0 : 1);
    ATRBLE1_PRINTF("\tLoad a Firmware on Bank%d\n", (CurrentActiveBank == 1) ? 0 : 1);
  }
}

