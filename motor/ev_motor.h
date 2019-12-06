/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#ifndef __EV_MOTOR_H__
#define __EV_MOTOR_H__

#include <ev.h>

/* operations that need to be implemented by sub-class*/
struct ev_motor_ops
{
    void (*lock_motors)(int boolvalue);
    void (*set_speed)(const struct msg_motor *mmotor);
};

struct ev_motor
{
    struct ev_object parent;
    const struct ev_motor_ops *ops;
    // ...
};

typedef struct ev_motor *ev_motor_t;

int ev_motor_install(ev_motor_t motor);

#endif /* __EV_MOTOR_H__ */
