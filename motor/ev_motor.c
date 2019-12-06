/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "ev_motor.h"

#define DBG_SECTION_NAME  "ev_motor"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

static void pssc_notify(pss_client_t pssc, int topic, void *msg)
{

    struct ev_motor *motor = (struct ev_motor *)pssc;

    if (msg == RT_NULL) return;

    if (IS_SAME_TOPIC(topic, EV_TOPIC_SET_MOTOR_SPEED))
    {
        motor->ops->set_speed((struct msg_motor *)msg);
    }
    else if (IS_SAME_TOPIC(topic, EV_TOPIC_LOCK_MOTOR))
    {
        motor->ops->lock_motors(((struct msg_lock_motor *)msg)->bool_value);
    }
}

static int start(ev_object_t obj)
{
    LOG_I("motor start");

    obj->pssc.notify = pssc_notify;

    return RT_EOK;
}

static int stop(ev_object_t obj)
{
    LOG_I("motor stop");

    obj->pssc.notify = RT_NULL;

    return RT_EOK;
}

int ev_motor_install(ev_motor_t motor)
{
    // TODO
    if (ev_object_install(&motor->parent, EV_OBJ_MOTOR) != RT_EOK)
    {
        return RT_ERROR;
    }
    motor->parent.start = start;
    motor->parent.stop = stop;
    /* ! only after then can you do something 
         that involves the parent class
         such as subscribe and publish.*/

    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(motor, EV_TOPIC_SET_MOTOR_SPEED));
    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(motor, EV_TOPIC_LOCK_MOTOR));

    return RT_EOK;
}
