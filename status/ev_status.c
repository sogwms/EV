/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "ev_status.h"

#define DBG_SECTION_NAME  "ev_status"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

static void pssc_notify(pss_client_t pssc, int topic, void *msg)
{
    ev_status_t status = (ev_status_t)pssc;

    switch (topic)
    {
    case EV_TOPIC_IMU:
        if (msg != RT_NULL) 
        {
            struct msg_imu *mimu = (struct msg_imu *)msg;
            rt_memcpy(&status->imu, mimu, sizeof(struct msg_imu));
        }
        break;
    case EV_TOPIC_RPY:
        if (msg != RT_NULL) 
        {
            struct msg_imu *mrpy = (struct msg_imu *)msg;
            rt_memcpy(&status->rpy, mrpy, sizeof(struct msg_rpy));
        }
        break;
    case EV_TOPIC_MOTION:
        if (msg != RT_NULL) 
        {
            struct msg_imu *mmotion = (struct msg_imu *)msg;
            rt_memcpy(&status->motion, mmotion, sizeof(struct msg_motion));
        }
        break;
    case EV_TOPIC_POWER:
        if (msg != RT_NULL) 
        {
            struct msg_imu *mpower = (struct msg_imu *)msg;
            rt_memcpy(&status->power, mpower, sizeof(struct msg_power));
        }
        break;
    default:
        break;
    }
}

static int start(ev_object_t obj)
{
    LOG_I("status start");

    obj->pssc.notify = pssc_notify;

    return RT_EOK;
}

static int stop(ev_object_t obj)
{
    LOG_I("status stop");

    obj->pssc.notify = RT_NULL;

    return RT_EOK;
}

int ev_status_install(ev_status_t status)
{
    rt_memset(status, 0, sizeof(struct ev_status));
    EV_CHECK_EOK_RVL(ev_object_install(&status->parent, EV_OBJ_STATUS));

    status->parent.start = start;
    status->parent.stop = stop;
    /* ! only after then can you do something 
         that involves the parent class
         such as subscribe and publish.*/

    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(status, EV_TOPIC_IMU));
    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(status, EV_TOPIC_RPY));
    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(status, EV_TOPIC_MOTION));
    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(status, EV_TOPIC_POWER));

    return RT_EOK;
}
