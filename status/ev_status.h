/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#ifndef __ECD_STATUS_H__
#define __ECD_STATUS_H__

#include <ev.h>

struct ev_status
{
    struct ev_object parent;
    
    struct msg_imu imu;
    struct msg_rpy rpy;
    struct msg_power power;
    struct msg_motion motion;
};

typedef struct ev_status *ev_status_t;

extern int ev_status_install(ev_status_t status);

#endif /* __ECD_STATUS_H__ */
