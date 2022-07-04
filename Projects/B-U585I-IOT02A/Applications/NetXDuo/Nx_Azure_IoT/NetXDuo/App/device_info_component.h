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
/* The following source code has been taken from 'sample_pnp_deviceinfo_component.h'    */
/* by Microsoft (https://github.com/azure-rtos) and modified by STMicroelectronics.     */
/* Main changes summary :                                                               */
/* - Adaptation to STMicroelectronics B-U585I-IoT02A Discovery board PnP Model          */
/*                                                                                      */
/****************************************************************************************/

#ifndef DEVICEINFO_COMPONENT_H
#define DEVICEINFO_COMPONENT_H

#ifdef __cplusplus
extern   "C" {
#endif

#include "nx_azure_iot_hub_client.h"
#include "nx_api.h"

typedef struct DEVICE_INFO_COMPONENT_TAG
{
    /* Name of this component */
    UCHAR *component_name_ptr;

    /* Name length of this component */
    UINT component_name_length;
  
} DEVICE_INFO_COMPONENT;

UINT device_info_component_init(DEVICE_INFO_COMPONENT *handle,
                       UCHAR *component_name_ptr, UINT component_name_length);

UINT device_info_component_pnp_report_all_properties(DEVICE_INFO_COMPONENT *handle,
                                                 NX_AZURE_IOT_HUB_CLIENT *iotpnp_client_ptr);

#ifdef __cplusplus
}
#endif
#endif /* DEVICEINFO_COMPONENT_H */
