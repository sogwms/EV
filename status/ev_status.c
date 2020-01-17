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
    case EV_TOPIC_SIGNAL:
        if (msg != RT_NULL) 
        {
            struct msg_signal *msignal = (struct msg_signal *)msg;

            switch (msignal->info)
            {
            case EV_SIGNAL_LOW_POWER:
                status->sys.bl_low_power = RT_TRUE;
                break;
            case EV_SIGNAL_NORMAL_POWER:
                status->sys.bl_low_power = RT_FALSE;
                break;
            case EV_SIGNAL_IMBALANCE:
                status->sys.bl_balance = RT_FALSE;
                break;
            case EV_SIGNAL_BALANCE:
                status->sys.bl_balance = RT_TRUE;
                break;
            default:
                break;
            }
            if (rt_memcmp(&status->sys, &status->sys_shadow, sizeof(struct msg_sys_status)) != 0)
            {
                if (status->sys.bl_low_power || !status->sys.bl_balance)
                {
                    status->sys.bl_fault = RT_TRUE;
                }
                else
                {
                    status->sys.bl_fault = RT_FALSE;
                }

                if (status->sys.bl_fault)   
                    status->sys.bl_running = RT_FALSE;
                else
                    status->sys.bl_running = RT_TRUE;
                
                EV_PUBLISH(&status, EV_TOPIC_SYS_STATUS, &status->sys);
                rt_memcpy(&status->sys_shadow, &status->sys, sizeof(struct msg_sys_status));
            }
        }
        break;
    default:
        break;
    }
}

static int start(ev_object_t obj)
{
    LOG_I("Start");

    obj->pssc.notify = pssc_notify;

    return RT_EOK;
}

static int stop(ev_object_t obj)
{
    LOG_I("Stop");

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
    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(status, EV_TOPIC_SIGNAL));

    LOG_I("Installed");

    return RT_EOK;
}
