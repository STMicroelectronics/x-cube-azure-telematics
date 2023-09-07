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
/*   1 - Modified to manage the telemetry:                                              */
/*    .temperature                                                                      */
/*    .humidity                                                                         */
/*    .pressure                                                                         */
/*    .acceleration                                                                     */
/*    .gyroscope                                                                        */
/*    .magnetometer                                                                     */
/*    .button_counter                                                                   */
/*                                                                                      */
/*   2 - Modified to manage the property:                                               */
/*    .acc_fullscale                                                                    */
/*    .gyro_fullscale                                                                   */
/*    .telemetry_interval                                                               */
/*    .control_LED                                                                      */
/*                                                                                      */
/****************************************************************************************/

#include "std_component.h"
#include "main.h"
#include "nx_azure_iot_hub_client_properties.h"
#include "dm_http_status_code.h"

#define DOUBLE_DECIMAL_PLACE_DIGITS                                     (2)

/* Telemetry key */
static const CHAR temp_value_telemetry_name[]   = "temperature";
static const CHAR hum_value_telemetry_name[]    = "humidity";
static const CHAR press_value_telemetry_name[]  = "pressure";
static const CHAR acc_value_telemetry_name[]    = "acceleration";
static const CHAR gryo_value_telemetry_name[]   = "gyroscope";
static const CHAR mag_value_telemetry_name[]    = "magnetometer";
static const CHAR button_counter_telemetry_name[]    = "button_counter";

/* Telemetry value key names  */
static const CHAR acc_x_key[] = "a_x";
static const CHAR acc_y_key[] = "a_y";
static const CHAR acc_z_key[] = "a_z";
static const CHAR gyro_x_key[] = "g_x";
static const CHAR gyro_y_key[] = "g_y";
static const CHAR gyro_z_key[] = "g_z";
static const CHAR mag_x_key[] = "m_x";
static const CHAR mag_y_key[] = "m_y";
static const CHAR mag_z_key[] = "m_z";

/* Property Names  */
static const CHAR acc_fullscale_property_name[]             = "acc_fullscale";
static const CHAR gyro_fullscale_property_name[]            = "gyro_fullscale";
static const CHAR telemetry_interval_property_name[]        = "telemetry_interval";
static const CHAR led_control_name[]                        = "control_LED";

static const CHAR property_response_description_success[]       = "success";
static const CHAR property_response_description_failed[]        = "failed";

static UCHAR scratch_buffer[512];

static UINT sensors_initialized = 0;

static UINT append_acc_fullscale(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr, VOID *context)
{
    return(nx_azure_iot_json_writer_append_property_with_int32_value(json_writer_ptr,
                                                                     (UCHAR *)acc_fullscale_property_name,
                                                                     sizeof(acc_fullscale_property_name) - 1,
                                                                     *(STD_COMP_AccFullScaleTypeDef *)context));
}

static UINT append_gyro_fullscale(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr, VOID *context)
{
    return(nx_azure_iot_json_writer_append_property_with_int32_value(json_writer_ptr,
                                                                     (UCHAR *)gyro_fullscale_property_name,
                                                                     sizeof(gyro_fullscale_property_name) - 1,
                                                                     *(STD_COMP_GyroFullScaleTypeDef *)context));
}

static UINT append_telemetry_interval(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr, VOID *context)
{
    return(nx_azure_iot_json_writer_append_property_with_double_value(json_writer_ptr,
                                                                      (UCHAR *)telemetry_interval_property_name,
                                                                      sizeof(telemetry_interval_property_name) - 1,
                                                                      *(double *)context,DOUBLE_DECIMAL_PLACE_DIGITS));
}

static UINT append_control_LED(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr, VOID *context)
{
    return(nx_azure_iot_json_writer_append_property_with_int32_value(json_writer_ptr,
                                                                     (UCHAR *)led_control_name,
                                                                     sizeof(led_control_name) - 1,
                                                                     *(UINT *)context));
}
            
static UINT append_int32_value(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr, VOID *context)
{
    return (nx_azure_iot_json_writer_append_int32(json_writer_ptr,*(int32_t *)context));
}

static UINT append_double_value(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr, VOID *context)
{
    return (nx_azure_iot_json_writer_append_double(json_writer_ptr,*(double *)context,DOUBLE_DECIMAL_PLACE_DIGITS));
}

static UINT report_property(STD_COMPONENT *handle,
                                       NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr,
                                       UINT (*append_reported_property)(NX_AZURE_IOT_JSON_WRITER *json_builder_ptr,
                                                                        VOID *context),
                                       VOID *context)
{
UINT status;
UINT response_status = 0;
NX_PACKET *packet_ptr;
NX_AZURE_IOT_JSON_WRITER json_writer;

    if ((status = nx_azure_iot_hub_client_reported_properties_create(iothub_client_ptr,
                                                                     &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Failed create reported properties: error code = 0x%08x\r\n", status);
        return(status);
    }

    if ((status = nx_azure_iot_json_writer_init(&json_writer, packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Failed to initialize json writer\r\n");
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    if ((status = nx_azure_iot_json_writer_append_begin_object(&json_writer)) ||
        (status = nx_azure_iot_hub_client_reported_properties_component_begin(iothub_client_ptr,
                                                                              &json_writer,
                                                                              handle->component_name_ptr, 
                                                                              handle->component_name_length)) ||
        (status = append_reported_property(&json_writer, context)) ||
        (status = nx_azure_iot_hub_client_reported_properties_component_end(iothub_client_ptr,
                                                                            &json_writer)) ||
        (status = nx_azure_iot_json_writer_append_end_object(&json_writer)))
    {
        printf("Failed to build reported property!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return(status);
    }

    if ((status = nx_azure_iot_hub_client_reported_properties_send(iothub_client_ptr,
                                                                   packet_ptr,
                                                                   NX_NULL, &response_status,
                                                                   NX_NULL,
                                                                   NX_IP_PERIODIC_RATE)))
    {
        printf("Device twin reported properties failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return(status);
    }

    if (DM_HTTP_CODE_IS_ERROR(response_status))
    {
        printf("device twin report properties failed with code : %d\r\n", response_status);
        return(NX_NOT_SUCCESSFUL);
    }

    return(status);
}

UINT std_component_pnp_report_acc_fullscale_property(STD_COMPONENT *handle, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr)
{
  return(report_property(handle, iothub_client_ptr, append_acc_fullscale, &handle->currentAccFS));
}

UINT std_component_pnp_report_gyro_fullscale_property(STD_COMPONENT *handle, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr)
{
  return(report_property(handle, iothub_client_ptr, append_gyro_fullscale, &handle->currentGyroFS));
}

UINT std_component_pnp_report_telemetry_interval_property(STD_COMPONENT *handle, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr)
{
  return(report_property(handle, iothub_client_ptr, append_telemetry_interval, &handle->CurrentTelemetryInterval));
}

UINT std_component_pnp_report_control_led_property(STD_COMPONENT *handle, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr)
{
  return(report_property(handle, iothub_client_ptr, append_control_LED, &handle->CurrentLED_ControlStatus));
}

static VOID report_property_update(STD_COMPONENT *handle,
                                              NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr,
                                              INT status_code, UINT version, const CHAR *description,
                                              UCHAR *property_name, UINT property_name_len,
                                              UINT (*append_reported_property)(NX_AZURE_IOT_JSON_WRITER *json_builder_ptr,
                                                                               VOID *context),
                                              VOID *context)
{
UINT response_status;
NX_PACKET *packet_ptr;
NX_AZURE_IOT_JSON_WRITER json_writer;

    if (nx_azure_iot_hub_client_reported_properties_create(iothub_client_ptr,
                                                           &packet_ptr, NX_WAIT_FOREVER))
    {
        printf("Failed to build reported response\r\n");
        return;
    }

    if (nx_azure_iot_json_writer_init(&json_writer, packet_ptr, NX_WAIT_FOREVER))
    {
        printf("Failed init json writer\r\n");
        nx_packet_release(packet_ptr);
        return;
    }

    if (nx_azure_iot_json_writer_append_begin_object(&json_writer) ||
        nx_azure_iot_hub_client_reported_properties_component_begin(iothub_client_ptr, &json_writer,
                                                                    handle ->component_name_ptr,
                                                                    handle -> component_name_length) ||
        nx_azure_iot_hub_client_reported_properties_status_begin(iothub_client_ptr,
                                                                 &json_writer, (UCHAR *)property_name,
                                                                 property_name_len,
                                                                 status_code, version,
                                                                 (const UCHAR *)description, strlen(description)) ||
        append_reported_property(&json_writer, context) ||
        nx_azure_iot_hub_client_reported_properties_status_end(iothub_client_ptr, &json_writer) ||
        nx_azure_iot_hub_client_reported_properties_component_end(iothub_client_ptr, &json_writer) ||
        nx_azure_iot_json_writer_append_end_object(&json_writer))
    {
        printf("Failed to create reported response\r\n");
        nx_packet_release(packet_ptr);
    }
    else
    {
        if (nx_azure_iot_hub_client_reported_properties_send(iothub_client_ptr,
                                                             packet_ptr, NX_NULL,
                                                             &response_status, NX_NULL,
                                                             NX_IP_PERIODIC_RATE))
        {
            printf("Failed to send reported response\r\n");
            nx_packet_release(packet_ptr);
        }
    }
}


static VOID std_component_pnp_report_acc_fullscale_property_update(STD_COMPONENT *handle,
                                                            NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr, int32_t fullscale,
                                                            INT status_code, UINT version, const CHAR *description)
{
    report_property_update(handle, iothub_client_ptr, status_code, version, description,
                                      (UCHAR *)acc_fullscale_property_name,
                                      sizeof(acc_fullscale_property_name) - 1,
                                      append_int32_value, (VOID *)&fullscale);
}

static VOID std_component_pnp_report_gyro_fullscale_property_update(STD_COMPONENT *handle,
                                                             NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr, int32_t fullscale,
                                                             INT status_code, UINT version, const CHAR *description)
{
    report_property_update(handle, iothub_client_ptr, status_code, version, description,
                                      (UCHAR *)gyro_fullscale_property_name,
                                      sizeof(gyro_fullscale_property_name) - 1,
                                      append_int32_value, (VOID *)&fullscale);
}


static VOID std_component_pnp_report_telemetry_interval_property_update(STD_COMPONENT *handle,
                                                                 NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr, double Interval,
                                                                 INT status_code, UINT version, const CHAR *description)
{
    report_property_update(handle, iothub_client_ptr, status_code, version, description,
                                      (UCHAR *)telemetry_interval_property_name,
                                      sizeof(telemetry_interval_property_name) - 1,
                                      append_double_value, (VOID *)&Interval);
}

static VOID std_component_pnp_report_control_led_property_update(STD_COMPONENT *handle,
                                                          NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr, UINT LedControl,
                                                          INT status_code, UINT version, const CHAR *description)
{
    report_property_update(handle, iothub_client_ptr, status_code, version, description,
                                      (UCHAR *)led_control_name,
                                      sizeof(led_control_name) - 1,
                                      append_int32_value, (VOID *)&LedControl);
}

static UINT std_component_read_acc_values(STD_COMPONENT *handle) 
{
  UINT status;
  BSP_MOTION_SENSOR_Axes_t Acc_XYZ_Data;

  if(handle == NULL) 
  {
    return NX_AZURE_IOT_FAILURE;
  }

  /* Read the Acc values */
  if (BSP_MOTION_SENSOR_GetAxes(ISM330DHCX_0, MOTION_ACCELERO, &Acc_XYZ_Data)) {
      printf("ACC[%d]: Error\r\n", (int)ISM330DHCX_0);
      status = NX_AZURE_IOT_FAILURE;
  } 
  else {
      handle -> Acc_X= (double)Acc_XYZ_Data.xval;
      handle -> Acc_Y= (double)Acc_XYZ_Data.yval;
      handle -> Acc_Z= (double)Acc_XYZ_Data.zval;
      status = NX_AZURE_IOT_SUCCESS;
  }
  return status;
}

static UINT std_component_read_gyro_values(STD_COMPONENT *handle) 
{
  UINT status;
  BSP_MOTION_SENSOR_Axes_t Gyro_XYZ_Data;

  if(handle == NULL) 
  {
    return NX_AZURE_IOT_FAILURE;
  }

  /* Read the gyro values */
  if (BSP_MOTION_SENSOR_GetAxes(ISM330DHCX_0, MOTION_GYRO, &Gyro_XYZ_Data)){
      printf("GYR[%d]: Error\r\n", (int)ISM330DHCX_0);
      status = NX_AZURE_IOT_FAILURE;
  } 
  else {
    handle -> Gyro_X= (double)Gyro_XYZ_Data.xval;
    handle -> Gyro_Y= (double)Gyro_XYZ_Data.yval;
    handle -> Gyro_Z= (double)Gyro_XYZ_Data.zval;
      status = NX_AZURE_IOT_SUCCESS;
  }

  return status;
}

static UINT std_component_read_mag_values(STD_COMPONENT *handle) 
{
  UINT status;
  BSP_MOTION_SENSOR_Axes_t Mag_XYZ_Data;

  if(handle == NULL) 
  {
    return NX_AZURE_IOT_FAILURE;
  }

  /* Read the magnetometer values */
  if (BSP_MOTION_SENSOR_GetAxes(IIS2MDC_0, MOTION_MAGNETO, &Mag_XYZ_Data)){
      printf("MAG[%d]: Error\r\n", (int)IIS2MDC_0);
      status = NX_AZURE_IOT_FAILURE;
  } else 
  {
    handle -> Mag_X= (double)Mag_XYZ_Data.xval;
    handle -> Mag_Y= (double)Mag_XYZ_Data.yval;
    handle -> Mag_Z= (double)Mag_XYZ_Data.zval;
    status = NX_AZURE_IOT_SUCCESS;
  }
  
  return status;
}

static UINT std_component_read_temperature_value(STD_COMPONENT *handle) 
{
  UINT status;
  float temperatureValue;

  if(handle == NULL) 
  {
    return NX_AZURE_IOT_FAILURE;
  }

  /* Read temperature value */
  if (BSP_ENV_SENSOR_GetValue(HTS221_0, ENV_TEMPERATURE, &temperatureValue)){
    printf("Temp[%d]: Error\r\n", (int)HTS221_0);
    status = NX_AZURE_IOT_FAILURE;
  } else 
  {
    handle -> Temperature = temperatureValue;
    status = NX_AZURE_IOT_SUCCESS;
  }

  return status;
}

static UINT std_component_read_humidity_value(STD_COMPONENT *handle) 
{
  UINT status;
  float humidityValue;

  if(handle == NULL) 
  {
    return NX_AZURE_IOT_FAILURE;
  }

  /* Read humidity value */
  if (BSP_ENV_SENSOR_GetValue(HTS221_0, ENV_HUMIDITY, &humidityValue)) {
      printf("Hum[%d]: Error\r\n", (int)HTS221_0);
    status = NX_AZURE_IOT_FAILURE;
  } else 
  {
    handle -> Humidity = humidityValue;
    status = NX_AZURE_IOT_SUCCESS;
  }

  return status;
}

static UINT std_component_read_pressure_value(STD_COMPONENT *handle) 
{
  UINT status;
  float pressureValue;

  if(handle == NULL) 
  {
    return NX_AZURE_IOT_FAILURE;
  }

  /* Read pressure value */
  if (BSP_ENV_SENSOR_GetValue(LPS22HH_0, ENV_PRESSURE, &pressureValue)){
      printf("Press[%d]: Error\r\n", (int)LPS22HH_0);
    status = NX_AZURE_IOT_FAILURE;
  } else 
  {
    handle -> Pressure = pressureValue;
    status = NX_AZURE_IOT_SUCCESS;
  }

  return status;
}

static UINT append_telemetry_acc_values(STD_COMPONENT *handle, NX_AZURE_IOT_JSON_WRITER *json_writer) 
{
  UINT status;

  /* Appends the property name as a JSON string */
  status= nx_azure_iot_json_writer_append_property_name(json_writer,
                                                (UCHAR *)acc_value_telemetry_name,
                                                sizeof(acc_value_telemetry_name) - 1);
      
  /* Appends the beginning of the JSON object (i.e. `{`) */
  if(status == NX_AZURE_IOT_SUCCESS) {
    status = nx_azure_iot_json_writer_append_begin_object (json_writer);
  }
  
  /* Appends the objects name and related values in double format */
  if(status == NX_AZURE_IOT_SUCCESS) {
    status= nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
                                                              (const UCHAR *)acc_x_key,
                                                              sizeof(acc_x_key) - 1,
                                                              handle -> Acc_X,
                                                              DOUBLE_DECIMAL_PLACE_DIGITS);
  }
  
  if(status == NX_AZURE_IOT_SUCCESS) {
      status= nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
                                                                (const UCHAR *)acc_y_key,
                                                                sizeof(acc_y_key) - 1,
                                                                handle -> Acc_Y,
                                                                DOUBLE_DECIMAL_PLACE_DIGITS);
  }
  
  if(status == NX_AZURE_IOT_SUCCESS) {
      status= nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
                                                                (UCHAR *)acc_z_key,
                                                                sizeof(acc_z_key) - 1,
                                                                handle -> Acc_Z,
                                                                DOUBLE_DECIMAL_PLACE_DIGITS);
  }
  
  /* Appends the end of the current JSON object (i.e. `}`) */
  if(status == NX_AZURE_IOT_SUCCESS) {
      status= nx_azure_iot_json_writer_append_end_object(json_writer);
  }

  return status;
}

static UINT append_telemetry_gyro_values(STD_COMPONENT *handle, NX_AZURE_IOT_JSON_WRITER *json_writer) 
  {
  UINT status;
  /* Appends the property name as a JSON string */
  status= nx_azure_iot_json_writer_append_property_name(json_writer,
                                                (UCHAR *)gryo_value_telemetry_name,
                                                sizeof(gryo_value_telemetry_name) - 1);
      
  /* Appends the beginning of the JSON object (i.e. `{`) */
  if(status == NX_AZURE_IOT_SUCCESS) {
    status = nx_azure_iot_json_writer_append_begin_object (json_writer);
  }
  
  /* Appends the objects name and related values in double format */
  if(status == NX_AZURE_IOT_SUCCESS) {
    status= nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
                                                              (const UCHAR *)gyro_x_key,
                                                              sizeof(gyro_x_key) - 1,
                                                              handle -> Gyro_X,
                                                              DOUBLE_DECIMAL_PLACE_DIGITS);
  }
  
  if(status == NX_AZURE_IOT_SUCCESS) {
      status= nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
                                                                (UCHAR *)gyro_y_key,
                                                                sizeof(gyro_y_key) - 1,
                                                                handle -> Gyro_Y,
                                                                DOUBLE_DECIMAL_PLACE_DIGITS);
  }
  
  if(status == NX_AZURE_IOT_SUCCESS) {
      status= nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
                                                                (UCHAR *)gyro_z_key,
                                                                sizeof(gyro_z_key) - 1,
                                                                handle -> Gyro_Z,
                                                                DOUBLE_DECIMAL_PLACE_DIGITS);
  }
  
  /* Appends the end of the current JSON object (i.e. `}`) */
  if(status == NX_AZURE_IOT_SUCCESS) {
      status= nx_azure_iot_json_writer_append_end_object(json_writer);
  }

  return status;
}

static UINT append_telemetry_mag_values(STD_COMPONENT *handle, NX_AZURE_IOT_JSON_WRITER *json_writer) 
{
  UINT status;
  /* Appends the property name as a JSON string */
  status= nx_azure_iot_json_writer_append_property_name(json_writer,
                                                (UCHAR *)mag_value_telemetry_name,
                                                sizeof(mag_value_telemetry_name) - 1);
      
  /* Appends the beginning of the JSON object (i.e. `{`) */
  if(status == NX_AZURE_IOT_SUCCESS) {
    status = nx_azure_iot_json_writer_append_begin_object (json_writer);
  }
  
  /* Appends the objects name and related values in double format */
  if(status == NX_AZURE_IOT_SUCCESS) {
    status= nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
                                                              (UCHAR *)mag_x_key,
                                                              sizeof(mag_x_key) - 1,
                                                              handle -> Mag_X,
                                                              DOUBLE_DECIMAL_PLACE_DIGITS);
  }
  
  if(status == NX_AZURE_IOT_SUCCESS) {
      status= nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
                                                                (UCHAR *)mag_y_key,
                                                                sizeof(mag_y_key) - 1,
                                                                handle ->  Mag_Y,
                                                                DOUBLE_DECIMAL_PLACE_DIGITS);
  }
  
  if(status == NX_AZURE_IOT_SUCCESS) {
      status= nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
                                                                (UCHAR *)mag_z_key,
                                                                sizeof(mag_z_key) - 1,
                                                                handle ->  Mag_Z,
                                                                DOUBLE_DECIMAL_PLACE_DIGITS);
  }
  
  /* Appends the end of the current JSON object (i.e. `}`) */
  if(status == NX_AZURE_IOT_SUCCESS) {
      status= nx_azure_iot_json_writer_append_end_object(json_writer);
  }

  return status;
}

static UINT append_telemetry_temperature_value(STD_COMPONENT *handle, NX_AZURE_IOT_JSON_WRITER *json_writer) 
{
  return (nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
                                                                (UCHAR *)temp_value_telemetry_name,
                                                                sizeof(temp_value_telemetry_name) - 1,
                                                                handle -> Temperature,
                                                                DOUBLE_DECIMAL_PLACE_DIGITS));
}

static UINT append_telemetry_humidity_value(STD_COMPONENT *handle, NX_AZURE_IOT_JSON_WRITER *json_writer) 
{
  return (nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
                                                                  (UCHAR *)hum_value_telemetry_name,
                                                                  sizeof(hum_value_telemetry_name) - 1,
                                                                  handle -> Humidity,
                                                                  DOUBLE_DECIMAL_PLACE_DIGITS));
}

static UINT append_telemetry_pressure_value(STD_COMPONENT *handle, NX_AZURE_IOT_JSON_WRITER *json_writer) 
{
  return (nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
                                                                  (UCHAR *)press_value_telemetry_name,
                                                                  sizeof(press_value_telemetry_name) - 1,
                                                                  handle -> Pressure,
                                                                  DOUBLE_DECIMAL_PLACE_DIGITS));
}

static UINT append_telemetry_button_counter_value(STD_COMPONENT *handle, NX_AZURE_IOT_JSON_WRITER *json_writer) 
{
  return (nx_azure_iot_json_writer_append_property_with_int32_value(json_writer,
                                                                    (UCHAR *)button_counter_telemetry_name,
                                                                    sizeof(button_counter_telemetry_name) - 1,
                                                                    handle -> ButtonCounter));
}

static VOID on_telemetry_sent(STD_COMPONENT *handle) {
  if(NULL != handle)
  {
    /* Reset button counter on telemetry sent */
    handle->ButtonCounter = 0;
  }
}

static UINT process_acc_fullscale_property_update(STD_COMPONENT *handle, 
                                                  NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr, 
                                                  NX_AZURE_IOT_JSON_READER *json_reader_ptr, 
                                                  UINT version) 
{
UINT status = NX_NOT_SUCCESSFUL;
UINT status_code;
const CHAR *description;
int32_t parsed_value_int = 0;

  if (nx_azure_iot_json_reader_token_is_text_equal(json_reader_ptr,
                                                     (UCHAR *)acc_fullscale_property_name,
                                                     sizeof(acc_fullscale_property_name) - 1))
    {
      if (nx_azure_iot_json_reader_next_token(json_reader_ptr) ||
          nx_azure_iot_json_reader_token_int32_get(json_reader_ptr, &parsed_value_int))
      {
        status_code = DM_HTTP_BAD_REQUEST;
        description = property_response_description_failed;
      }
      else
      {
        status_code = DM_HTTP_SUCCESS;
        description = property_response_description_success;

        switch (parsed_value_int)
        {
        case STD_COMP_ACC_FS_2G:
          /* Nothing to do */
          break;
        case STD_COMP_ACC_FS_4G:
          /* Nothing to do */
          break;
        case STD_COMP_ACC_FS_8G:
          /* Nothing to do */
          break;
        case STD_COMP_ACC_FS_16G:
          /* Nothing to do */
          break;
        default:
          printf("Wrong Full Scale %ldG for accelerometer ISM330DLC\r\n", parsed_value_int);
          status_code = DM_HTTP_BAD_REQUEST;
          description = property_response_description_failed;
          break;
        }

        if(DM_HTTP_SUCCESS == status_code)
        {
          if(BSP_MOTION_SENSOR_SetFullScale(ISM330DHCX_0, MOTION_ACCELERO, parsed_value_int)==BSP_ERROR_NONE) {
            handle->currentAccFS = (STD_COMP_AccFullScaleTypeDef)parsed_value_int;
            printf("Set Full Scale %ldG for accelerometer ISM330DLC\r\n", parsed_value_int);
          } else {
            status_code = DM_HTTP_INTERNAL_SERVER_ERROR;
            description = property_response_description_failed;
            printf("Error Set Full Scale %ldG for accelerometer ISM330DLC\r\n", parsed_value_int);
          }
        }
      }
      // Set Acc FullScale report
      std_component_pnp_report_acc_fullscale_property_update(handle,
                                                      iothub_client_ptr, handle->currentAccFS,
                                                      status_code, version, description);

      status = NX_AZURE_IOT_SUCCESS;
    }
    return status;
}

static UINT process_gyro_fullscale_property_update(STD_COMPONENT *handle, 
                                                  NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr, 
                                                  NX_AZURE_IOT_JSON_READER *json_reader_ptr, 
                                                  UINT version) 
{
UINT status = NX_NOT_SUCCESSFUL;
UINT status_code;
const CHAR *description;
int32_t parsed_value_int = 0;

  if (nx_azure_iot_json_reader_token_is_text_equal(json_reader_ptr,
                                                     (UCHAR *)gyro_fullscale_property_name,
                                                     sizeof(gyro_fullscale_property_name) - 1))
    {
      if (nx_azure_iot_json_reader_next_token(json_reader_ptr) ||
          nx_azure_iot_json_reader_token_int32_get(json_reader_ptr, &parsed_value_int))
      {
        status_code = DM_HTTP_BAD_REQUEST;
        description = property_response_description_failed;
      }
      else
      {
        status_code = DM_HTTP_SUCCESS;
        description = property_response_description_success;

        switch (parsed_value_int)
        {
        case STD_COMP_GYRO_FS_125_DPS:
          /* Nothing to do */
          break;
        case STD_COMP_GYRO_FS_250_DPS:
          /* Nothing to do */
          break;
        case STD_COMP_GYRO_FS_500_DPS:
          /* Nothing to do */
          break;
        case STD_COMP_GYRO_FS_1000_DPS:
          /* Nothing to do */
          break;
        case STD_COMP_GYRO_FS_2000_DPS:
          /* Nothing to do */
          break;
        default:
          printf("Wrong Full Scale %ldDPS for gyroscope ISM330DLC\r\n", parsed_value_int);
          description = property_response_description_failed;
          status_code = DM_HTTP_BAD_REQUEST;
          break;
        }

        if(DM_HTTP_SUCCESS == status_code)
        {
          if(BSP_MOTION_SENSOR_SetFullScale(ISM330DHCX_0, MOTION_GYRO, parsed_value_int)==BSP_ERROR_NONE) {
              printf("Set Full Scale %ldDPS for gyroscope ISM330DLC\r\n", parsed_value_int);
              handle->currentGyroFS = (STD_COMP_GyroFullScaleTypeDef)parsed_value_int;
          } else {
            description = property_response_description_failed;
            status_code = DM_HTTP_INTERNAL_SERVER_ERROR;
            printf("Error Set Full Scale %ldDPS for gyroscope ISM330DLC\r\n", parsed_value_int);
          }
        }
      }
      // Set Gyro FullScale report
      std_component_pnp_report_gyro_fullscale_property_update(handle,
                                                       iothub_client_ptr, handle->currentGyroFS,
                                                       status_code, version, description);
      status = NX_AZURE_IOT_SUCCESS;
    }
    return (status);
}

static UINT process_telemetry_interval_property_update(STD_COMPONENT *handle, 
                                                  NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr, 
                                                  NX_AZURE_IOT_JSON_READER *json_reader_ptr, 
                                                  UINT version) 
{
UINT status = NX_NOT_SUCCESSFUL;
UINT status_code;
const CHAR *description;
double parsed_value_double = 0;
  if (nx_azure_iot_json_reader_token_is_text_equal(json_reader_ptr,
                                                     (UCHAR *)telemetry_interval_property_name,
                                                     sizeof(telemetry_interval_property_name) - 1))
    {
      if (nx_azure_iot_json_reader_next_token(json_reader_ptr) ||
          nx_azure_iot_json_reader_token_double_get(json_reader_ptr, &parsed_value_double))
      {
        status_code = DM_HTTP_BAD_REQUEST;
        description = property_response_description_failed;
      }
      else
      {
        status_code = DM_HTTP_SUCCESS;
        description = property_response_description_success;
        handle->CurrentTelemetryInterval = parsed_value_double;
        handle->TelemetryIntervall_IpPeriodicalRate = ((INT)(handle->CurrentTelemetryInterval)) * NX_IP_PERIODIC_RATE;

        printf("Set Telemetry Interval report %f\r\n", parsed_value_double);
      }
      // Set Telemetry Interval report
      std_component_pnp_report_telemetry_interval_property_update(handle,
                                                           iothub_client_ptr, handle->CurrentTelemetryInterval,
                                                           status_code, version, description);
    status = NX_AZURE_IOT_SUCCESS;
  }
  return (status);
}

static UINT process_led_control_property_update(STD_COMPONENT *handle, 
                                                  NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr, 
                                                  NX_AZURE_IOT_JSON_READER *json_reader_ptr, 
                                                  UINT version) 
{
UINT status = NX_NOT_SUCCESSFUL;
UINT status_code;
const CHAR *description;
int32_t parsed_value_int = 0;
  if (nx_azure_iot_json_reader_token_is_text_equal(json_reader_ptr,
                                                     (UCHAR *)led_control_name,
                                                     sizeof(led_control_name) - 1))
    {
      if (nx_azure_iot_json_reader_next_token(json_reader_ptr) ||
          nx_azure_iot_json_reader_token_int32_get(json_reader_ptr, &parsed_value_int))
      {
        status_code = DM_HTTP_BAD_REQUEST;
        description = property_response_description_failed;
      }
      else
      {
        if (parsed_value_int == 1)
        {
          if (BSP_ERROR_NONE == BSP_LED_On(LED_GREEN)) {
            printf("Set LED On\r\n");
            handle->CurrentLED_ControlStatus = parsed_value_int;
            status_code = DM_HTTP_SUCCESS;
            description = property_response_description_success;
          } else {
            printf("Set LED On error\r\n");
            status_code = DM_HTTP_INTERNAL_SERVER_ERROR;
            description = property_response_description_failed;
          }
        }

        else if (parsed_value_int == 0)
        {
          
          if (BSP_ERROR_NONE == BSP_LED_Off(LED_GREEN)) {
            printf("Set LED Off\r\n");
            handle->CurrentLED_ControlStatus = parsed_value_int;
            status_code = DM_HTTP_SUCCESS;
            description = property_response_description_success;
          } else {
            printf("Set LED Off error\r\n");
            status_code = DM_HTTP_INTERNAL_SERVER_ERROR;
            description = property_response_description_failed;
          }
        } else {
          printf("Set LED failed (with value %ld)\r\n", parsed_value_int);
          status_code = DM_HTTP_BAD_REQUEST;
          description = property_response_description_failed;
        }
      }
      // Set LED state
      std_component_pnp_report_control_led_property_update(handle,
                                                    iothub_client_ptr, handle->CurrentLED_ControlStatus,
                                                    status_code, version, description);
    status = NX_AZURE_IOT_SUCCESS;
  }
  return (status);
}

static void init_mems_sensors(void)
{
  if(sensors_initialized == 0) {
    printf("Init sensors on board:\r\n");
    if(BSP_MOTION_SENSOR_Init(IIS2MDC_0, MOTION_MAGNETO)==BSP_ERROR_NONE) {
      printf("\tInit magnetometer IIS2MDC sensor\r\n");
    } else {
      printf("\tError Init magnetometer IIS2MDC sensor\r\n");
    }

    if(BSP_MOTION_SENSOR_Init(ISM330DHCX_0, MOTION_ACCELERO | MOTION_GYRO)==BSP_ERROR_NONE) {
      printf("\tInit gyroscope and accelerometer ISM330DHCX sensor\r\n");
    } else {
      printf("\tError Init gyroscope and accelerometer ISM330DHCX sensor\r\n");
    }

    if(BSP_ENV_SENSOR_Init(HTS221_0, ENV_TEMPERATURE | ENV_HUMIDITY)==BSP_ERROR_NONE) {
      printf("\tInit temperature and humidity HTS221 sensor\r\n");
    } else {
      printf("\tError temperature and humidity Init HTS221 sensor\r\n");
    }

    if(BSP_ENV_SENSOR_Init(LPS22HH_0, ENV_PRESSURE)==BSP_ERROR_NONE) {
      printf("\tInit temperature and pressure LPS22HH sensor\r\n");
    } else {
      printf("\tError Init temperature and pressure LPS22HH sensor\r\n");
    }
    
    if(BSP_MOTION_SENSOR_Enable(IIS2MDC_0, MOTION_MAGNETO)==BSP_ERROR_NONE) {
      printf("\tEnabled magnetometer IIS2MDC sensor\r\n");
    } else {
      printf("\tNot enabled magnetometer IIS2MDC sensor\r\n");
    }
    
    if(BSP_MOTION_SENSOR_Enable(ISM330DHCX_0, MOTION_ACCELERO)==BSP_ERROR_NONE) {
      printf("\tEnabled accelerometer ISM330DHCX sensor\r\n");
    } else {
      printf("\tNot enabled accelerometer ISM330DHCX sensor\r\n");
    }
    
    if(BSP_MOTION_SENSOR_Enable(ISM330DHCX_0, MOTION_GYRO)==BSP_ERROR_NONE) {
      printf("\tEnabled gyroscope ISM330DHCX sensor\r\n");
    } else {
      printf("\tNot enabled gyroscope ISM330DHCX sensor\r\n");
    }
    
    if(BSP_ENV_SENSOR_Enable(HTS221_0, ENV_TEMPERATURE)==BSP_ERROR_NONE) {
      printf("\tEnabled temperature HTS221 sensor\r\n");
    } else {
      printf("\tNot enabled temperature HTS221 sensor\r\n");
    }
    
    if(BSP_ENV_SENSOR_Enable(HTS221_0, ENV_HUMIDITY)==BSP_ERROR_NONE) {
      printf("\tEnabled humidity HTS221 sensor\r\n");
    } else {
      printf("\tNot enabled humidity HTS221 sensor\r\n");
    }
    
    if(BSP_ENV_SENSOR_Enable(LPS22HH_0, ENV_PRESSURE)==BSP_ERROR_NONE) {
      printf("\tEnabled pressure LPS22HH sensor\r\n");
    } else {
      printf("\tNot enabled pressure LPS22HH sensor\r\n");
    }
    sensors_initialized = 1;
  } else {
    printf("Sensors already initialized\r\n");
  }
}

UINT std_component_init(STD_COMPONENT *handle,
                       UCHAR *component_name_ptr, UINT component_name_length)
{
  if (handle == NX_NULL)
  {
      return(NX_NOT_SUCCESSFUL);
  }

  init_mems_sensors();
  
  handle -> component_name_ptr = component_name_ptr;
  handle -> component_name_length = component_name_length;
  
  handle -> Acc_X = 0.0;
  handle -> Acc_Y = 0.0;
  handle -> Acc_Z = 0.0;
  
  handle -> Gyro_X = 0.0;
  handle -> Gyro_Y = 0.0;
  handle -> Gyro_Z = 0.0;

  handle -> Mag_X = 0.0;
  handle -> Mag_Y = 0.0;
  handle -> Mag_Z = 0.0;
  
  handle -> Temperature = 0;
  handle -> Humidity = 0;
  handle -> Pressure = 0;
  
  handle -> currentAccFS  = STD_COMP_ACC_FS_4G;
  
  if(BSP_MOTION_SENSOR_SetFullScale(ISM330DHCX_0, MOTION_ACCELERO, handle->currentAccFS)==BSP_ERROR_NONE) {
    printf("Init Full Scale %dG for accelerometer ISM330DLC\r\n", handle->currentAccFS);
  } else {
    printf("Error Init Full Scale %dG for accelerometer ISM330DLC\r\n", handle->currentAccFS);
  }
  
  handle -> currentGyroFS  = STD_COMP_GYRO_FS_250_DPS;

  if(BSP_MOTION_SENSOR_SetFullScale(ISM330DHCX_0, MOTION_GYRO, handle->currentGyroFS)==BSP_ERROR_NONE) {
    printf("Init Full Scale %dDPS for gyroscope ISM330DLC\r\n", handle->currentGyroFS);
  } else {
    printf("Error Init Full Scale %dDPS for gyroscope ISM330DLC\r\n", handle->currentGyroFS);
  }
  
  //Telemetry Update Interval
  handle -> CurrentTelemetryInterval = 5.0;
  handle -> TelemetryIntervall_IpPeriodicalRate = ((INT)(handle -> CurrentTelemetryInterval)) * NX_IP_PERIODIC_RATE;
  
  handle -> CurrentLED_ControlStatus = 0;

  handle -> ButtonCounter = 0;
  BSP_LED_Off(LED_GREEN);

  return(NX_AZURE_IOT_SUCCESS);
}

UINT std_component_read_sensor_values(STD_COMPONENT *handle) {
  UINT status = NX_AZURE_IOT_SUCCESS;
  
  /* Read sensors as best efforts, ie. read other sensors even if a sensor failed */
  if(std_component_read_acc_values(handle) != NX_AZURE_IOT_SUCCESS) {
    status = NX_AZURE_IOT_FAILURE;
  }

  if(std_component_read_gyro_values(handle) != NX_AZURE_IOT_SUCCESS) {
    status = NX_AZURE_IOT_FAILURE;
  }

  if(std_component_read_mag_values(handle) != NX_AZURE_IOT_SUCCESS) {
    status = NX_AZURE_IOT_FAILURE;
  }

  if(std_component_read_temperature_value(handle) != NX_AZURE_IOT_SUCCESS) {
    status = NX_AZURE_IOT_FAILURE;
  }

  if(std_component_read_humidity_value(handle) != NX_AZURE_IOT_SUCCESS) {
    status = NX_AZURE_IOT_FAILURE;
  }

  if(std_component_read_pressure_value(handle) != NX_AZURE_IOT_SUCCESS) {
    status = NX_AZURE_IOT_FAILURE;
  }

  return status;
}

UINT std_component_telemetry_send(STD_COMPONENT *handle, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr)
{
  UINT status;
  NX_PACKET *packet_ptr;
  NX_AZURE_IOT_JSON_WRITER json_writer;
  UINT buffer_length;
  
  if (handle == NX_NULL)
  {
      return(NX_NOT_SUCCESSFUL);
  }

  /* Create a telemetry message packet. */
  status = nx_azure_iot_hub_client_telemetry_message_create(iothub_client_ptr,
                                                            &packet_ptr, NX_WAIT_FOREVER);
  if(status != NX_AZURE_IOT_SUCCESS) {
     return(NX_NOT_SUCCESSFUL);
  }

  /* Set telemetry component name. */
  status = nx_azure_iot_hub_client_telemetry_component_set(packet_ptr, 
                                                              handle -> component_name_ptr,
                                                              (USHORT)handle -> component_name_length,
                                                              NX_WAIT_FOREVER);
  
  if(status == NX_AZURE_IOT_SUCCESS) {
    status = nx_azure_iot_json_writer_with_buffer_init(&json_writer, scratch_buffer, sizeof(scratch_buffer));
  }

  /* Begin Json object. */
  if(status == NX_AZURE_IOT_SUCCESS) {
    status = nx_azure_iot_json_writer_append_begin_object (&json_writer);
  }

  if(status == NX_AZURE_IOT_SUCCESS) {
    status = append_telemetry_acc_values(handle, &json_writer);
  }

  if(status == NX_AZURE_IOT_SUCCESS) {
    status = append_telemetry_gyro_values(handle, &json_writer);
  }

  if(status == NX_AZURE_IOT_SUCCESS) {
    status = append_telemetry_mag_values(handle, &json_writer);
  }

  if(status == NX_AZURE_IOT_SUCCESS) {
    status = append_telemetry_temperature_value(handle, &json_writer);
  }

  if(status == NX_AZURE_IOT_SUCCESS) {
    status = append_telemetry_humidity_value(handle, &json_writer);
  }

  if(status == NX_AZURE_IOT_SUCCESS) {
    status = append_telemetry_pressure_value(handle, &json_writer);
  }

  if(status == NX_AZURE_IOT_SUCCESS) {
    status = append_telemetry_button_counter_value(handle, &json_writer);
  }

  /* End Json object. */
  if(status == NX_AZURE_IOT_SUCCESS) {
    status= nx_azure_iot_json_writer_append_end_object(&json_writer);
  }

  /*Send telemetry message packet. */
  if(status == NX_AZURE_IOT_SUCCESS) {
    buffer_length = nx_azure_iot_json_writer_get_bytes_used(&json_writer);
    status = nx_azure_iot_hub_client_telemetry_send(iothub_client_ptr, packet_ptr,
                                                        (UCHAR *)scratch_buffer, buffer_length, NX_WAIT_FOREVER);
    if (status != NX_AZURE_IOT_SUCCESS)
    {
        printf("Telemetry message send failed!: error code = 0x%08x\r\n", status);
    } else {
      on_telemetry_sent(handle);
      printf("Std Component %.*s Telemetry message send: %.*s.\r\n", handle -> component_name_length,
         handle -> component_name_ptr, buffer_length, scratch_buffer);
    }
  }

  if (status != NX_AZURE_IOT_SUCCESS)
  {
    nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
  }

  return(status);
}

UINT std_component_process_property_update(STD_COMPONENT *handle,
                                                     NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr,
                                                     UCHAR *component_name_ptr, UINT component_name_length,
                                                     NX_AZURE_IOT_JSON_READER *json_reader_ptr, UINT version)
{
UINT status;

    if (handle == NX_NULL)
    {
      return (NX_NOT_SUCCESSFUL);
    }

    if (handle->component_name_length != component_name_length ||
        strncmp((CHAR *)handle->component_name_ptr, (CHAR *)component_name_ptr, component_name_length) != 0)
    {
      return (NX_NOT_SUCCESSFUL);
    }

    status = process_acc_fullscale_property_update(handle, iothub_client_ptr, json_reader_ptr, version);

    if(status != NX_AZURE_IOT_SUCCESS) {
      status = process_gyro_fullscale_property_update(handle, iothub_client_ptr, json_reader_ptr, version);
    }

    if(status != NX_AZURE_IOT_SUCCESS) {
      status = process_telemetry_interval_property_update(handle, iothub_client_ptr, json_reader_ptr, version);
    }

    if(status != NX_AZURE_IOT_SUCCESS) {
      status = process_led_control_property_update(handle, iothub_client_ptr, json_reader_ptr, version);
    }

    if(status == NX_AZURE_IOT_SUCCESS) {
      nx_azure_iot_json_reader_next_token(json_reader_ptr);
    }

    return (status);
}

VOID std_component_on_button_pushed(STD_COMPONENT *handle) {
  if (handle != NX_NULL)
    {
      handle->ButtonCounter++;
    }
}
