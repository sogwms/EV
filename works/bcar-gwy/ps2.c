/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "ps2.h"
#include <ev_rc.h>

#define DBG_SECTION_NAME  "ps2"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

#define THREAD_DELAY_TIME           30

#define THREAD_PRIORITY             ((RT_THREAD_PRIORITY_MAX / 3) + 3)
#define THREAD_STACK_SIZE           1024
#define THREAD_TIMESLICE            10

#define KEEP_TIME()                 _delay_us(16);

static uint8_t light_mode = PS2_NO_MODE;
static rt_thread_t g_thread = RT_NULL;

struct ps2_ctrl_data g_ctrldata;

static rt_base_t ps2_cs_pin; 
static rt_base_t ps2_clk_pin;
static rt_base_t ps2_do_pin; 
static rt_base_t ps2_di_pin;

static void hal_cs_high(void)
{
    rt_pin_write(ps2_cs_pin, PIN_HIGH);
}
static void hal_cs_low(void)
{
    rt_pin_write(ps2_cs_pin, PIN_LOW);
}
static void hal_clk_high(void)
{
    rt_pin_write(ps2_clk_pin, PIN_HIGH);
}
static void hal_clk_low(void)
{
    rt_pin_write(ps2_clk_pin, PIN_LOW);
}
static void hal_do_high(void)
{
    rt_pin_write(ps2_do_pin, PIN_HIGH);
}
static void hal_do_low(void)
{
    rt_pin_write(ps2_do_pin, PIN_LOW);
}
static int hal_read_di(void)
{
    return rt_pin_read(ps2_di_pin);
}

static void _delay_us(uint16_t us)
{
    for (int i = 0; i < us; i++)
    {
        for (int j = 0; j < 0x1F;)
            j++;
    }
}

static uint8_t _transfer(uint8_t data)
{
    uint8_t temp = 0;

    for (uint16_t i = 0x01; i < 0x0100; i <<= 1)
    {
        if (i & data)
            hal_do_high(); 
        else
            hal_do_low();

        hal_clk_high();
        KEEP_TIME();
        hal_clk_low();
        if (hal_read_di())
            temp = i | temp;
        KEEP_TIME();
        hal_clk_high();
    }
    
    return temp;
}

static void transfer(const uint8_t *pb_send, uint8_t *pb_recv, uint8_t len)
{
    hal_cs_low();
    _delay_us(16);
    for (uint8_t i = 0; i < len; i++)
    {
        pb_recv[i] = _transfer(pb_send[i]);
    }
    hal_cs_high();
    _delay_us(16);
}

int ps2_scan(ps2_ctrl_data_t pt)
{
    uint8_t temp[9] = {0};

    temp[0] = 0x01;
    temp[1] = 0x42;
    temp[3] = 0;
    temp[4] = 0;

    transfer(temp, temp, 9);
    
    pt->button = temp[3] | (temp[4] << 8);
    pt->right_stick_x = temp[5];
    pt->right_stick_y = temp[6];
    pt->left_stick_x = temp[7];
    pt->left_stick_y = temp[8];

    if (temp[2] == 0x5A)
    {
        light_mode = temp[1];
        return 1;
    }
    else
    {
        light_mode = PS2_NO_MODE;
    }

    return 0;
}

/**
 * @return PS2_GREEN_MDOE or PS2_RED_MDOE or other(no connect) 
 */
int ps2_read_light(void)
{
    return light_mode;
}

static struct msg_motion g_ps2_motion = {0};
static float g_scope_dir = 0.0f;
static float g_scope_speed = 0.0f;

static void get_control_info(struct msg_motion *mmotion)
{
    rt_memcpy(mmotion, &g_ps2_motion, sizeof(struct msg_motion));
}

static const struct ev_rc_ops _ops = {
    .get_rc_info = get_control_info,
};

struct ev_rc ev_rc = {
    .ops = &_ops,
};

static void ps2_thread_entry(void *param)
{
    while (1)
    {
        rt_thread_mdelay(THREAD_DELAY_TIME);   
        ps2_scan(&g_ctrldata);
        if (g_ctrldata.left_stick_x == 0xFF && g_ctrldata.left_stick_y == 0xFF && g_ctrldata.right_stick_x == 0xFF && g_ctrldata.right_stick_y == 0xFF)
        {
            g_ps2_motion.velocity_x = 0;
            g_ps2_motion.rotation_z = 0;
            LOG_E("analog mode?");
        }
        else 
        {
            g_ps2_motion.velocity_x = -(g_ctrldata.right_stick_y - 128) * (g_scope_speed * EV_SCALE) / 128;
            g_ps2_motion.rotation_z = (g_ctrldata.left_stick_x - 128) * (g_scope_dir * EV_SCALE) / 128;
        }
        
        ev_rc.up_notify(&ev_rc);
    }
}

int rc_ps2_install(rt_base_t cs_pin, rt_base_t clk_pin, rt_base_t do_pin, rt_base_t di_pin, float scope_dir, float scope_speed)
{
    ps2_cs_pin = cs_pin;
    ps2_clk_pin = clk_pin;
    ps2_do_pin = do_pin;
    ps2_di_pin = di_pin;
    g_scope_dir = scope_dir;
    g_scope_speed = scope_speed;

    rt_pin_mode(ps2_cs_pin,  PIN_MODE_OUTPUT);
    rt_pin_mode(ps2_clk_pin, PIN_MODE_OUTPUT);
    rt_pin_mode(ps2_do_pin,  PIN_MODE_OUTPUT);
    rt_pin_mode(ps2_di_pin,  PIN_MODE_INPUT);
    
    hal_cs_high();
    hal_clk_high();

    ev_rc_install(&ev_rc);

    g_thread = rt_thread_create("ps2",
                          ps2_thread_entry, RT_NULL,
                          THREAD_STACK_SIZE,
                          THREAD_PRIORITY, THREAD_TIMESLICE);

    if (g_thread != RT_NULL)
    {
        rt_thread_startup(g_thread);
    }
    else
    {
        LOG_E("Failed to create thread");
        return RT_ERROR;
    }

    return RT_EOK;
}
