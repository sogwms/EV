/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#ifndef __EV_MOTOR_SPEED_H__
#define __EV_MOTOR_SPEED_H__

#include <ev.h>

/* operations that need to be implemented by sub-class*/
struct ev_motor_speed_ops
{
    void (*get_motor_speed)(struct msg_motor *mmotor);
};

struct ev_motor_speed
{
    struct ev_object parent;
    const struct ev_motor_speed_ops *ops;
    void (*up_notify)(struct ev_motor_speed *motor_speed);
};

typedef struct ev_motor_speed *ev_motor_speed_t;

int ev_motor_speed_install(ev_motor_speed_t motor_speed);

#endif /* __EV_MOTOR_SPEED_H__ */
