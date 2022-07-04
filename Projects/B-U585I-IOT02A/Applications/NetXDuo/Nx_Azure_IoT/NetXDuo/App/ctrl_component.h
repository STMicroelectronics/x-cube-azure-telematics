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
/* The following source code has been taken from 'sample_pnp_thermostat_component.h'    */
/* by Microsoft (https://github.com/azure-rtos) and modified by STMicroelectronics.     */
/* Main changes summary :                                                               */
/* - Adaptation to STMicroelectronics B-U585I-IoT02A Discovery board PnP Model          */
/* - Modified to manage the commands:                                                   */
/*      .Play                                                                             */
/*      .Pause                                                                            */
/*      .Reset                                                                            */
/*                                                                                      */
/****************************************************************************************/

#ifndef CTRL_COMPONENT_H
#define CTRL_COMPONENT_H

#ifdef __cplusplus
extern   "C" {
#endif

#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_json_reader.h"
#include "nx_azure_iot_json_writer.h"
#include "nx_api.h"
  
typedef enum  {
    CTRL_COMPONENT_CMD_PAUSE = 0,
    CTRL_COMPONENT_CMD_PLAY,
    CTRL_COMPONENT_CMD_RESET
} CTRL_COMPONENT_CMD;

typedef struct CTRL_COMPONENT_TAG
{
    /* Name of this component */
    UCHAR *component_name_ptr;

    /* Name length of this component */
    UINT component_name_length;

    /* User command callback */
    VOID(*cmd_callback)(CTRL_COMPONENT_CMD event, VOID *context);
    
    /* User command callback context */
    VOID *cmd_callback_context;
} CTRL_COMPONENT;


UINT ctrl_component_pnp_process_command(CTRL_COMPONENT *handle,
                                         const UCHAR *component_name_ptr, UINT component_name_length,
                                         const UCHAR *pnp_command_name_ptr, UINT pnp_command_name_length,
                                         NX_AZURE_IOT_JSON_READER *json_reader_ptr,
                                         NX_AZURE_IOT_JSON_WRITER *json_response_ptr, UINT *status_code);
                               
UINT ctrl_component_init(CTRL_COMPONENT *handle,
                        UCHAR *component_name_ptr, UINT component_name_length,
                        VOID (*ctrl_component_cmd_cbk)(CTRL_COMPONENT_CMD event, VOID *context), VOID *context);
                              
#ifdef __cplusplus
}
#endif
#endif /* CTRL_COMPONENT_H */
