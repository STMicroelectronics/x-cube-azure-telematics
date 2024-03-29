/**
  ******************************************************************************
  * @file    plf_cellular_config.h
  * @author  GPM Application Team
  * @brief   Includes cellular configuration
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the ST_LICENSE file
  * in the root directory of this software component.
  * If no ST_LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef PLF_CELLULAR_CONFIG_H
#define PLF_CELLULAR_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/** @addtogroup PLF_CELLULAR_CONFIG_Constants
  * @{
  */

/** @note Cellular config parameters */
/* values used for sim slot interface defines :                                  */
/*  CA_SIM_REMOVABLE_SLOT         SIM inserted in a card reader                  */
/*  CA_SIM_EXTERNAL_MODEM_SLOT    SIM soldered external to the Modem             */
/*  CA_SIM_INTERNAL_MODEM_SLOT    SIM soldered internal to the Modem             */
/*  CA_SIM_NON_EXISTING_SLOT      Not applicable / non existing sim slot         */

#define PLF_CELLULAR_SIM_SLOT_NB         (1U)

#define PLF_CELLULAR_SIM_INDEX_0         (CA_SIM_REMOVABLE_SLOT)           /*!< SIM SLOT 0 interface */
#define PLF_CELLULAR_SIM_INDEX_1         (CA_SIM_NON_EXISTING_SLOT)        /*!< SIM SLOT 1 interface */
#define PLF_CELLULAR_SIM_INDEX_2         (CA_SIM_NON_EXISTING_SLOT)        /*!< SIM SLOT 2 interface */
#define PLF_CELLULAR_SIM_INDEX_3         (CA_SIM_NON_EXISTING_SLOT)        /*!< SIM SLOT 3 interface */

/* SIM PIN Code */
#define PLF_CELLULAR_SIM_PINCODE         ((uint8_t *)"")       /*!< SET PIN CODE HERE (for example "1234")
                                                                    if no PIN code, use an empty string "" */

#define PLF_CELLULAR_DNS_SERVER_IP_ADDR  ((uint8_t*)"8.8.8.8") /*!< DNS server IP address configuration value.
                                                                    the last byte is the LSB e.g 8.8.4.4 */

#define PLF_CELLULAR_APN_USER_DEFINED    (false)               /*!< APN is defined by user */
#define PLF_CELLULAR_APN                 ((uint8_t*)"")        /*!< APN */
#define PLF_CELLULAR_CID                 (1U)                  /*!< CID - possible values are [1-9] */
#define PLF_CELLULAR_USERNAME            ((uint8_t*)"")        /*!< User name  ( "": No Authentication) */
#define PLF_CELLULAR_PASSWORD            ((uint8_t*)"")        /*!< Password   ( "": No Authentication) */
#define PLF_CELLULAR_TARGET_STATE        (2U)                  /*!< Modem target state
                                                                    0: modem off
                                                                    1: SIM only
                                                                    2: Full data transfer enabled (default) */

#define PLF_CELLULAR_ATTACHMENT_TIMEOUT  (180000U)             /*!< Attachment timeout in ms (3 minutes) */

#define PLF_CELLULAR_NFMC_ACTIVATION     (0U)                  /*!< NFMC activation
                                                                    0: NFMC disabled (default)
                                                                    1: NFMC enabled           */

#define PLF_CELLULAR_NFMC_TEMPO1         (60000U)              /*!< NFMC value 1 */
#define PLF_CELLULAR_NFMC_TEMPO2         (120000U)             /*!< NFMC value 2 */
#define PLF_CELLULAR_NFMC_TEMPO3         (240000U)             /*!< NFMC value 3 */
#define PLF_CELLULAR_NFMC_TEMPO4         (480000U)             /*!< NFMC value 4 */
#define PLF_CELLULAR_NFMC_TEMPO5         (960000U)             /*!< NFMC value 5 */
#define PLF_CELLULAR_NFMC_TEMPO6         (1920000U)            /*!< NFMC value 6 */
#define PLF_CELLULAR_NFMC_TEMPO7         (3840000U)            /*!< NFMC value 6 */

#define PLF_CELLULAR_NETWORK_REG_MODE     (CA_NTW_REGISTRATION_AUTO) /*!< Network Register Mode set to automatic see
                                                                          cellular_control_api.h for other values */
#define PLF_CELLULAR_OPERATOR_NAME_FORMAT (CA_OPERATOR_NAME_FORMAT_NOT_PRESENT) /*!<
                                                                    Operator Name format set to not present
                                                                    see cellular_control_api.h for other values */
#define PLF_CELLULAR_OPERATOR_NAME        ((uint8_t*)"00101")
#define PLF_CELLULAR_ACT_PRESENT          (CA_ACT_NOT_PRESENT)  /*!< Access techno not set
                                                                     see cellular_control_api.h for other values */
#define PLF_CELLULAR_ACCESS_TECHNO        (CA_ACT_E_UTRAN)      /*!< Below some values:
                                                                     CA_ACT_GSM          Value for GSM
                                                                     CA_ACT_E_UTRAN      Value for LTE Cat.M1
                                                                     CA_ACT_E_UTRAN_NBS1 Value for LTE Cat.NB1
                                                                     see cellular_control_api.h for other values */

#define PLF_CELLULAR_LP_INACTIVITY_TIMEOUT        (1000U)      /*!< Low power mode entry timeout in ms */

/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* PLF_CELLULAR_CONFIG_H */
