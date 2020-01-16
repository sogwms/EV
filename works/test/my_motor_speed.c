/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-16     sogwms       The first version
 */

#include "my_motor_speed.h"
#include "my_motor.h"

#define DBG_SECTION_NAME  "my_motor_speed"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

#define DEFAULT_SAMPLE_TIME     5

static struct ev_motor_speed motor_speed_obj;

static rt_thread_t g_thread = RT_NULL;
static rt_int32_t  g_sample_time = DEFAULT_SAMPLE_TIME;
static rt_uint32_t g_maximum_speed[2];
static struct msg_motor g_mmotor = {0};

static void thread_entry(void *param)
{
    rt_int32_t e1, e2;
    struct msg_motor tmp_motor;

    while (1)
    {
        rt_thread_mdelay(g_sample_time);
        my_motor_get_sp(&tmp_motor);

        e1 = tmp_motor.m1;
        e2 = tmp_motor.m2;

        e1 = e1 * ((float)EV_SCALE / g_maximum_speed[0]);
        e2 = e2 * ((float)EV_SCALE / g_maximum_speed[1]);
        if (e1 > EV_SCALE)
          e1 = EV_SCALE;
        else if (e1 < -EV_SCALE)
          e1 = -EV_SCALE;
        if (e2 > EV_SCALE)
          e2 = EV_SCALE;
        else if (e2 < -EV_SCALE)
          e2 = -EV_SCALE;

        g_mmotor.m1 = e1;
        g_mmotor.m2 = e2;
        motor_speed_obj.up_notify(&motor_speed_obj);
    }
}

static void get_motor_speed(struct msg_motor *mmotor)
{
    mmotor->m1 = g_mmotor.m1;
    mmotor->m2 = g_mmotor.m2;
}

static const struct ev_motor_speed_ops _ops = {
    .get_motor_speed = get_motor_speed,
};

int my_motor_speed_install(int sample_time_ms, rt_uint32_t m1_maximum_speed, rt_uint32_t m2_maximum_speed, char *thread_name, rt_uint32_t stack_size, rt_uint8_t thread_priority)
{
    if (sample_time_ms != 0)
    {
        g_sample_time = sample_time_ms;
    }
    g_maximum_speed[0] = m1_maximum_speed;
    g_maximum_speed[1] = m2_maximum_speed;
    
    motor_speed_obj.ops = &_ops;
    EV_CHECK_EOK_RVL(ev_motor_speed_install(&motor_speed_obj));

    g_thread = rt_thread_create(thread_name, thread_entry, RT_NULL, stack_size, thread_priority, 20);
    if (g_thread == RT_NULL)
    {
        LOG_E("Failed to create thread");
        return RT_ERROR;
    }
    rt_thread_startup(g_thread);

    return RT_EOK;
}
