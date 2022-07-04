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
/* The following source code has been taken from 'sample_device_identity.c'             */
/* by Microsoft (https://github.com/azure-rtos) and modified by STMicroelectronics.     */
/* Main changes summary :                                                               */
/* - Modified to add an API instead of using extern variable declaration                */
/*                                                                                      */
/****************************************************************************************/

#include "nx_api.h"

/* Device certificate.  */
#ifndef DEVICE_CERT
#define DEVICE_CERT {0x00}
#endif /* DEVICE_CERT */

/* Device Private Key.  */
#ifndef DEVICE_PRIVATE_KEY
#define DEVICE_PRIVATE_KEY {0x00}
#endif /* DEVICE_PRIVATE_KEY */

static const UCHAR device_cert_ptr[] = DEVICE_CERT;
static const UINT device_cert_len = sizeof(device_cert_ptr);
static const UCHAR device_private_key_ptr[] = DEVICE_PRIVATE_KEY;
static const UINT device_private_key_len = sizeof(device_private_key_ptr);

UINT device_identity_retrieve_credentials(const UCHAR  **cert_ptr, UINT *cert_len, const UCHAR **private_key_ptr, UINT *private_key_len) 
{
  UINT status;
  if(cert_ptr != NULL && cert_len != NULL && private_key_ptr != NULL && private_key_len != NULL) 
  {
    *cert_ptr = device_cert_ptr;
    *cert_len = device_cert_len;
    *private_key_ptr = device_private_key_ptr;
    *private_key_len = device_private_key_len;

    status = NX_SUCCESS;
  }
  else
  {
    status = NX_INVALID_PARAMETERS;
  }
  
  return status;
}

