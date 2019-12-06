/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#ifndef __MY_EV_H__
#define __MY_EV_H__

#include <ev.h>
#include <ev_status.h>

struct my_vehicle
{
    struct ev parent;
    struct ev_status status;
};

extern struct my_vehicle mv;

void my_ev_startup(void);

#endif 
