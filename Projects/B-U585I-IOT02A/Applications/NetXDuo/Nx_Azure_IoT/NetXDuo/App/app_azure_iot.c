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

#include <stdio.h>

#include "stm32u5xx.h"
#include "b_u585i_iot02a.h"

#include "nx_api.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_hub_client_properties.h"
#include "nx_azure_iot_provisioning_client.h"

/* These are sample files, user can build their own certificate and ciphersuites.  */
#include "nx_azure_iot_cert.h"
#include "nx_azure_iot_ciphersuites.h"

#include "app_azure_iot_config.h"
#include "device_info_component.h"
#include "std_component.h"
#include "ctrl_component.h"
#include "app_azure_iot.h"
#include "dm_http_status_code.h"

#if (USE_DEVICE_CERTIFICATE == 1)
#include "device_identity.h"
#endif

/* Define events.  */
#define APP_AZURE_IOT_ALL_EVENTS                                               ((ULONG)0xFFFFFFFF)
#define APP_AZURE_IOT_CONNECTED_EVENT                                          ((ULONG)0x00000001)
#define APP_AZURE_IOT_DISCONNECT_EVENT                                         ((ULONG)0x00000002)
#define APP_AZURE_IOT_PERIODIC_EVENT                                           ((ULONG)0x00000004)
#define APP_AZURE_IOT_TELEMETRY_SEND_EVENT                                     ((ULONG)0x00000008)
#define APP_AZURE_IOT_COMMAND_RECEIVE_EVENT                                    ((ULONG)0x00000010)
#define APP_AZURE_IOT_PROPERTIES_RECEIVE_EVENT                                 ((ULONG)0x00000020)
#define APP_AZURE_IOT_WRITABLE_PROPERTIES_RECEIVE_EVENT                        ((ULONG)0x00000040)
#define APP_AZURE_IOT_REPORTED_PROPERTIES_SEND_EVENT                           ((ULONG)0x00000080)
#define APP_AZURE_IOT_RESET_EVENT                                              ((ULONG)0x00000100)

#define APP_AZURE_IOT_PNP_DPS_PAYLOAD                                          "{\"modelId\":\"" NX_AZURE_IOT_PNP_MODEL_ID "\"}"

/* Generally, IoTHub Client and DPS Client do not run at the same time, user can use union as below to
   share the memory between IoTHub Client and DPS Client.

   NOTE: If user can not make sure sharing memory is safe, IoTHub Client and DPS Client must be defined separately.  */
typedef union APP_AZURE_IOT_CLIENT_UNION
{
    NX_AZURE_IOT_HUB_CLIENT                         iothub_client;

#ifdef ENABLE_DPS
    NX_AZURE_IOT_PROVISIONING_CLIENT                prov_client;
#endif /* ENABLE_DPS */

} APP_AZURE_IOT_CLIENT;

typedef struct CTRL_COMPONENT_MONITOR_TAG
{
    INT ReceivedPauseCommand;
    INT ReceivedPlayCommand;
    INT ReceivedResetCommand;
    INT ResetTime;

} CTRL_COMPONENT_MONITOR;

typedef struct STD_COMPONENT_PROPERTY_SENT_STATUS_TAG
{
    INT SentIntervalSec;
    INT SentAccFullScale;
    INT SentGyroFullScale;
    INT SentLED_ControlStatus;
} STD_COMPONENT_PROPERTY_SENT_STATUS;

typedef struct DEVICE_INFO_COMPONENT_PROPERTY_SENT_STATUS_TAG
{
    INT SentAllProperties;
} DEVICE_INFO_COMPONENT_PROPERTY_SENT_STATUS;

static APP_AZURE_IOT_CLIENT                                client;

#define iothub_client client.iothub_client
#ifdef ENABLE_DPS
#define prov_client client.prov_client
#endif /* ENABLE_DPS */

#ifdef ENABLE_DPS
static UINT dps_entry(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                             UCHAR **iothub_hostname, UINT *iothub_hostname_length,
                             UCHAR **iothub_device_id, UINT *iothub_device_id_length);
#endif /* ENABLE_DPS */

/* Define Azure RTOS TLS info.  */
static NX_SECURE_X509_CERT root_ca_cert;
static NX_SECURE_X509_CERT root_ca_cert_2;
static NX_SECURE_X509_CERT root_ca_cert_3;
static UCHAR nx_azure_iot_tls_metadata_buffer[NX_AZURE_IOT_TLS_METADATA_BUFFER_SIZE];
static ULONG nx_azure_iot_thread_stack[NX_AZURE_IOT_STACK_SIZE / sizeof(ULONG)];

/* Using X509 certificate authenticate to connect to IoT Hub,
   set the device certificate as your device.  */
#if (USE_DEVICE_CERTIFICATE == 1)
static const UCHAR* device_cert_ptr;
UINT device_cert_len;
static const UCHAR* device_private_key_ptr;
UINT device_private_key_len;
NX_SECURE_X509_CERT device_certificate;
#endif /* USE_DEVICE_CERTIFICATE */

/* Define buffer for IoTHub info.  */
#ifdef ENABLE_DPS
static UCHAR iothub_hostname_buffer[NX_AZURE_IOT_MAX_BUFFER];
static UCHAR iothub_device_id_buffer[NX_AZURE_IOT_MAX_BUFFER];
#endif /* ENABLE_DPS */

/* Define the prototypes for AZ IoT.  */
static NX_AZURE_IOT nx_azure_iot;

static TX_EVENT_FLAGS_GROUP tx_events;
static TX_TIMER tx_timer;
static volatile UINT connection_status = NX_AZURE_IOT_NOT_INITIALIZED;
static volatile ULONG last_periodic_send_action_tick = 0;
static volatile ULONG last_periodic_action_tick = 0;

static UCHAR scratch_buffer[256];


/* Standard Component Handle */
STD_COMPONENT std_comp;
STD_COMPONENT_PROPERTY_SENT_STATUS std_comp_property_sent_status = {
    .SentIntervalSec = 0,
    .SentAccFullScale = 0,
    .SentGyroFullScale = 0,
    .SentLED_ControlStatus = 0
};
static const CHAR std_component_name[] = "std_comp";

/* Control Component Handle */
CTRL_COMPONENT ctrl_comp;
CTRL_COMPONENT_MONITOR ctrl_component_monitor = {
    .ReceivedPauseCommand = 0,
    .ReceivedPlayCommand = 0,
    .ReceivedResetCommand = 0,
    .ResetTime = 0
};
static const CHAR ctrl_component_name[] = "ctrl_comp";

/* Device info component Handle */
DEVICE_INFO_COMPONENT device_info_component;
DEVICE_INFO_COMPONENT_PROPERTY_SENT_STATUS device_info_component_property_sent_status = {
    .SentAllProperties = 0
};
static const CHAR device_info_component_name[] = "deviceinfo";

/* User push button callback*/
VOID app_azure_iot_on_user_button_pushed(VOID) {
    std_component_on_button_pushed(&std_comp);
}

/* Include the sample connection monitor function from sample_azure_iot_embedded_sdk_connect.c.  */
extern VOID sample_connection_monitor(NX_IP *ip_ptr, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr, UINT connection_status,
                                      UINT (*iothub_init)(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr));

/* Control compoonent command callback */
VOID ctrl_component_cmd_cbk(CTRL_COMPONENT_CMD event, VOID *context) {
    if(NULL != context) 
    {
        switch(event) 
        {
            case CTRL_COMPONENT_CMD_PAUSE: 
                ((CTRL_COMPONENT_MONITOR*)context)->ReceivedPauseCommand = 1;
                break;
            case CTRL_COMPONENT_CMD_PLAY: 
                ((CTRL_COMPONENT_MONITOR*)context)->ReceivedPlayCommand = 1;
                break;
            case CTRL_COMPONENT_CMD_RESET: 
                ((CTRL_COMPONENT_MONITOR*)context)->ReceivedResetCommand = 1;
                ((CTRL_COMPONENT_MONITOR*)context)->ResetTime = tx_time_get();
                break;
            default:
                break;
        }
    }
}

/* Parses writable properties document.  */
static UINT parse_writable_component_property(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                        NX_AZURE_IOT_JSON_READER *json_reader_ptr,
                                                        UINT message_type, ULONG version)
{
UINT status;
const UCHAR *component_name_ptr;
USHORT component_length = 0;

    while ((status = nx_azure_iot_hub_client_properties_component_property_next_get(hub_client_ptr,
                                                                                    json_reader_ptr,
                                                                                    message_type,
                                                                                    NX_AZURE_IOT_HUB_CLIENT_PROPERTY_WRITABLE,
                                                                                    &component_name_ptr, &component_length)) == NX_AZURE_IOT_SUCCESS)
    {
        status = std_component_process_property_update(&std_comp,
                hub_client_ptr,
                (UCHAR *)component_name_ptr, component_length,
                json_reader_ptr, version);
        if (status != NX_AZURE_IOT_SUCCESS)
        {

            /* The JSON reader must be advanced regardless of whether the property
               is of interest or not.  */
            nx_azure_iot_json_reader_next_token(json_reader_ptr);
 
            /* Skip children in case the property value is an object.  */
            if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
            {
                nx_azure_iot_json_reader_skip_children(json_reader_ptr);
            }
            nx_azure_iot_json_reader_next_token(json_reader_ptr);
        }
    }

    return(NX_AZURE_IOT_SUCCESS);
}

static VOID connection_status_callback(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, UINT status)
{
    NX_PARAMETER_NOT_USED(hub_client_ptr);
    if (status)
    {
        printf("Disconnected from IoTHub!: error code = 0x%08x\r\n", status);
        tx_event_flags_set(&tx_events, APP_AZURE_IOT_DISCONNECT_EVENT, TX_OR);
    }
    else
    {
        printf("Connected to IoTHub.\r\n");
        tx_event_flags_set(&tx_events, APP_AZURE_IOT_CONNECTED_EVENT, TX_OR);
    }

    connection_status = status;
}

static VOID message_receive_callback_properties(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, VOID *context)
{

    NX_PARAMETER_NOT_USED(hub_client_ptr);
    NX_PARAMETER_NOT_USED(context);
    tx_event_flags_set(&tx_events, APP_AZURE_IOT_PROPERTIES_RECEIVE_EVENT, TX_OR);
}

static VOID message_receive_callback_command(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, VOID *context)
{
    NX_PARAMETER_NOT_USED(hub_client_ptr);
    NX_PARAMETER_NOT_USED(context);
    tx_event_flags_set(&(tx_events), APP_AZURE_IOT_COMMAND_RECEIVE_EVENT, TX_OR);
}

static VOID message_receive_callback_writable_properties(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, VOID *context)
{
    NX_PARAMETER_NOT_USED(hub_client_ptr);
    NX_PARAMETER_NOT_USED(context);
    tx_event_flags_set(&(tx_events), APP_AZURE_IOT_WRITABLE_PROPERTIES_RECEIVE_EVENT, TX_OR);
}

static UINT initialize_iothub(NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr)
{
UINT status;
#ifdef ENABLE_DPS
UCHAR *iothub_hostname = NX_NULL;
UCHAR *iothub_device_id = NX_NULL;
UINT iothub_hostname_length = 0;
UINT iothub_device_id_length = 0;
#else
UCHAR *iothub_hostname = (UCHAR *)HOST_NAME;
UCHAR *iothub_device_id = (UCHAR *)DEVICE_ID;
UINT iothub_hostname_length = sizeof(HOST_NAME) - 1;
UINT iothub_device_id_length = sizeof(DEVICE_ID) - 1;
#endif /* ENABLE_DPS */

#if (USE_DEVICE_CERTIFICATE == 1) 
    if ((status = device_identity_retrieve_credentials(&device_cert_ptr, &device_cert_len, &device_private_key_ptr, &device_private_key_len)))
    {
        printf("Failed to retrieve device identity: error code = 0x%08x\r\n", status);
        return(status);
    }
    
    /* Initialize the device certificate.  */
    if ((status = nx_secure_x509_certificate_initialize(&device_certificate,
                                                        (UCHAR *)device_cert_ptr, (USHORT)device_cert_len,
                                                        NX_NULL, 0,     
                                                        (UCHAR *)device_private_key_ptr,
                                                        (USHORT)device_private_key_len,
                                                        DEVICE_KEY_TYPE)))
    {
        printf("Failed on nx_secure_x509_certificate_initialize!: error code = 0x%08x\r\n", status);
        return(status);
    }
    
    /* Extract and Print Device registration ID from device certificate. */
    iothub_device_id = (UCHAR*)device_certificate.nx_secure_x509_distinguished_name.nx_secure_x509_common_name;
    iothub_device_id_length = device_certificate.nx_secure_x509_distinguished_name.nx_secure_x509_common_name_length;
    printf("\r\n\r\nRegistration ID: \033[0;32m %.*s \033[0;39;49m\r\n\r\n", iothub_device_id_length, iothub_device_id); 
    
#endif /* USE_DEVICE_CERTIFICATE */

#ifdef ENABLE_DPS

    /* Run DPS.  */
    if ((status = dps_entry(&prov_client, &iothub_hostname, &iothub_hostname_length,
                                   &iothub_device_id, &iothub_device_id_length)))
    {
        printf("Failed on dps_entry!: error code = 0x%08x\r\n", status);
        return(status);
    }
#endif /* ENABLE_DPS */

    printf("IoTHub Host Name: %.*s; \r\nDevice ID: %.*s.\r\n",
           iothub_hostname_length, iothub_hostname, iothub_device_id_length, iothub_device_id);

    /* Initialize IoTHub client.  */
    if ((status = nx_azure_iot_hub_client_initialize(iothub_client_ptr, &nx_azure_iot,
                                                     iothub_hostname, iothub_hostname_length,
                                                     iothub_device_id, iothub_device_id_length,
                                                     (const UCHAR *)MODULE_ID, sizeof(MODULE_ID) - 1,
                                                     _nx_azure_iot_tls_supported_crypto,
                                                     _nx_azure_iot_tls_supported_crypto_size,
                                                     _nx_azure_iot_tls_ciphersuite_map,
                                                     _nx_azure_iot_tls_ciphersuite_map_size,
                                                     nx_azure_iot_tls_metadata_buffer,
                                                     sizeof(nx_azure_iot_tls_metadata_buffer),
                                                     &root_ca_cert)))
    {
        printf("Failed on nx_azure_iot_hub_client_initialize!: error code = 0x%08x\r\n", status);
        return(status);
    }

    /* Set the model id.  */
    if ((status = nx_azure_iot_hub_client_model_id_set(iothub_client_ptr,
                                                       (const UCHAR *)NX_AZURE_IOT_PNP_MODEL_ID,
                                                       sizeof(NX_AZURE_IOT_PNP_MODEL_ID) - 1)))
    {
        printf("Failed on nx_azure_iot_hub_client_model_id_set!: error code = 0x%08x\r\n", status);
    }
    
    /* Add more CA certificates.  */
    else if ((status = nx_azure_iot_hub_client_trusted_cert_add(iothub_client_ptr, &root_ca_cert_2)))
    {
        printf("Failed on nx_azure_iot_hub_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_trusted_cert_add(iothub_client_ptr, &root_ca_cert_3)))
    {
        printf("Failed on nx_azure_iot_hub_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }

#if (USE_DEVICE_CERTIFICATE == 1)

    /* Set device certificate.  */
    else if ((status = nx_azure_iot_hub_client_device_cert_set(iothub_client_ptr, &device_certificate)))
    {
        printf("Failed on nx_azure_iot_hub_client_device_cert_set!: error code = 0x%08x\r\n", status);
    }
#else

    /* Set symmetric key.  */
    else if ((status = nx_azure_iot_hub_client_symmetric_key_set(iothub_client_ptr,
                                                                 (UCHAR *)DEVICE_SYMMETRIC_KEY,
                                                                 sizeof(DEVICE_SYMMETRIC_KEY) - 1)))
    {
        printf("Failed on nx_azure_iot_hub_client_symmetric_key_set!\r\n");
    }
#endif /* USE_DEVICE_CERTIFICATE */

    /* Enable command and properties features.  */
    else if ((status = nx_azure_iot_hub_client_command_enable(iothub_client_ptr)))
    {
        printf("Failed on nx_azure_iot_hub_client_command_enable!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_properties_enable(iothub_client_ptr)))
    {
        printf("Failed on nx_azure_iot_hub_client_properties_enable!: error code = 0x%08x\r\n", status);
    }

    /* Set connection status callback.  */
    else if ((status = nx_azure_iot_hub_client_connection_status_callback_set(iothub_client_ptr,
                                                                              connection_status_callback)))
    {
        printf("Failed on connection_status_callback!\r\n");
    }
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(iothub_client_ptr,
                                                                    NX_AZURE_IOT_HUB_COMMAND,
                                                                    message_receive_callback_command,
                                                                    NX_NULL)))
    {
        printf("device command callback set!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(iothub_client_ptr,
                                                                    NX_AZURE_IOT_HUB_PROPERTIES,
                                                                    message_receive_callback_properties,
                                                                    NX_NULL)))
    {
        printf("device properties callback set!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(iothub_client_ptr,
                                                                    NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES,
                                                                    message_receive_callback_writable_properties,
                                                                    NX_NULL)))
    {
        printf("device writable properties callback set!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_component_add(iothub_client_ptr,
                                                             (const UCHAR *)std_component_name,
                                                             sizeof(std_component_name) - 1)) ||
             (status = nx_azure_iot_hub_client_component_add(iothub_client_ptr,
                                                             (const UCHAR *)ctrl_component_name,
                                                             sizeof(ctrl_component_name) - 1)) ||
             (status = nx_azure_iot_hub_client_component_add(iothub_client_ptr,
                                                             (const UCHAR *)device_info_component_name,
                                                             sizeof(device_info_component_name) - 1)))
    {
        printf("Failed to add component to client!: error code = 0x%08x\r\n", status);
    }

    if (status)
    {
        nx_azure_iot_hub_client_deinitialize(iothub_client_ptr);
    }

    return(status);
}

static void command_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status = 0;
const UCHAR *component_name_ptr;
USHORT component_name_length;
const UCHAR *command_name_ptr;
USHORT command_name_length;
USHORT context_length;
VOID *context_ptr;
UINT dm_status = 404;
UINT response_payload = 0;
NX_PACKET *packet_ptr;
NX_AZURE_IOT_JSON_READER json_reader;
NX_AZURE_IOT_JSON_WRITER json_writer;

    /* Loop to receive command message.  */
    while (1)
    {

        if (connection_status != NX_SUCCESS)
        {
            return;
        }

        if ((status = nx_azure_iot_hub_client_command_message_receive(hub_client_ptr,
                                                                      &component_name_ptr, &component_name_length,
                                                                      &command_name_ptr, &command_name_length,
                                                                      &context_ptr, &context_length,
                                                                      &packet_ptr, NX_NO_WAIT)))
        {
            return;
        }

        printf("Received command: %.*s", (INT)command_name_length, (CHAR *)command_name_ptr);
        printf("\r\n");

        if ((status = nx_azure_iot_json_reader_init(&json_reader,
                                                    packet_ptr)))
        {
            printf("Failed to initialize json reader \r\n");
            nx_packet_release(packet_ptr);
            return;
        }

        if ((status = nx_azure_iot_json_writer_with_buffer_init(&json_writer,
                                                                scratch_buffer,
                                                                sizeof(scratch_buffer))))
        {
            printf("Failed to initialize json writer response \r\n");
            nx_packet_release(packet_ptr);
            return;
        }

        if ((status = ctrl_component_pnp_process_command(&ctrl_comp, component_name_ptr,
                                                          component_name_length, command_name_ptr,
                                                          command_name_length, &json_reader,
                                                          &json_writer, &dm_status)) == NX_AZURE_IOT_SUCCESS)
        {
            printf("Successfully executed command %.*s on std_comp\r\n", component_name_length, component_name_ptr);
            response_payload = nx_azure_iot_json_writer_get_bytes_used(&json_writer);
        }
        else
        {
            printf("Failed to find any handler for method %.*s\r\n", component_name_length, component_name_ptr);
            dm_status = DM_HTTP_BAD_REQUEST;
        }

        nx_packet_release(packet_ptr);

        if ((status = nx_azure_iot_hub_client_command_message_response(hub_client_ptr, dm_status,
                                                                       context_ptr, context_length, scratch_buffer,
                                                                       response_payload, NX_WAIT_FOREVER)))
        {
            printf("Command response failed!: error code = 0x%08x\r\n", status);
        }
    }
}

static void writable_properties_receive_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status = 0;
NX_PACKET *packet_ptr;
NX_AZURE_IOT_JSON_READER json_reader;
ULONG properties_version;

    if (connection_status != NX_SUCCESS)
    {
        return;
    }

    if ((status = nx_azure_iot_hub_client_writable_properties_receive(hub_client_ptr,
                                                                      &packet_ptr,
                                                                      NX_WAIT_FOREVER)))
    {
        printf("Receive writable property receive failed!: error code = 0x%08x\r\n", status);
        return;
    }

    printf("Received writable property");
    printf("\r\n");

    if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
    {
        printf("Init json reader failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    /* Get the version.  */
    if ((status = nx_azure_iot_hub_client_properties_version_get(hub_client_ptr, &json_reader, 
                                                                 NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES,
                                                                 &properties_version)))
    {
        printf("Properties version get failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
    {
        printf("Init json reader failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    status = parse_writable_component_property(hub_client_ptr, &json_reader,
                                                         NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES, properties_version);
    if (status && (status != NX_AZURE_IOT_NOT_FOUND))
    {
        printf("Failed to parse value\r\n");
    }

    nx_packet_release(packet_ptr);
}

static void reported_properties_send_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status = 0;

    if (connection_status != NX_SUCCESS)
    {
        return;
    }

    /* Only report once.  */
    if (std_comp_property_sent_status.SentAccFullScale == 0)
    {
        if ((status = std_component_pnp_report_acc_fullscale_property(&std_comp,
                                                               &(iothub_client))))
        {
            printf("Failed std_component_pnp_report_acc_fullscale_property: error code = 0x%08x\r\n", status);
        }
        else
        {
            std_comp_property_sent_status.SentAccFullScale = 1;
            printf("std_component_pnp_report_acc_fullscale_property: Reported property sent\r\n");
        }
    }

    if (std_comp_property_sent_status.SentGyroFullScale == 0)
    {
        if ((status = std_component_pnp_report_gyro_fullscale_property(&std_comp,
                                                                &(iothub_client))))
        {
            printf("Failed std_component_pnp_report_gyro_fullscale_property: error code = 0x%08x\r\n", status);
        }
        else
        {
            std_comp_property_sent_status.SentGyroFullScale = 1;
            printf("std_component_pnp_report_gyro_fullscale_property: Reported property sent\r\n");
        }
    }

    if (std_comp_property_sent_status.SentIntervalSec == 0)
    {
        if ((status = std_component_pnp_report_telemetry_interval_property(&std_comp,
                                                                    &(iothub_client))))
        {
            printf("Failed std_component_pnp_report_telemetry_interval_property: error code = 0x%08x\r\n", status);
        }
        else
        {
            std_comp_property_sent_status.SentIntervalSec = 1;
            printf("std_component_pnp_report_telemetry_interval_property: Reported property sent\r\n");
        }
    }

    if (std_comp_property_sent_status.SentLED_ControlStatus == 0)
    {
        if ((status = std_component_pnp_report_control_led_property(&std_comp,
                                                             &(iothub_client))))
        {
            printf("Failed std_component_pnp_report_control_led_property: error code = 0x%08x\r\n", status);
        }
        else
        {
            std_comp_property_sent_status.SentLED_ControlStatus = 1;
            printf("std_component_pnp_report_control_led_property: Reported property sent\r\n");
        }
    }

    if (device_info_component_property_sent_status.SentAllProperties == 0)
    {
        if ((status = device_info_component_pnp_report_all_properties(&device_info_component,
                                                                  &(iothub_client))))
        {
            printf("Failed device_info_component_pnp_report_all_properties: error code = 0x%08x\r\n", status);
        }
        else
        {
            device_info_component_property_sent_status.SentAllProperties = 1;
            printf("device_info_component_pnp_report_all_properties: Reported property sent\r\n");
        }
    }
}

static void properties_receive_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status = 0;
NX_PACKET *packet_ptr;
NX_AZURE_IOT_JSON_READER json_reader;
ULONG writable_properties_version;

    if (connection_status != NX_SUCCESS)
    {
        return;
    }

    if ((status = nx_azure_iot_hub_client_properties_receive(hub_client_ptr,
                                                             &packet_ptr,
                                                             NX_WAIT_FOREVER)))
    {
        printf("Get all properties receive failed!: error code = 0x%08x\r\n", status);
        return;
    }

    printf("Received all properties");
    printf("\r\n");

    if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
    {
        printf("Init json reader failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    if ((status = nx_azure_iot_hub_client_properties_version_get(hub_client_ptr, &json_reader,
                                                                 NX_AZURE_IOT_HUB_PROPERTIES,
                                                                 &writable_properties_version)))
    {
        printf("Properties version get failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
    {
        printf("Init json reader failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    status = parse_writable_component_property(hub_client_ptr, &json_reader,
                                                         NX_AZURE_IOT_HUB_PROPERTIES, writable_properties_version);
    if (status && (status != NX_AZURE_IOT_NOT_FOUND))
    {
        printf("Failed to parse value\r\n");
    }

    nx_packet_release(packet_ptr);
}

static void telemetry_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status = 0;

    if (connection_status != NX_SUCCESS)
    {
        return;
    }

    if ((status = std_component_read_sensor_values(&std_comp)) != NX_AZURE_IOT_SUCCESS)
    {
        printf("Failed to read sensor values, error: %d\r\n", status);
    }

    if ((status = std_component_telemetry_send(&std_comp,
                                              hub_client_ptr)) != NX_AZURE_IOT_SUCCESS)
    {
        printf("Failed to send std_component_telemetry_send, error: %d\r\n", status);
    }
}

#ifdef ENABLE_DPS
static UINT dps_entry(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                             UCHAR **iothub_hostname, UINT *iothub_hostname_length,
                             UCHAR **iothub_device_id, UINT *iothub_device_id_length)
{
UINT status;

    printf("Start Provisioning Client...\r\n");

    /* Initialize IoT provisioning client.  */
    if ((status = nx_azure_iot_provisioning_client_initialize(prov_client_ptr, &nx_azure_iot,
                                                              (UCHAR *)ENDPOINT, strlen(ENDPOINT),
                                                              (UCHAR *)ID_SCOPE, strlen(ID_SCOPE),
#if (USE_DEVICE_CERTIFICATE == 1) 
                                                              *iothub_device_id, *iothub_device_id_length,
#else
                                                              (UCHAR *)REGISTRATION_ID, sizeof(REGISTRATION_ID) - 1,
#endif
                                                              _nx_azure_iot_tls_supported_crypto,
                                                              _nx_azure_iot_tls_supported_crypto_size,
                                                              _nx_azure_iot_tls_ciphersuite_map,
                                                              _nx_azure_iot_tls_ciphersuite_map_size,
                                                              nx_azure_iot_tls_metadata_buffer,
                                                              sizeof(nx_azure_iot_tls_metadata_buffer),
                                                              &root_ca_cert)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_initialize!: error code = 0x%08x\r\n", status);
        return(status);
    }

    /* Initialize length of hostname and device ID.  */
    *iothub_hostname_length = sizeof(iothub_hostname_buffer);
    *iothub_device_id_length = sizeof(iothub_device_id_buffer);

    /* Add more CA certificates.  */
    if ((status = nx_azure_iot_provisioning_client_trusted_cert_add(prov_client_ptr, &root_ca_cert_2)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_provisioning_client_trusted_cert_add(prov_client_ptr, &root_ca_cert_3)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }

#if (USE_DEVICE_CERTIFICATE == 1)

    /* Set device certificate.  */
    else if ((status = nx_azure_iot_provisioning_client_device_cert_set(prov_client_ptr, &device_certificate)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_device_cert_set!: error code = 0x%08x\r\n", status);
    }
#else

    /* Set symmetric key.  */
    else if ((status = nx_azure_iot_provisioning_client_symmetric_key_set(prov_client_ptr, (UCHAR *)DEVICE_SYMMETRIC_KEY,
                                                                          sizeof(DEVICE_SYMMETRIC_KEY) - 1)))
    {
        printf("Failed on nx_azure_iot_hub_client_symmetric_key_set!: error code = 0x%08x\r\n", status);
    }
#endif /* USE_DEVICE_CERTIFICATE */
    else if ((status = nx_azure_iot_provisioning_client_registration_payload_set(prov_client_ptr, (UCHAR *)APP_AZURE_IOT_PNP_DPS_PAYLOAD,
                                                                                 sizeof(APP_AZURE_IOT_PNP_DPS_PAYLOAD) - 1)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_registration_payload_set!: error code = 0x%08x\r\n", status);
    }
    /* Register device.  */
    else if ((status = nx_azure_iot_provisioning_client_register(prov_client_ptr, NX_WAIT_FOREVER)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_register!: error code = 0x%08x\r\n", status);
    }

    /* Get Device info.  */
    else if ((status = nx_azure_iot_provisioning_client_iothub_device_info_get(prov_client_ptr,
                                                                               iothub_hostname_buffer, iothub_hostname_length,
                                                                               iothub_device_id_buffer, iothub_device_id_length)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_iothub_device_info_get!: error code = 0x%08x\r\n", status);
    }
    else
    {
        *iothub_hostname = iothub_hostname_buffer;
        *iothub_device_id = iothub_device_id_buffer;
        printf("Registered Device Successfully.\r\n");
    }

    /* Destroy Provisioning Client.  */
    nx_azure_iot_provisioning_client_deinitialize(prov_client_ptr);

    return(status);
}
#endif /* ENABLE_DPS */

static VOID connected_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status;

    /* Request all properties.  */
    if ((status = nx_azure_iot_hub_client_properties_request(hub_client_ptr, NX_WAIT_FOREVER)))
    {
        printf("Properties request failed!: error code = 0x%08x\r\n", status);
    }
    else
    {
        printf("Sent properties request.\r\n");
    }
}

static VOID periodic_timer_entry(ULONG context)
{

    NX_PARAMETER_NOT_USED(context);
    tx_event_flags_set(&(tx_events), APP_AZURE_IOT_PERIODIC_EVENT, TX_OR);
}

static VOID periodic_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{    
static int32_t SendTelemetry = 1;

    if ((tx_time_get() - last_periodic_send_action_tick) >= (std_comp.TelemetryIntervall_IpPeriodicalRate))
    {
        last_periodic_send_action_tick = tx_time_get();
        if (SendTelemetry) 
        {
            tx_event_flags_set(&(tx_events), APP_AZURE_IOT_TELEMETRY_SEND_EVENT, TX_OR);
        }
    }
          
    if ((tx_time_get() - last_periodic_action_tick) >= (NX_IP_PERIODIC_RATE))
    {
        last_periodic_action_tick = tx_time_get();
        if((std_comp_property_sent_status.SentAccFullScale==0) || (std_comp_property_sent_status.SentGyroFullScale==0) ||
           (std_comp_property_sent_status.SentIntervalSec==0)  || (std_comp_property_sent_status.SentLED_ControlStatus==0) ||
           (device_info_component_property_sent_status.SentAllProperties == 0))
        {

            /* Only if we need to update something */
            tx_event_flags_set(&(tx_events), APP_AZURE_IOT_REPORTED_PROPERTIES_SEND_EVENT, TX_OR);
        }
    }

    if (ctrl_component_monitor.ReceivedPlayCommand)
    {
        ctrl_component_monitor.ReceivedPlayCommand = 0;
        SendTelemetry = 1;
        printf("Telemetry Transmission Restarted\r\n");
    }

    if (ctrl_component_monitor.ReceivedPauseCommand)
    {
        ctrl_component_monitor.ReceivedPauseCommand = 0;
        SendTelemetry = 0;
        printf("Telemetry Transmission Paused\r\n");
    }

    if (((tx_time_get() - ctrl_component_monitor.ResetTime) >= 5 * NX_IP_PERIODIC_RATE) && (ctrl_component_monitor.ReceivedResetCommand))
    {
        ctrl_component_monitor.ReceivedResetCommand = 0;
        tx_event_flags_set(&(tx_events), APP_AZURE_IOT_RESET_EVENT, TX_OR);
    }
}

static void log_callback(az_log_classification classification, UCHAR *msg, UINT msg_len)
{
    if (classification == AZ_LOG_IOT_AZURERTOS)
    {
        printf("%.*s", msg_len, (CHAR *)msg);
    }
}

static UINT components_init()
{
UINT status;

    if ((status = std_component_init(&std_comp,
                                    (UCHAR *)std_component_name,
                                    sizeof(std_component_name) - 1)))
    {
        printf("Failed to initialize %s: error code = 0x%08x\r\n", std_component_name, status);
    }
    
    if ((status = ctrl_component_init(&ctrl_comp,
                                     (UCHAR *)ctrl_component_name,
                                     sizeof(ctrl_component_name) - 1,
                                     &ctrl_component_cmd_cbk,
                                     &ctrl_component_monitor)))
    {
        printf("Failed to initialize %s: error code = 0x%08x\r\n", ctrl_component_name, status);
    }

    if ((status = device_info_component_init(&device_info_component,
                                     (UCHAR *)device_info_component_name,
                                     sizeof(device_info_component_name) - 1)))
    {
        printf("Failed to initialize %s: error code = 0x%08x\r\n", device_info_component_name, status);
    }

    return(status);
}

void app_azure_iot_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time))
{
UINT status = 0;
UINT loop = NX_TRUE;
ULONG app_events;

    nx_azure_iot_log_init(log_callback);

    if ((status = components_init()))
    {
        printf("Failed on initialize components!: error code = 0x%08x\r\n", status);
        return;
    }

    /* Create Azure IoT handler.  */
    if ((status = nx_azure_iot_create(&nx_azure_iot, (UCHAR *)"Azure IoT", ip_ptr, pool_ptr, dns_ptr,
                                      nx_azure_iot_thread_stack, sizeof(nx_azure_iot_thread_stack),
                                      NX_AZURE_IOT_THREAD_PRIORITY, unix_time_callback)))
    {
        printf("Failed on nx_azure_iot_create!: error code = 0x%08x\r\n", status);
        return;
    }

    /* Initialize CA certificates.  */
    if ((status = nx_secure_x509_certificate_initialize(&root_ca_cert, (UCHAR *)_nx_azure_iot_root_cert,
                                                        (USHORT)_nx_azure_iot_root_cert_size,
                                                        NX_NULL, 0, NULL, 0, NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return;
    }

    if ((status = nx_secure_x509_certificate_initialize(&root_ca_cert_2, (UCHAR *)_nx_azure_iot_root_cert_2,
                                                        (USHORT)_nx_azure_iot_root_cert_size_2,
                                                        NX_NULL, 0, NULL, 0, NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return;
    }

    if ((status = nx_secure_x509_certificate_initialize(&root_ca_cert_3, (UCHAR *)_nx_azure_iot_root_cert_3,
                                                        (USHORT)_nx_azure_iot_root_cert_size_3,
                                                        NX_NULL, 0, NULL, 0, NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return;
    }

    tx_timer_create(&(tx_timer), (CHAR*)"app_timer", periodic_timer_entry, 0,
                    NX_IP_PERIODIC_RATE, NX_IP_PERIODIC_RATE, TX_AUTO_ACTIVATE);
    tx_event_flags_create(&tx_events, (CHAR*)"app_event");

    while (loop)
    {

        /* Pickup app event flags.  */
        tx_event_flags_get(&(tx_events), APP_AZURE_IOT_ALL_EVENTS, TX_OR_CLEAR, &app_events, NX_WAIT_FOREVER);

        if (app_events & APP_AZURE_IOT_CONNECTED_EVENT)
        {
            connected_action(&iothub_client);
        }

        if (app_events & APP_AZURE_IOT_PERIODIC_EVENT)
        {
            periodic_action(&iothub_client);
        }

        if (app_events & APP_AZURE_IOT_TELEMETRY_SEND_EVENT)
        {
            telemetry_action(&iothub_client);
        }

        if (app_events & APP_AZURE_IOT_COMMAND_RECEIVE_EVENT)
        {
            command_action(&iothub_client);
        }

        if (app_events & APP_AZURE_IOT_PROPERTIES_RECEIVE_EVENT)
        {
            properties_receive_action(&iothub_client);
        }

        if (app_events & APP_AZURE_IOT_WRITABLE_PROPERTIES_RECEIVE_EVENT)
        {
            writable_properties_receive_action(&iothub_client);
        }

        if (app_events & APP_AZURE_IOT_REPORTED_PROPERTIES_SEND_EVENT)
        {
            reported_properties_send_action(&iothub_client);
        }
        
        if (app_events & APP_AZURE_IOT_RESET_EVENT)
        {
            NVIC_SystemReset();
        }

        /* Connection monitor.  */
        sample_connection_monitor(ip_ptr, &iothub_client, connection_status, initialize_iothub);
    }

    /* Cleanup.  */
    tx_event_flags_delete(&tx_events);
    tx_timer_delete(&tx_timer);
    nx_azure_iot_hub_client_deinitialize(&iothub_client);
    nx_azure_iot_delete(&nx_azure_iot);
}
