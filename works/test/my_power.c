/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-16     sogwms       The first version
 */

#include "my_power.h"
#include <rtdevice.h>

#define TOTAL_POWER         1260
#define ALERT_POWER         1110

#define POWER_RADIUS        (TOTAL_POWER - ALERT_POWER)

#define AVERAGE_WINDOW_SIZE 8
#define DEFAULT_SAMPLE_TIME 200

static rt_int32_t g_sample_time = DEFAULT_SAMPLE_TIME;
static int g_average_power = 0;
static rt_thread_t g_thread = RT_NULL;

static void get_current_power(int *current_power)
{
    *current_power = g_average_power;
}

static struct ev_power_ops _ops = {
    .get_current_power = get_current_power,
};

static struct ev_power power_obj;

static int __calculate_power(void)
{
    static int real_power = ALERT_POWER;
    static int inc = -1;

    real_power += inc;
    if (real_power > (ALERT_POWER + POWER_RADIUS))
        inc = -1;
    if (real_power < (ALERT_POWER - POWER_RADIUS))
        inc = 1;

    return real_power;
}

static void thread_entry(void *param)
{
    int buffer[AVERAGE_WINDOW_SIZE];
    uint32_t sigma = 0;
    uint8_t index = 0;

    rt_thread_mdelay(g_sample_time);
    buffer[0] = __calculate_power();
    for (int i = 1; i < AVERAGE_WINDOW_SIZE; i++)
    {
        buffer[i] = buffer[0];
    }

    for(;;)
    {
        rt_thread_mdelay(g_sample_time);
        index %= AVERAGE_WINDOW_SIZE;
        buffer[index++] = __calculate_power();

        sigma = 0;
        for (int i = 0; i < AVERAGE_WINDOW_SIZE; i++)
        {
            sigma += buffer[i];
        }
        g_average_power = (sigma / AVERAGE_WINDOW_SIZE);

        power_obj.up_notify(&power_obj);
    }
}

int my_power_install(int sample_time_ms, char *thread_name, rt_uint32_t stack_size, rt_uint8_t thread_priority)
{
    if (sample_time_ms != 0)
        g_sample_time = sample_time_ms;

    g_thread = rt_thread_create(thread_name, thread_entry, RT_NULL, stack_size, thread_priority, 20);
    if (g_thread == RT_NULL)
    {
        rt_kprintf("Can't create thread for power\n");
        return RT_ERROR;
    }
    rt_thread_startup(g_thread);

    power_obj.ops = &_ops;
    EV_CHECK_EOK_RVL(ev_power_install(&power_obj, TOTAL_POWER, ALERT_POWER))

    return RT_EOK;
}
