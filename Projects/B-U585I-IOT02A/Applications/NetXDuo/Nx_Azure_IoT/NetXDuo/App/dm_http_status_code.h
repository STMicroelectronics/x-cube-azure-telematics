/**
  ******************************************************************************
  * @file    dm_http_status_code.h
  * @author  GPM Application Team
  * @brief   Device Manager HTTP status code header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the ST_LICENSE file
  * in the root directory of this software component.
  * If no ST_LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef DM_HTTP_STATUS_CODE
#define DM_HTTP_STATUS_CODE

/* Device Manager HTTP status code declaration */
#define DM_HTTP_SUCCESS                                (200)
#define DM_HTTP_MULTIPLE_CHOICE                        (300)
#define DM_HTTP_BAD_REQUEST                            (400)
#define DM_HTTP_INTERNAL_SERVER_ERROR                  (500)

/* Helper to know is http status is an error */
#define DM_HTTP_CODE_IS_ERROR(status_code) ((status_code < DM_HTTP_SUCCESS) || (status_code >= DM_HTTP_MULTIPLE_CHOICE))

#endif /* DM_HTTP_STATUS_CODE */