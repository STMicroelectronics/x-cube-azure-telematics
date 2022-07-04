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

#ifndef STD_COMPONENT_H
#define STD_COMPONENT_H

#ifdef __cplusplus
extern   "C" {
#endif

#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_json_reader.h"
#include "nx_azure_iot_json_writer.h"
#include "nx_api.h"
  
/**
  * @brief  Accelerometer Full Scale structure definition
  */
typedef enum
{
  STD_COMP_ACC_FS_2G      = 2,
  STD_COMP_ACC_FS_4G      = 4,
  STD_COMP_ACC_FS_8G      = 8,
  STD_COMP_ACC_FS_16G     = 16
} STD_COMP_AccFullScaleTypeDef;

/**
  * @brief  Gyroscope Full Scale structure definition
  */
typedef enum
{
  STD_COMP_GYRO_FS_125_DPS      = 125,
  STD_COMP_GYRO_FS_250_DPS      = 250,
  STD_COMP_GYRO_FS_500_DPS      = 500,
  STD_COMP_GYRO_FS_1000_DPS     = 1000,
  STD_COMP_GYRO_FS_2000_DPS     = 2000
} STD_COMP_GyroFullScaleTypeDef;


/**
  * @brief  Standard Component definition
  */
typedef struct STD_COMPONENT_TAG
{
    /* Name of this component */
    UCHAR *component_name_ptr;

    /* Name length of this component */
    UINT component_name_length;

    /*************
     * Telemetry *
     *************/
    
    /* Current acceleration X/Y/Z */
    double Acc_X;
    double Acc_Y;
    double Acc_Z;
    
    /* Current gyroscope X/Y/Z  */
    double Gyro_X;
    double Gyro_Y;
    double Gyro_Z;
    
    /* Current magnetometer X/Y/Z  */
    double Mag_X;
    double Mag_Y;
    double Mag_Z;

    /* Current temperature of this thermostat component */
    double Temperature;
    
    /* Current humidity  */
    double Humidity;
    
    /* Current Pressure  */
    double Pressure;

    /* Current Button pushed counter between two telemetry  */
    INT ButtonCounter;
    
    /************
     * Property *
     ************/
  
    /* Transmission Interval */
    double CurrentTelemetryInterval;
    INT TelemetryIntervall_IpPeriodicalRate;
    
    /* Current accelerometer full scale */
    STD_COMP_AccFullScaleTypeDef currentAccFS;
    
    /* Current gyroscope full scale */
    STD_COMP_GyroFullScaleTypeDef currentGyroFS;
    
    /* Current control LED status */
    UINT CurrentLED_ControlStatus;

} STD_COMPONENT;

UINT std_component_init(STD_COMPONENT *handle,
                       UCHAR *component_name_ptr, UINT component_name_length);


UINT std_component_telemetry_send(STD_COMPONENT *handle, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr);


UINT std_component_process_property_update(STD_COMPONENT *handle,
                                                   NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr,
                                                   UCHAR *component_name_ptr, UINT component_name_length,
                                                   NX_AZURE_IOT_JSON_READER *json_reader_ptr, UINT version);

UINT std_component_read_sensor_values(STD_COMPONENT *handle);

UINT std_component_pnp_report_acc_fullscale_property(STD_COMPONENT *handle, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr);

UINT std_component_pnp_report_gyro_fullscale_property(STD_COMPONENT *handle, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr);

UINT std_component_pnp_report_telemetry_interval_property(STD_COMPONENT *handle, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr);

UINT std_component_pnp_report_control_led_property(STD_COMPONENT *handle, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr);

VOID std_component_on_button_pushed(STD_COMPONENT *handle);

#ifdef __cplusplus
}
#endif
#endif /* STD_COMPONENT_H */
