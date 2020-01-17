/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "ev_motor_speed.h"

#define DBG_SECTION_NAME  "ev_motor_speed"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

static void pssc_notify(pss_client_t pssc, int topic, void *msg)
{
    struct ev_motor_speed *motor_speed = (struct ev_motor_speed *)pssc;

    if (msg == RT_NULL) return;

    if (IS_SAME_TOPIC(topic, EV_TOPIC_REQUEST_MOTOR_SPEED))
    {
        struct msg_motor *mmotor = (struct msg_motor *)msg;
        motor_speed->ops->get_motor_speed(mmotor);
        EV_PUBLISH(motor_speed, EV_TOPIC_MOTOR_SPEED, mmotor);
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

static void up_notify(struct ev_motor_speed *motor_speed)
{
    struct msg_motor mmotor;
    motor_speed->ops->get_motor_speed(&mmotor);
    EV_PUBLISH(motor_speed, EV_TOPIC_MOTOR_SPEED, &mmotor);
}

int ev_motor_speed_install(ev_motor_speed_t motor_speed)
{
    EV_CHECK_EOK_RVL(ev_object_install(&motor_speed->parent, EV_OBJ_MOTOR_INFO));
    motor_speed->parent.start = start;
    motor_speed->parent.stop = stop;
    /* ! only after then can you do something 
         that involves the parent class
         such as subscribe and publish.*/

    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(motor_speed, EV_TOPIC_REQUEST_MOTOR_SPEED));
    motor_speed->up_notify = up_notify;

    LOG_I("Installed");

    return RT_EOK;
}
