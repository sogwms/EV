/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "evt.h"
#if defined(EV_USING_MSH_CMD) && defined (FINSH_USING_MSH)

#include <ev.h>
#include <stdlib.h>

static void __report_motor_speed(int interval, int duration)
{
    struct msg_motor mmotor_speed = {0};
    int average[4] = {0};
    rt_kprintf("duration:%d\n", duration);
    for (int i = 0; i < duration/interval; i++)
    {
        rt_thread_mdelay(interval);
        pss_publish(EV_TOPIC_REQUEST_MOTOR_SPEED, &mmotor_speed);
        rt_kprintf("[t%d] m1:%d  m2:%d  m3:%d  m4:%d\n", i, mmotor_speed.m1, mmotor_speed.m2, mmotor_speed.m3, mmotor_speed.m4);
        average[0] = average[0] * 0.6 + mmotor_speed.m1 * 0.4;
        average[1] = average[1] * 0.6 + mmotor_speed.m2 * 0.4;
        average[2] = average[2] * 0.6 + mmotor_speed.m3 * 0.4;
        average[3] = average[3] * 0.6 + mmotor_speed.m4 * 0.4;
    }
    rt_kprintf("[!average] m1:%d m2:%d m3:%d m4:%d\n", average[0], average[1], average[2], average[3]);
}

static void __set_motor_speed(int m1, int m2, int m3, int m4)
{
    struct msg_motor mmotor_speed = {
        .m1 = m1,
        .m2 = m2,
        .m3 = m3,
        .m4 = m4,
    };

    pss_publish(EV_TOPIC_SET_MOTOR_SPEED, &mmotor_speed);
}

static void evt_motor(int argc, char *argv[])
{
    if (argc < 2)
    {
        rt_kprintf("Usage: evt_motor [OPTION]\n");
        rt_kprintf("Options:\n");
        rt_kprintf("    read <interval> <duration>\n");
        rt_kprintf("    set <m1> <m2> <m3> <m4> <duration>\n");
    }
    else if (argc < 3)
    {
        if (!rt_strcmp("check", argv[1]))
        {
            // ...
        }
    }
    else if (argc < 5)
    {
        if (!rt_strcmp("read", argv[1]))
        {
            int interval = atoi(argv[2]);
            int duration = atoi(argv[3]);
            __report_motor_speed(interval, duration);
        }
    }
    else if (argc < 8)
    {
        if (!rt_strcmp("set", argv[1]))
        {
            int m1 = atoi(argv[2]);
            int m2 = atoi(argv[3]);
            int m3 = atoi(argv[4]);
            int m4 = atoi(argv[5]);
            rt_uint32_t duration = atoi(argv[6]);

            rt_kprintf("set-motor-speed: %d %d %d %d\n", m1, m2, m3, m4);
            __set_motor_speed(m1, m2, m3, m4);
            __report_motor_speed(5, duration);
            __set_motor_speed(0, 0, 0, 0);
        }
    }
}
MSH_CMD_EXPORT(evt_motor, test motor);

#endif
