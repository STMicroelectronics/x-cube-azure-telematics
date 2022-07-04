/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/****************************************************************************************/
/*                                                                                      */
/* The following source code has been taken from 'sample_pnp_temperature_controller.c'  */
/* by Microsoft (https://github.com/azure-rtos) and modified by STMicroelectronics.     */
/* Main changes summary :                                                               */
/* - Adaptation to STMicroelectronics B-U585I-IoT02A Discovery board PnP Model          */
/*                                                                                      */
/*   1 - Modified to manage the telemetry:                                              */
/*      .temperature                                                                    */
/*      .humidity                                                                       */
/*      .pressure                                                                       */
/*      .acceleration                                                                   */
/*      .gyroscope                                                                      */
/*      .magnetometer                                                                   */
/*      .button_counter                                                                 */
/*                                                                                      */
/*   2 - Modified to manage the property:                                               */
/*      .acc_fullscale                                                                  */
/*      .gyro_fullscale                                                                 */
/*      .telemetry_interval                                                             */
/*      .control_LED                                                                    */
/*                                                                                      */
/*   3 - Modified to manage the commands:                                               */
/*      .Play                                                                           */
/*      .Pause                                                                          */
/*      .Reset                                                                          */
/****************************************************************************************/

#ifndef APP_AZURE_IOT_H
#define APP_AZURE_IOT_H

#include "nx_api.h"  
#include "nxd_dns.h"  

VOID app_azure_iot_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time));

VOID app_azure_iot_on_user_button_pushed(VOID);
#endif /* APP_AZURE_IOT_H */