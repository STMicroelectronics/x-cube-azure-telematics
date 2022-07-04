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
/* The following source code has been taken from 'sample_pnp_thermostat_component.c'    */
/* by Microsoft (https://github.com/azure-rtos) and modified by STMicroelectronics.     */
/* Main changes summary :                                                               */
/* - Adaptation to STMicroelectronics B-U585I-IoT02A Discovery board PnP Model          */
/* - Modified to manage the commands:                                                   */
/*      .Play                                                                           */
/*      .Pause                                                                          */
/*      .Reset                                                                          */
/*                                                                                      */
/****************************************************************************************/

#include "ctrl_component.h"
#include "main.h"

#include "nx_azure_iot_hub_client_properties.h"
#include "dm_http_status_code.h"

/* Pnp command supported */
static const CHAR playCommand[]  = "Play";
static const CHAR pauseCommand[] = "Pause";
static const CHAR resetCommand[] = "Reset";
 
/* Names of properties for desired/reporting */
static const CHAR reported_result_name[] = "Result";

static const CHAR play_response_report[]  = "Play command executed.";
static const CHAR pause_response_report[] = "Pause command executed.";
static const CHAR reset_response_report[] = "Reset will be executed between 5 sec.";

static UINT ctrl_component_pnp_reset_command(CTRL_COMPONENT *handle,
                                              NX_AZURE_IOT_JSON_READER *json_reader_ptr,
                                              NX_AZURE_IOT_JSON_WRITER *out_json_builder_ptr)
{
  NX_PARAMETER_NOT_USED(json_reader_ptr);
  UINT status;

  if(NULL == handle)
  {
      status = NX_NOT_SUCCESSFUL;
  }
  else
  {
    /* Build the method response payload */
    if (nx_azure_iot_json_writer_append_begin_object(out_json_builder_ptr) ||
        nx_azure_iot_json_writer_append_property_with_string_value(out_json_builder_ptr,
                                                                    (UCHAR *)reported_result_name,
                                                                    sizeof(reported_result_name) - 1,
                                                                    (UCHAR *)reset_response_report,
                                                                    sizeof(reset_response_report) - 1) ||
        nx_azure_iot_json_writer_append_end_object(out_json_builder_ptr))
    {
        status = NX_NOT_SUCCESSFUL;
    }
    else
    {
        if(NULL != handle->cmd_callback)
        {
            handle->cmd_callback(CTRL_COMPONENT_CMD_RESET, handle->cmd_callback_context);
        }
        status = NX_AZURE_IOT_SUCCESS;
    }
  }

  return status;
}

static UINT ctrl_component_pnp_play_command(CTRL_COMPONENT *handle,
                                             NX_AZURE_IOT_JSON_READER *json_reader_ptr,
                                             NX_AZURE_IOT_JSON_WRITER *out_json_builder_ptr)
{
  UINT status;
  
  NX_PARAMETER_NOT_USED(json_reader_ptr);
  
  if(NULL == handle)
  {
      status = NX_NOT_SUCCESSFUL;
  }
  else
  {
    /* Build the method response payload */
    if (nx_azure_iot_json_writer_append_begin_object(out_json_builder_ptr) ||
        nx_azure_iot_json_writer_append_property_with_string_value(out_json_builder_ptr,
                                                                    (UCHAR *)reported_result_name,
                                                                    sizeof(reported_result_name) - 1,
                                                                    (UCHAR *)play_response_report,
                                                                    sizeof(play_response_report) - 1) ||
        nx_azure_iot_json_writer_append_end_object(out_json_builder_ptr))
    {
        status = NX_NOT_SUCCESSFUL;
    }
    else
    {
        if(NULL != handle->cmd_callback)
        {
            handle->cmd_callback(CTRL_COMPONENT_CMD_PLAY, handle->cmd_callback_context);
        }
        status = NX_AZURE_IOT_SUCCESS;
    }
  }
  
  return(status);
}

static UINT ctrl_component_pnp_pause_command(CTRL_COMPONENT *handle,
                                              NX_AZURE_IOT_JSON_READER *json_reader_ptr,
                                              NX_AZURE_IOT_JSON_WRITER *out_json_builder_ptr)
{
  UINT status;
  
  NX_PARAMETER_NOT_USED(json_reader_ptr);
  
  if(NULL == handle)
  {
      status = NX_NOT_SUCCESSFUL;
  }
  else
  {
    /* Build the method response payload */
    if (nx_azure_iot_json_writer_append_begin_object(out_json_builder_ptr) ||
        nx_azure_iot_json_writer_append_property_with_string_value(out_json_builder_ptr,
                                                                    (UCHAR *)reported_result_name,
                                                                    sizeof(reported_result_name) - 1,
                                                                    (UCHAR *)pause_response_report,
                                                                    sizeof(pause_response_report) - 1) ||
        nx_azure_iot_json_writer_append_end_object(out_json_builder_ptr))
    {
        status = NX_NOT_SUCCESSFUL;
    }
    else
    {
        if(NULL != handle->cmd_callback)
        {
            handle->cmd_callback(CTRL_COMPONENT_CMD_PAUSE, handle->cmd_callback_context);
        }
        status = NX_AZURE_IOT_SUCCESS;
    }
  }
  
  return(status);
}

UINT ctrl_component_pnp_process_command(CTRL_COMPONENT *handle,
                                         const UCHAR *component_name_ptr, UINT component_name_length,
                                         const UCHAR *pnp_command_name_ptr, UINT pnp_command_name_length,
                                         NX_AZURE_IOT_JSON_READER *json_reader_ptr,
                                         NX_AZURE_IOT_JSON_WRITER *json_response_ptr, UINT *status_code)
{
  UINT dm_status;

  if (handle == NX_NULL)
  {
      return(NX_NOT_SUCCESSFUL);
  }

  if (handle -> component_name_length != component_name_length ||
      strncmp((CHAR *)handle -> component_name_ptr, (CHAR *)component_name_ptr, component_name_length) != 0)
  {
      return(NX_NOT_SUCCESSFUL);
  }
  
  if ( (pnp_command_name_length == (sizeof(resetCommand) - 1)) &&
       (strncmp((CHAR *)pnp_command_name_ptr, (CHAR *)resetCommand, pnp_command_name_length) == 0))
  {
      if (ctrl_component_pnp_reset_command(handle, json_reader_ptr, json_response_ptr))
      {
          dm_status = DM_HTTP_INTERNAL_SERVER_ERROR;
      }
      else
      {
          dm_status = DM_HTTP_SUCCESS;
      }
  }
  
  else if (pnp_command_name_length == (sizeof(playCommand) - 1) &&
      strncmp((CHAR *)pnp_command_name_ptr, (CHAR *)playCommand, pnp_command_name_length) == 0)
  {
      if (ctrl_component_pnp_play_command(handle, json_reader_ptr, json_response_ptr))
      {
          dm_status = DM_HTTP_INTERNAL_SERVER_ERROR;
      }
      else
      {
          dm_status = DM_HTTP_SUCCESS;
      }
  }

  else if (pnp_command_name_length == (sizeof(pauseCommand) - 1) &&
      strncmp((CHAR *)pnp_command_name_ptr, (CHAR *)pauseCommand, pnp_command_name_length) == 0)
  {
      if (ctrl_component_pnp_pause_command(handle, json_reader_ptr, json_response_ptr))
      {
          dm_status = DM_HTTP_INTERNAL_SERVER_ERROR;
      }
      else
      {
          dm_status = DM_HTTP_SUCCESS;
      }
  }
  
  else
  {
      printf("PnP command=%.*s is not supported on CtrlComp component\r\n", pnp_command_name_length, pnp_command_name_ptr);
      dm_status = DM_HTTP_INTERNAL_SERVER_ERROR;
  }

  *status_code = dm_status;

  return(NX_AZURE_IOT_SUCCESS);
}

UINT ctrl_component_init(CTRL_COMPONENT *handle,
                        UCHAR *component_name_ptr, UINT component_name_length,
                        VOID (*ctrl_component_cmd_cbk)(CTRL_COMPONENT_CMD event, VOID *context), VOID *context)
{
  if (handle == NX_NULL)
  {
      return(NX_NOT_SUCCESSFUL);
  }
  
  handle -> component_name_ptr = component_name_ptr;
  handle -> component_name_length = component_name_length;
  handle -> cmd_callback = ctrl_component_cmd_cbk;
  handle -> cmd_callback_context = context;

  return(NX_AZURE_IOT_SUCCESS);
}


