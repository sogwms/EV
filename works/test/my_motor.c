/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-16     sogwms       The first version
 */

#include <rtdevice.h>
#include "my_motor.h"
#include <board.h>

#define DBG_SECTION_NAME  "my_motor"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

static struct msg_motor g_sp;

void my_motor_get_sp(struct msg_motor *mmotor)
{
    rt_memcpy(mmotor, &g_sp, sizeof(struct msg_motor));
}

static void set_speed(const struct msg_motor *mmotor)
{
    rt_int32_t m1, m2;

    m1 = mmotor->m1;
    m2 = mmotor->m2;

    g_sp.m1 = m1;
    g_sp.m2 = m2;

    LOG_I("set_mortors:%d %d", m1, m2);
}

void lock_motors(int boolvalue)
{
    static int islocked = RT_TRUE;

    if (boolvalue && islocked)
        return;

    if (boolvalue)
    {
        islocked = RT_TRUE;
        LOG_I("lock motors");
    }
    else
    {
        islocked = RT_FALSE;
        LOG_I("unlock motors");
    }
}

static const struct ev_motor_ops _ops = {
    .set_speed = set_speed,
    .lock_motors = lock_motors,
};

static struct ev_motor motor_obj;

int my_motor_install(float ratio)
{
    lock_motors(RT_TRUE);

    motor_obj.ops = &_ops;

    return ev_motor_install(&motor_obj);
}
