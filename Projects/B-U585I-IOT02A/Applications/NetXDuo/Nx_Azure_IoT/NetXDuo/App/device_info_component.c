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
/* The following source code has been taken from 'sample_pnp_deviceinfo_component.c'    */
/* by Microsoft (https://github.com/azure-rtos) and modified by STMicroelectronics.     */
/* Main changes summary :                                                               */
/* - Adaptation to STMicroelectronics B-U585I-IoT02A Discovery board PnP Model          */
/*                                                                                      */
/****************************************************************************************/

#include "device_info_component.h"

#include "nx_azure_iot_hub_client_properties.h"
#include "dm_http_status_code.h"
#include "app_azure_iot_version.h"

#define DOUBLE_DECIMAL_PLACE_DIGITS                                     (2)

/* Reported property keys and values.  */
static const CHAR software_version_property_name[] = "swVersion";
static CHAR software_version_property_buffer[64];
static const CHAR manufacturer_property_name[] = "manufacturer";
static const CHAR manufacturer_property_value[] = "STMicroelectronics";
static const CHAR model_property_name[] = "model";
static const CHAR model_property_value[] = "B-U585I-IOT02A";
static const CHAR os_name_property_name[] = "osName";
static const CHAR os_name_property_value[] = "AzureRTOS";
static const CHAR processor_architecture_property_name[] = "processorArchitecture";
static const CHAR processor_architecture_property_value[] = "STM32 ARM Cortex-M33";
static const CHAR processor_manufacturer_property_name[] = "processorManufacturer";
static const CHAR processor_manufacturer_property_value[] = "STMicroelectronics";
static const CHAR total_storage_property_name[] = "totalStorage";
static const double total_storage_property_value = 2000000.0;
static const CHAR total_memory_property_name[] = "totalMemory";
static const double total_memory_property_value = 786000;

static UINT append_properties(NX_AZURE_IOT_JSON_WRITER *json_writer)
{
UINT status;

    snprintf(software_version_property_buffer, 
             sizeof(software_version_property_buffer), 
             "%s %d.%d.%d %s",
             FW_VERSION_NAME, 
             FW_VERSION_MAJOR, 
             FW_VERSION_MINOR, 
             FW_VERSION_PATCH,
             APP_VERSION_NAME
             );

    if (nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
                                                                   (UCHAR *)manufacturer_property_name,
                                                                   sizeof(manufacturer_property_name) - 1,
                                                                   (UCHAR *)manufacturer_property_value,
                                                                   sizeof(manufacturer_property_value) - 1) ||
        nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
                                                                   (UCHAR *)model_property_name,
                                                                   sizeof(model_property_name) - 1,
                                                                   (UCHAR *)model_property_value,
                                                                   sizeof(model_property_value) - 1) ||
        nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
                                                                   (UCHAR *)software_version_property_name,
                                                                   sizeof(software_version_property_name) - 1,
                                                                   (UCHAR *)software_version_property_buffer,
                                                                   strlen(software_version_property_buffer)) ||
        nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
                                                                   (UCHAR *)os_name_property_name,
                                                                   sizeof(os_name_property_name) - 1,
                                                                   (UCHAR *)os_name_property_value,
                                                                   sizeof(os_name_property_value) - 1) ||
        nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
                                                                   (UCHAR *)processor_architecture_property_name,
                                                                   sizeof(processor_architecture_property_name) - 1,
                                                                   (UCHAR *)processor_architecture_property_value,
                                                                   sizeof(processor_architecture_property_value) - 1) ||
        nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
                                                                   (UCHAR *)processor_manufacturer_property_name,
                                                                   sizeof(processor_manufacturer_property_name) - 1,
                                                                   (UCHAR *)processor_manufacturer_property_value,
                                                                   sizeof(processor_manufacturer_property_value) - 1) ||
        nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
                                                                   (UCHAR *)total_storage_property_name,
                                                                   sizeof(total_storage_property_name) - 1,
                                                                   total_storage_property_value,
                                                                   DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
                                                                   (UCHAR *)total_memory_property_name,
                                                                   sizeof(total_memory_property_name) - 1,
                                                                   total_memory_property_value, DOUBLE_DECIMAL_PLACE_DIGITS))
    {
        status = NX_NOT_SUCCESSFUL;
    }
    else
    {
        status = NX_AZURE_IOT_SUCCESS;
    }

    return(status);
}

UINT device_info_component_pnp_report_all_properties(DEVICE_INFO_COMPONENT *handle,
                                                 NX_AZURE_IOT_HUB_CLIENT *iotpnp_client_ptr)
{
    UINT status;
    UINT response_status = 0;
    NX_PACKET *packet_ptr;
    NX_AZURE_IOT_JSON_WRITER json_writer;

    if (handle == NX_NULL)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    if ((status = nx_azure_iot_hub_client_reported_properties_create(iotpnp_client_ptr,
                                                                     &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Failed create reported properties: error code = 0x%08x\r\n", status);
        return(status);
    }

    if ((status = nx_azure_iot_json_writer_init(&json_writer, packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Failed init json writer: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return(status);
    }

    if ((status = nx_azure_iot_json_writer_append_begin_object(&json_writer)) ||
        (status = nx_azure_iot_hub_client_reported_properties_component_begin(iotpnp_client_ptr,
                                                                              &json_writer,
                                                                              handle->component_name_ptr,
                                                                              handle->component_name_length)) ||
        (status = append_properties(&json_writer)) ||
        (status = nx_azure_iot_hub_client_reported_properties_component_end(iotpnp_client_ptr,
                                                                            &json_writer)) ||
        (status = nx_azure_iot_json_writer_append_end_object(&json_writer)))
    {
        printf("Failed to build reported property!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return(status);
    }

    if ((status = nx_azure_iot_hub_client_reported_properties_send(iotpnp_client_ptr,
                                                                   packet_ptr,
                                                                   NX_NULL, &response_status,
                                                                   NX_NULL,
                                                                   (5 * NX_IP_PERIODIC_RATE))))
    {
        printf("Reported properties send failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return(status);
    }

    if (DM_HTTP_CODE_IS_ERROR(response_status))
    {
        printf("Reported properties send failed with code : %d\r\n", response_status);
        return(NX_NOT_SUCCESSFUL);
    }

    return(status);
}

UINT device_info_component_init(DEVICE_INFO_COMPONENT *handle,
                        UCHAR *component_name_ptr, UINT component_name_length)
{
  if (handle == NX_NULL)
  {
      return(NX_NOT_SUCCESSFUL);
  }
  
  handle -> component_name_ptr = component_name_ptr;
  handle -> component_name_length = component_name_length;

  return(NX_AZURE_IOT_SUCCESS);
}
