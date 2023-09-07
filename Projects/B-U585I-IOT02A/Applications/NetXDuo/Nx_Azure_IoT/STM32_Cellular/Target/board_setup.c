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

#include <stdio.h>
#include <stdlib.h>

#include "plf_config.h"
#include "main.h"

int hardware_rand(void)
{
  /* Return the random number.  */
  uint32_t aRandom32bit;
  /* Initialize local port to a random value */
  if (HAL_RNG_GenerateRandomNumber(&hrng, &aRandom32bit) != HAL_OK)
  {
    aRandom32bit = (uint32_t)rand();
  }
  return aRandom32bit;
}

#if (USE_CELLULAR == 1)
#if defined(__ICCARM__)
/* New definition from EWARM V9, compatible with EWARM8 */
int iar_fputc(int ch);
#define PUTCHAR_PROTOTYPE int iar_fputc(int ch)
#elif defined ( __CC_ARM ) || defined(__ARMCC_VERSION)
/* ARM Compiler 5/6*/
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#elif defined(__GNUC__)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#endif /* __ICCARM__ */

#if (USE_PRINTF == 0)
#if (USE_TRACE_APPLICATION == 1)
#include "trace_interface.h"
size_t __write(int handle, const unsigned char *buf, size_t bufSize)
{
  /* Go through trace interface module to display traces */

#if defined(__ICCARM__)
  size_t idx;
  unsigned char const *pdata = buf;

  for (idx = 0; idx < bufSize; idx++)
  {
    iar_fputc((int)*pdata);
    pdata++;
  }
#endif /* __ICCARM__ */

  traceIF_uartPrintForce(DBG_CHAN_APPLICATION, (uint8_t *)buf, bufSize);
  return (bufSize);
}
#else /* USE_TRACE_APPLICATION == 0 */
size_t __write(int handle, const unsigned char *buf, size_t bufSize)
{
#if defined(__ICCARM__)
  size_t idx;
  unsigned char const *pdata = buf;

  for (idx = 0; idx < bufSize; idx++)
  {
    iar_fputc((int)*pdata);
    pdata++;
  }
#endif /* __ICCARM__ */

  UNUSED(handle);
  UNUSED(buf);
  return (bufSize);
}
#endif /* USE_TRACE_APPLICATION == 1 */
#else /* USE_PRINTF == 1 */

#if defined(__ICCARM__)
size_t __write(int file, unsigned char const *buf, size_t bufSize)
{
  size_t idx;
  unsigned char const *pdata = buf;

  for (idx = 0; idx < bufSize; idx++)
  {
    iar_fputc((int)*pdata);
    pdata++;
  }
  return bufSize;
}
#endif /* __ICCARM__ */

size_t __read(int handle, unsigned char *buf, size_t bufSize)
{

  /* Check for stdin      (only necessary if FILE descriptors are enabled) */
  if (handle != 0)
  {
    return -1;
  }

  if (HAL_UART_Receive(&TRACE_INTERFACE_UART_HANDLE, (uint8_t *)buf, bufSize, 0x10000000) != HAL_OK)
  {
    return -1;
  }
  return bufSize;
}
#endif /* USE_PRINTF == 1 */

/**
  * @brief  Retargets the C library printf function to the USART.
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of putchar here */
  /* e.g. write a character to the USART3 and Loop until the end of transmission */
  HAL_UART_Transmit(&TRACE_INTERFACE_UART_HANDLE, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

#endif /* USE_CELLULAR == 1 */
