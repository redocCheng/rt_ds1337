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
#include "drv_ds1337.h"
#include "string.h"
#include "drv_soft_i2c.h"

#define DRV_DEBUG
#define LOG_TAG "drv.ds1337"
#include <drv_log.h>

#define DS1337_I2C_BUS_NAME      "i2c1"  		
#define DS1337_ADDR      		 0x68    	

/*******************************         sec min  hour week  DD   MM   YY  */	
const rt_uint8_t ds1337_wr_addr[7]   = {0x00,0x01,0x02,0x03,0x04,0x05,0x06};
const rt_uint8_t ds1337_wr_format[7] = {0x7f,0x7f,0x3f,0x07,0x3f,0x1f,0xff};

/*******************************  sec min  hour week  DD   MM   YY  */	
static rt_uint8_t time_buf[7] = {0};

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;     
static rt_bool_t initialized = RT_FALSE;               

static rt_err_t write_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t *data, rt_uint8_t len)
{
    rt_uint8_t buf[8];
    struct rt_i2c_msg msgs;

    buf[0] = reg; 
    rt_memcpy(buf+1, data, len);
	
    msgs.addr = DS1337_ADDR;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = len + 1;

    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

static rt_err_t read_regs(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t *buf, rt_uint8_t len)
{
    struct rt_i2c_msg msgs;

    msgs.addr = DS1337_ADDR;
    msgs.flags = RT_I2C_RD;
    msgs.buf = buf;
    msgs.len = len;

    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

static uint8_t dec2bcd(uint8_t dec)
{
    return (dec + (dec/10)*6);
}

static uint8_t bcd2dec(uint8_t bcd)
{
    return (bcd - (bcd >> 4) * 6);
}

void ds1337_write(time_t *time)
{
    struct tm *p_tm;
    struct tm tm_new;

    if (!initialized)
    {
        i2c_ds1337_init();
    }

    rt_enter_critical();
    p_tm = localtime(time);
    memcpy(&tm_new, p_tm, sizeof(struct tm));
    rt_exit_critical();

    time_buf[0] = dec2bcd(tm_new.tm_sec) & ds1337_wr_format[0];
    time_buf[1] = dec2bcd(tm_new.tm_min) & ds1337_wr_format[1];
    time_buf[2] = dec2bcd(tm_new.tm_hour) & ds1337_wr_format[2];
    time_buf[4] = dec2bcd(tm_new.tm_mday) & ds1337_wr_format[4];
    time_buf[5] = dec2bcd(tm_new.tm_mon) & ds1337_wr_format[5];
    time_buf[6] = dec2bcd(tm_new.tm_year) & ds1337_wr_format[6];

    write_reg(i2c_bus, ds1337_wr_addr[0], time_buf, 7);
}

time_t ds1337_read(void)
{
    time_t time;
    struct tm tm_new;

    if (!initialized)
    {
        i2c_ds1337_init();
    }

    write_reg(i2c_bus, ds1337_wr_addr[0], RT_NULL, 0);
    read_regs(i2c_bus, ds1337_wr_addr[0], time_buf, 7);

    tm_new.tm_sec  = bcd2dec(time_buf[0] & ds1337_wr_format[0]);
    tm_new.tm_min  = bcd2dec(time_buf[1] & ds1337_wr_format[1]);
    tm_new.tm_hour = bcd2dec(time_buf[2] & ds1337_wr_format[2]);
    tm_new.tm_mday = bcd2dec(time_buf[4] & ds1337_wr_format[4]);
    tm_new.tm_mon  = bcd2dec(time_buf[5] & ds1337_wr_format[5]);
    tm_new.tm_year = bcd2dec(time_buf[6] & ds1337_wr_format[6]);

    time = mktime(&tm_new);

    return time;
}

int i2c_ds1337_init(void)
{
    rt_err_t result = RT_EOK;

    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(DS1337_I2C_BUS_NAME);
    if (i2c_bus == RT_NULL)
    {
        result = -RT_ERROR;
        LOG_E("can't find %s device!\n",DS1337_I2C_BUS_NAME);
    }
    else
    {
        initialized = RT_TRUE;
    }

    return result;
}
