/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "ev_power.h"

#define DBG_SECTION_NAME  "ev_power"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

static int start(ev_object_t obj)
{
    LOG_I("power start");

    return RT_EOK;
}

static int stop(ev_object_t obj)
{
    LOG_I("power stop");

    return RT_EOK;
}

static void up_notify(struct ev_power *power)
{
    power->ops->get_current_power(&power->info.current_power);
    EV_PUBLISH(power, EV_TOPIC_POWER, &power->info);
}

int ev_power_install(ev_power_t power, int total_power, int alert_power)
{
    // TODO
    if (ev_object_install(&power->parent, EV_OBJ_POWER) != RT_EOK)
    {
        return RT_ERROR;
    }
    power->parent.start = start;
    power->parent.stop = stop;
    /* ! only after then can you do something 
         that involves the parent class
         such as subscribe and publish.*/
    power->info.total_power = total_power;
    power->info.alert_power = alert_power;
    power->up_notify = up_notify;

    return RT_EOK;
}
