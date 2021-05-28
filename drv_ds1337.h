/*
 * Copyright (c) 2019-2020, redoc
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-04     redoc        the first version
 */

#ifndef __DRV_DS1337_H
#define __DRV_DS1337_H

#include <rtthread.h>
#include "rtdevice.h"
#include <drv_common.h>
#include <time.h>

void ds1337_write(time_t *time);
time_t ds1337_read(void);
int i2c_ds1337_init(void);

#endif
