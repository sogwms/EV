/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "ev_attitude.h"

#define DBG_SECTION_NAME  "ev_attitude"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

static void pssc_notify(pss_client_t pssc, int topic, void *msg)
{
    ev_attitude_t attitude = (ev_attitude_t)pssc;

    if (msg == RT_NULL) return;

    if (IS_SAME_TOPIC(topic, EV_TOPIC_IMU))
    {
        struct msg_imu *mimu = (struct msg_imu *)msg;
        struct msg_rpy mrpy;
        attitude->ops->compute(mimu, &mrpy);
        EV_PUBLISH(attitude, EV_TOPIC_RPY, &mrpy);
    }
}

static int start(ev_object_t obj)
{
    LOG_I("attitude start");

    obj->pssc.notify = pssc_notify;
    return RT_EOK;
}

static int stop(ev_object_t obj)
{
    LOG_I("attitude stop");

    obj->pssc.notify = RT_NULL;
    return RT_EOK;
}

int ev_attitude_install(ev_attitude_t attitude)
{
    if (ev_object_install(&attitude->parent, EV_OBJ_ATTITUDE) != RT_EOK)
    {
        return RT_ERROR;
    }
    attitude->parent.start = start;
    attitude->parent.stop = stop;
    /* ! only after then can you do something 
         that involves the parent class
         such as subscribe and publish.*/

    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(attitude, EV_TOPIC_IMU));

    return RT_EOK;
}
