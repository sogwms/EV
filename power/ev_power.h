/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#ifndef __EV_POWER_H__
#define __EV_POWER_H__

#include <ev.h>

/* operations that need to be implemented by sub-class*/
struct ev_power_ops
{
    void (*get_current_power)(int *current_power);
};

struct ev_power
{
    struct ev_object parent;
    const struct ev_power_ops *ops;
    struct msg_power info;
    void (*up_notify)(struct ev_power *power);
};

typedef struct ev_power *ev_power_t;

int ev_power_install(ev_power_t power, int total_power, int alert_power);

#endif /* __EV_POWER_H__ */
