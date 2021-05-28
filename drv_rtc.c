/*
 * Copyright (c) 2019-2020, redoc
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-11-19     redoc        the first version
 */
 
#include <board.h>
#include "drv_rtc.h"
#include "string.h"
#include "drv_ds1337.h"

#define DRV_DEBUG
#define LOG_TAG "drv.rtc"
#include <drv_log.h>

#ifdef RT_USING_RTC

static struct rt_device rtc_dev;
static time_t rtc_time;

static rt_err_t rtc_control(rt_device_t dev, int cmd, void *args)
{
    time_t *time;
    struct tm time_temp;

    RT_ASSERT(dev != RT_NULL);
    memset(&time_temp, 0, sizeof(struct tm));

    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
        time = (time_t *) args;
        *time = rtc_time;
        break;
    case RT_DEVICE_CTRL_RTC_SET_TIME:
    {
        time = (time_t *) args;
        ds1337_write(time);
        break;
    }
    }

    return RT_EOK;
}

static void rtc_thread_entry(void* param)
{
    while(1)
    {
        rt_thread_mdelay(500); 
        rtc_time = ds1337_read();
    }
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops soft_rtc_ops = 
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    rtc_control
};
#endif

int rt_hw_rtc_init(void)
{
    rt_thread_t thread_rtc = RT_NULL;
    static rt_bool_t init_ok = RT_FALSE;

    if (init_ok)
    {
        return 0;
    }
    /* make sure only one 'rtc' device */
    RT_ASSERT(!rt_device_find("rtc"));

    i2c_ds1337_init();
	
    rtc_dev.type    = RT_Device_Class_RTC;

    /* register rtc device */
#ifdef RT_USING_DEVICE_OPS
    soft_rtc_dev.ops     = &soft_rtc_ops;
#else
    rtc_dev.init    = RT_NULL;
    rtc_dev.open    = RT_NULL;
    rtc_dev.close   = RT_NULL;
    rtc_dev.read    = RT_NULL;
    rtc_dev.write   = RT_NULL;
    rtc_dev.control = rtc_control;
#endif

    /* no private */
    rtc_dev.user_data = RT_NULL;

    rt_device_register(&rtc_dev, "rtc", RT_DEVICE_FLAG_RDWR);
    
    thread_rtc = rt_thread_create(  "rtc", 
                                    rtc_thread_entry, 
                                    RT_NULL, 
                                    512,
                                    29, 
                                    500);
    if(thread_rtc == RT_NULL)
    {
        return RT_ERROR; 
    }
    rt_thread_startup(thread_rtc);

    init_ok = RT_TRUE;

    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_rtc_init);

#endif /* RT_USING_RTC */
