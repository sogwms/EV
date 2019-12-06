/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "my_ind_led.h"
#include <board.h>

#define LED_PIN GET_PIN(B, 14)

static void open_led(int boolvalue)
{
    rt_pin_write(LED_PIN, !boolvalue);
}

static struct ev_indicator_led_ops _ops = {
    .open_led = open_led,
};

static struct ev_indicator_led ind_led_obj;

int my_ind_led_install(char *name, rt_uint32_t size, rt_uint8_t priority)
{
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    ind_led_obj.ops = &_ops;
    return ev_indicator_led_install(&ind_led_obj, name, size, priority);
}
