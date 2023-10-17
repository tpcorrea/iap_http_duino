/**
  ******************************************************************************
  * @file    iap_http_duino/incluide/iapconfig_default.h
  * @author  MCD Application Team, modified by Tomas P. Correa
  * @brief   This file contains the address configurations for the In-Application 
  * Programming using a HTTP server 
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

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Flash user area definition *************************************************/   
/* 
   IMPORTANT NOTE:
   ==============
   This program uses a temporary flash range for storing the incoming program before
   replacing the current program at the user memory. Of course, this is suboptimal, 
   since we limit the maximum program size to half the available memory.
   
   The sectors and memory addresses must be in accordance to what was defined in the 
   linkers file ("iapscript.ld") and the exact STM32 device. 

   For example, here we use sector 5 (128kB long) for storing the incoming data and,
   after total reception, the firmware will stop all interrupts and copy this to the
   real user region, from Sector 1 to Sector 4. Sector 0 is assumed to hold the isr_vector
   and the iap functions that run while the programming is happening.
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

#endif /* __IAPCONFIG_DEFAULT_H */
