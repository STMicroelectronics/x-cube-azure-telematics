/**
  ******************************************************************************
  * @file    mx_wifi_conf.h
  * @author  GPM Application Team
  * @brief   Header for mx_wifi module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the ST_LICENSE file
  * in the root directory of this software component.
  * If no ST_LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MX_WIFI_CONF_H
#define MX_WIFI_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported macros -----------------------------------------------------------*/
#define MX_WIFI_USE_SPI                                                     (1)
#define MX_WIFI_NETWORK_BYPASS_MODE                                         (1)
#define MX_WIFI_TX_BUFFER_NO_COPY                                           (1)
#define MX_WIFI_USE_CMSIS_OS                                                (0)
#define DMA_ON_USE                                                          (1)

#if (MX_WIFI_NETWORK_BYPASS_MODE != 1)
#error The NetX driver requires bypass mode
#endif

#if (MX_WIFI_USE_CMSIS_OS != 0)
#error The NetX driver is not supporter when CMSIS OS is defined
#endif

  /* Includes ------------------------------------------------------------------*/
#include "app_azure_iot_config.h"
#include "mx_wifi_azure_rtos_conf.h"
#include "mx_wifi_conf_template.h"

#ifdef __cplusplus
}
#endif

#endif /* MX_WIFI_CONF_H */
