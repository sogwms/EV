/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "ev_rc.h"

#define DBG_SECTION_NAME  "ev_rc"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

static int start(ev_object_t obj)
{
    LOG_I("rc start");

    return RT_EOK;
}

static int stop(ev_object_t obj)
{
    LOG_I("rc stop");

    return RT_EOK;
}

static void up_notify(struct ev_rc *rc)
{
    struct msg_motion mmotion;
    rc->ops->get_rc_info(&mmotion);
    EV_PUBLISH(rc, EV_TOPIC_RC, &mmotion);
}

int ev_rc_install(ev_rc_t rc)
{
    if (ev_object_install(&rc->parent, EV_OBJ_RC) != RT_EOK)
    {
        return RT_ERROR;
    }
    rc->parent.start = start;
    rc->parent.stop = stop;
    /* ! only after then can you do something 
         that involves the parent class
         such as subscribe and publish.*/

    rc->up_notify = up_notify;

    return RT_EOK;
}
