/**
  ******************************************************************************
  * @file    LwIP/LwIP_IAP/Inc/main.h
  * @author  MCD Application Team, modified by Tomas P. Correa
  * @brief   This file contains all the functions prototypes for the main.c 
  *          file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IAPCONFIG_DEFAULT_H
#define __IAPCONFIG_DEFAULT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* IAP options selection ******************************************************/
#define USE_IAP_HTTP   /* enable IAP using HTTP */

/* Flash user area definition *************************************************/   
/* 
   IMPORTANT NOTE:
   ==============
   This program uses a temporary flash range for storing the incoming program before
   replacing the current program at the user memory.
   
   The sectors and memory addresses must be in accordance to what was defined in the 
   linkers file ("iapscript.ld") and the exact STM32 device. 

   For example, here we use sector 5 (128kB long) for storing the incoming data and,
   after total reception, the firmware will stop all interrupts and copy this to the
   real user region, from Sector 1 to Sector 4.
   */

#define USER_PROG_BEGIN_ADDRESS 0x8004000  
#define USER_PROG_END_ADDRESS  0x801FFFF
#define USER_PROG_BEGIN_SECTOR FLASH_SECTOR_1
#define USER_PROG_END_SECTOR FLASH_SECTOR_4

#define TEMP_PROG_BEGIN_ADDRESS 0x8020000  
#define TEMP_PROG_END_ADDRESS  0x803FFFF
#define TEMP_PROG_BEGIN_SECTOR FLASH_SECTOR_5
#define TEMP_PROG_END_SECTOR FLASH_SECTOR_5

#define IAP_SECTOR_SIZE 0x4000
   
/* UserID and Password definition *********************************************/
#define USERID       "user"
#define PASSWORD     "stm32"
#define LOGIN_SIZE   (17+ sizeof(USERID) + sizeof(PASSWORD))
 
/* Static IP Address definition ***********************************************/
#define IP_ADDR0   (uint8_t) 192
#define IP_ADDR1   (uint8_t) 168
#define IP_ADDR2   (uint8_t) 0
#define IP_ADDR3   (uint8_t) 10

/* NETMASK definition *********************************************************/
#define NETMASK_ADDR0   (uint8_t) 255
#define NETMASK_ADDR1   (uint8_t) 255
#define NETMASK_ADDR2   (uint8_t) 255
#define NETMASK_ADDR3   (uint8_t) 0

/* Gateway Address definition *************************************************/
#define GW_ADDR0   (uint8_t) 192
#define GW_ADDR1   (uint8_t) 168
#define GW_ADDR2   (uint8_t) 0
#define GW_ADDR3   (uint8_t) 1

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */  
/* Exported function prototypes ----------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
