/**
  ******************************************************************************
  * @file    LwIP/LwIP_IAP/Inc/flash_if.h 
  * @author  MCD Application Team, modified by Tomas P. Correa
  * @brief   Header for flash_if.c module
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
#ifndef __FLASH_IF_H
#define __FLASH_IF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define __IAP_FUNC __attribute__((section(".iap_flash")))
// #define __IAP_FUNC  __attribute__( ( long_call, section(".iap_flash") ) )
/* Exported functions ------------------------------------------------------- */
uint32_t FLASH_If_Write(__IO uint32_t* Address, uint32_t const * Data, uint16_t DataLength);
int8_t FLASH_If_Erase(uint32_t initialSector, uint32_t nbSectors);
void FLASH_If_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_IF_H */
