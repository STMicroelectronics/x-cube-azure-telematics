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

#ifndef DEVICE_IDENTITY_H
#define DEVICE_IDENTITY_H

#include "nx_api.h"

UINT device_identity_retrieve_credentials(const UCHAR **device_cert_ptr, UINT* device_cert_len, const UCHAR **device_private_key_ptr, UINT* device_private_key_len);

#endif /* DEVICE_IDENTITY_H */
