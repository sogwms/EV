/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#ifndef __EV_TEMPLATE_H__
#define __EV_TEMPLATE_H__

#include <ev.h>

/* operations that need to be implemented by sub-class*/
struct ev_indicator_led_ops
{
    void (*open_led)(int boolvalue);
};

struct ev_indicator_led
{
    struct ev_object parent;
    struct msg_sys_state state;
    rt_thread_t thread;

    uint8_t index;

    const struct ev_indicator_led_ops *ops;
};

typedef struct ev_indicator_led *ev_indicator_led_t;

int ev_indicator_led_install(ev_indicator_led_t ind_led, char *thread_name, rt_uint32_t thread_stack_size, rt_uint8_t thread_priority);

#endif /* __EV_TEMPLATE_H__ */
