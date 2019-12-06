/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#ifndef __EV_RC_H__
#define __EV_RC_H__

#include <ev.h>

/* operations that need to be implemented by sub-class*/
struct ev_rc_ops
{
    void (*get_rc_info)(struct msg_motion *mmotion);
};

struct ev_rc
{
    struct ev_object parent;
    const struct ev_rc_ops *ops;
    
    void (*up_notify)(struct ev_rc *rc);
};

typedef struct ev_rc *ev_rc_t;

int ev_rc_install(ev_rc_t rc);

#endif /* __EV_RC_H__ */
