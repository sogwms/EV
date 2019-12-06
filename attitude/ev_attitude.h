/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#ifndef __EV_ATTITUDE_H__
#define __EV_ATTITUDE_H__

#include <ev.h>

/* operations that need to be implemented by sub-class*/
struct ev_attitude_ops
{
    void (*compute)(const struct msg_imu *mimu, struct msg_rpy *mrpy);
};

struct ev_attitude
{
    struct ev_object parent;
    const struct ev_attitude_ops *ops;
    // ...
};

typedef struct ev_attitude *ev_attitude_t;

int ev_attitude_install(ev_attitude_t attitude);

#endif /* __EV_ATTITUDE_H__ */
