## Nx_Azure_IoT Application Description

This application provides an example of Azure RTOS NetX/NetXDuo Azure IoT cloud usage. 

It shows how to use Azure IoT Cloud add-on using MQTT protocol in an encrypted mode supporting TLS v1.2.

The main entry function tx_application_define() is called by ThreadX during kernel start. At this stage, all NetX resources are created.

 + A <i>NX_PACKET_POOL</i>is allocated

 + A <i>NX_IP</i> instance using that pool is initialized

 + The <i>ARP</i>, <i>ICMP</i>, <i>UDP</i> and <i>TCP</i> protocols are enabled for the <i>NX_IP</i> instance

 + A <i>DHCP client is created.</i>

The application then creates 2 threads :

 + **AppMainThread** : created with the <i>TX_AUTO_START</i> flag to start automatically.

 + **AppSampleThread** : created with the <i>TX_DONT_START</i> flag to be started later.

The **AppMainThread** starts and perform the following actions:

  + Starts the DHCP client

  + Waits for the IP address resolution

  + Resumes the **AppSampleThread**


The **AppSampleThread**, once started:

  + creates a dns_client with USER_DNS_ADDRESS used as DNS server.

  + retrieves the current time and date with SNTP

  + runs sample IoT Cloud client that connects to Azure IoT Hub.
 
  + sample client will publish messages with Temperature information from time to time. 

####  Expected success behavior

 + The board IP address is printed on the Serial Terminal
 
 + SNTP time and date server is printed
 
 + A message about sensors measurements is printed

#### Error behaviors

+ The red LED is toggling to indicate any error that has occurred.

#### Assumptions if any
None

#### Known limitations

+ 

#### ThreadX usage hints

 - ThreadX uses the Systick as time base, thus it is mandatory that the HAL uses a separate time base through the TIM IPs.

 - ThreadX is configured with 100 ticks/sec by default. This should be taken into account when using delays or timeouts at application. It is always possible to reconfigure it in the "tx_user.h", the "TX_TIMER_TICKS_PER_SECOND" define, but this should be reflected in "tx_initialize_low_level.s" file too.

 - ThreadX is disabling all interrupts during kernel start-up to avoid any unexpected behavior, therefore all system related calls (HAL, BSP) should be done either at the beginning of the application or inside the thread entry functions.

 - ThreadX offers the "tx_application_define()" function, that is automatically called by the tx_kernel_enter() API.
   It is highly recommended to use it to create all applications ThreadX related resources (threads, semaphores, memory pools...) but it should not in any way contain a system API call (HAL or BSP).

 - Using dynamic memory allocation requires to apply some changes to the linker file.

   ThreadX needs to pass a pointer to the first free memory location in RAM to the tx_application_define() function, using the "first_unused_memory" argument.
   This require changes in the linker files to expose this memory location.

    + For EWARM add the following section into the .icf file:
     ```
     place in RAM_region    { last section FREE_MEM };
     ```
    + For MDK-ARM:
    ```
    either define the RW_IRAM1 region in the ".sct" file
    or modify the line below in "tx_low_level_initilize.s to match the memory region being used
        LDR r1, =|Image$$RW_IRAM1$$ZI$$Limit|
    ```
    + For STM32CubeIDE add the following section into the .ld file:
    ```
    ._threadx_heap :
      {
         . = ALIGN(8);
         __RAM_segment_used_end__ = .;
         . = . + 64K;
         . = ALIGN(8);
       } >RAM_D1 AT> RAM_D1
    ``` 
    
       The simplest way to provide memory for ThreadX is to define a new section, see ._threadx_heap above.
       In the example above the ThreadX heap size is set to 64KBytes.
       The ._threadx_heap must be located between the .bss and the ._user_heap_stack sections in the linker script.
       Caution: Make sure that ThreadX does not need more than the provided heap memory (64KBytes in this example).
       Read more in STM32CubeIDE User Guide, chapter: "Linker script".

    + The "tx_initialize_low_level.s" should be also modified to enable the "USE_DYNAMIC_MEMORY_ALLOCATION" flag.

### Directory Contents

    AZURE_RTOS
        \App                         Azure RTOS start file
    Core                             
        |Inc                         
        \Src                         main.c file, initialisation files
    EWARM                            Project files for IAR EWARM
    NetXDuo                          
        |App                         network related sources
        |  app_azure_iot.c           main source for Azure IoT connection
        |  app_azure_iot_config.h    configuration file for Azure IoT cloud
        |  app_netxduo.c             source file for network protocol stack startup
        |  device_identity.c         can contain device X509 certificate and private key (if used).
        |  ctrl_component.c          IoT plug and play board control file (start/stop/reboot)
        |  std_component.c           IoT plug and play component for board sensors telemetry
        \Target                      network sources (dependent from MCU)
    STM32CubeIDE                     project files for STM32CubeIDE


### Keywords

RTOS, Network, ThreadX, NetXDuo, Azure IoT, MQTT, DNS, TLS, WIFI, MXCHIP


### Hardware and Software environment

  - This example runs on B-U585I-IOT02A board with STM32U585xx chip with either WiFi or cellular modem.
  
  - To use B-U585I-IOT02A on-board WiFi module (MXCHIP:EMW3080) the C pre-processor define USE_WIFI must be defined
    in IDE project configuration with value 1 (USE_WIFI=1). USE_CELLULAR=1 must not be defined.
    
    The WiFi module is used with following configuration:

    + MXCHIP Firmware 2.1.11

    + SPI mode used as interface

    + Bypass mode (TCP-IP stack is handled by NetXDuo, not by Wi-Fi module)

    If the EMW3080B MXCHIP Wi-Fi module firmware is not the required version 2.1.1, an update package is available at <https://www.st.com/en/development-tools/x-wifi-emw3080b.html>

    To achieve this, follow the instructions given at the above link, using the EMW3080updateV2.1.11RevC.bin flasher under the V2.1.11/SPI folder.

  - To use B-U585I-IOT02A with cellular modem, the C pre-processor define USE_CELLULAR=1 must be defined in IDE project configuration. USE_WIFI=1 must be removed.  (USE_WIFI=1 and USE_CELLULAR=1 are mutually exclusive. They must not be defined at same time.)  

    **EWARM:**  
    Right click on the project's name: **Nx_Azure_IoT** -> **Options** -> **C/C++ Compiler**  
    In section **Defined symbols**  
    replace **USE_WIFI=1** by **USE_CELLULAR=1**  

    **STM32CubeIDE:**  
    Right click on the project's name: **Nx_Azure_IoT** -> **Project properties** -> **C/C++ Build** -> **Settings** -> **MCU GCC Compiler**  
    In section **Preprocessor**  
    replace **USE_WIFI=1** by **USE_CELLULAR=1**  

    The supported cellular modems are the Quectel BG96 on ST MB1329 extension board and the EVK-TYPE1SC extension board with Murata chip.
    
    They connect to STMOD+2 connector CN2 on B-U585I-IOT02A board.
    
    When building with the USE_CELLULAR flag, the default modem is the BG96. To choose the TYPE1SC modem, please replace the BG96 driver's source and include files by the TYPE1SC driver's one.

    **EWARM:**  
    In the project source tree:  
    right click on **Drivers/BSP/Components/cellular/BG96** -> **Options** -> Check **Exclude from build**  
    right click on **Drivers/BSP/Components/cellular/TYPE1SC** -> **Options** -> Uncheck **Exclude from build**  

    Right click on the project's name **Nx_Azure_IoT** -> **Options** -> **C/C++ Compiler**  
    In section **Additional include directories**  
    replace **\$PROJ_DIR\$/../../../../../../Drivers/BSP/Components/cellular/BG96/Inc**  
    by **\$PROJ_DIR\$/../../../../../../Drivers/BSP/Components/cellular/TYPE1SC/Inc**  


    **STM32CubeIDE:**  
    Right click on the project's name -> **Nx_Azure_IoT** -> **Project properties** -> **C/C++ Build** -> **Settings** -> **MCU GCC Compiler**  
    In section **Include Path**  
    replace **../../../../../../../Drivers/BSP/Components/cellular/BG96/Inc**  
    by **../../../../../../../Drivers/BSP/Components/cellular/TYPE1SC/Inc**

    In the project source tree:  
    right click on **Drivers/BSP/Components/cellular/BG96** -> **Properties** -> Check **Exclude resource from build**  
    right click on **Drivers/BSP/Components/cellular/TYPE1SC** -> **Properties** -> Uncheck **Exclude resource from build**  


  - This application has been tested with B-U585I-IOT02A (MB1551-U585AI) boards Revision: RevC and can be adapted to other supported device and development board.

  - This application uses USART1 to display logs. The serial terminal configuration is as follows:
      - BaudRate = 115200 baud
      - Word Length = 8 Bits
      - Stop Bit = 1
      - Parity = None
      - Flow control = None


###  How to use it ?

To run the application :

 - The compilers require a short path to sources.

   Install the sources at root of `C:\` drive with a short directory name.

   If needed it is possible to use DOS `subst` command to create a virtual drive to shorten the path.
   
   Example: in a DOS command prompt:
   
       subst X: C:\STM32CubeExpansion_Cloud_AZURE_V2.1.0
   
   then use project file from `X:` drive

 - Open the application project file in development environment. (ex: EWARM/Project.eww)

 - In project configuration, in C pre-processor defines:
   - If using WiFi connection, USE_WIFI=1 must be defined. USE_CELLULAR=1 must be removed.
   - If using cellular modem, USE_CELLULAR=1 must be defined. USE_WIFI=1 must be removed.
   
 - Edit the file <code>NetXDuo/App/app_azure_iot_config.h</code>:

    - If using WiFi, configure the WiFi Settings (WIFI_SSID, WIFI_PASSWORD)

    - If using cellular modem, the network connectivity settings are detected in the SIM card inserted in the modem board SIM slot.

    - Azure IoT configuration:

        - An Azure IoT device connects to an Azure IoT hub. The device must be created in the IoT hub with an unique ID and credentials
          (shared access key or X509 certificate).
          For the first connection (provisioning), the device can be created staticaly by the user in an IoT Hub (using Azure CLI or Azure web portal)
          or the device can be created and assigned to an IoT Hub by a Device Provisioning Service (DPS).
          DPS is useful when managing a large number of devices and IoT hubs. It assigns automatically the new devices to IoT hubs in its scope.
        
        - an easy way to create everything is to use [Azure IoT Central](https://azure.microsoft.com/en-us/services/iot-central/). IoT Central provides IoT Hub, DPS and devices bundled in an "application".  
          See [howto-create-iot-central-application](https://docs.microsoft.com/en-us/azure/iot-central/core/howto-create-iot-central-application) to create an IoT Central "application".  
          See [add-a-device](https://docs.microsoft.com/en-us/azure/iot-central/core/howto-manage-devices-individually#add-a-device) to add a device. Choose SAS key for authentication.  
          See [get-device-connection-information](https://docs.microsoft.com/en-us/azure/iot-central/core/howto-manage-devices-individually#get-device-connection-information) to get its credentials.

        - if using Device Provisioning Service (DPS) (for example created with IoT Central), in file `app_azure_iot_config.h` :
            - uncomment ENABLE_DPS
            - configure ENDPOINT (DPS server address). If created with IoT Central it is `global.azure-devices-provisioning.net`.
            - ID_SCOPE: your DPS devices "group" (or "organisation").
            - REGISTRATION_ID: device name used for registration (must be unique)

        - if not using DPS (not using IoT Central): set DEVICE_ID, HOST_NAME in `app_azure_iot_config.h`:
            -  DEVICE_ID is the IoT device unique identifier created in the IoT Hub.
            -  HOST_NAME is the address of the IoT Hub server (created with Azure web portal or Azure CLI).

        - if the device was created with Shared Access Signature (symmetric key) for Azure IoT device authentication:
            -  in `app_azure_iot_config.h` set DEVICE_SYMMETRIC_KEY. It is the device SAS key (can be obtained during device creation in Azure portal or CLI).  
          
          __Note:__ symmetric keys are not recommended for production devices. It is strongly suggested to use X-CUBE-AZURE product with TFM and STSAFE security instead.

        - if using X509 certificate for IoT device authentication: 
            - uncomment `#define USE_DEVICE_CERTIFICATE 1` in `app_azure_iot_config.h`
            - configure DEVICE_KEY_TYPE
            - in <code>NetXDuo/App/device_identity.c</code>, the DEVICE_CERT and DEVICE_PRIVATE_KEY
              defines must be configured with C language table initialization for the device certificate and private key.
              (Note: the device certificate and private key must be in X509 DER binary format).
            
            Ex:
            
                #define DEVICE_CERT   { \
                            0x30, 0x82, 0x02, 0x0e, 0x30, 0x82, 0x01, 0x77, 0xa0, 0x03, 0x02, 0x01, \
                            /* ... */ \
                            0xbd, 0xcf, 0x15, 0x9b, 0x3b, 0xd1, 0x2e, 0xe4, 0xdd, 0x17, 0x11, 0x96, \
                            0xf9, 0x54 \
                          }
            
            - to convert DER binary certificates and key to C format it is possible to use "xxd" tool with "-i" option (tool available in Git for Windows Bash shell) : 

                xxd.exe -i device_certificate.der

          __Note:__ this example with X509 certificate embedded in sources is not adapted for production devices. It is strongly suggested to use X-CUBE-AZURE product with TFM and STSAFE security instead.

 - Optional: by default the application uses DNS server IP address obtained from DHCP server answer. If the DHCP server doesn't provide DNS server address, it is possible to set fixed DNS server address in USER_DNS_ADDRESS in `app_netxduo.h` file.

 - Once all the parameters are configured in the sources, run a full build in the development environment.

 - Program the image into target memory.

 - Run the application.

 - For a better experience with Azure IoT Central Application, you can  use the following device template:

       Projects\B-U585I-IOT02A\Applications\NetXDuo\Nx_Azure_IoT\Config\AzureIotCentral\dtmi_stmicroelectronics_b_u585i_iot02a_standard_fw;3.json

   Please refer to [Azure IoT Central Application device template documentation](https://docs.microsoft.com/en-us/azure/iot-central/core/concepts-device-templates) for more information.