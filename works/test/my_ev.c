/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-16     sogwms       The first version
 */

#include <ev_monitor_bc.h>
#include <my_ind_led.h>
#include <my_imu.h>
#include <my_attitude.h>
#include <my_motor.h>
#include <my_motor_speed.h>
#include <my_power.h>

#include <ev.h>
#include <ev_status.h>
#include <ev_cu_bcar.h>

struct my_vehicle
{
    struct ev parent;
    struct ev_status status;
};

struct my_vehicle mv;

int my_ev_startup(void)
{
    ev_init((ev_t)&mv);

    ev_status_install(&mv.status);
    ev_monitor_bc_install();
    // ev_cu_bcar_install(179, 0.85, 
    //                     1.90, 0.012,
    //                     0.5, 0);
    // my_ind_led_install("mled", 512, 20);
    my_imu_install(5, "mimu", 1024, 19);
    my_attitude_install(0.005);
    my_motor_speed_install(5, EV_SCALE, EV_SCALE, "mmsp", 512, 18);
    my_motor_install(0.8);
    my_power_install(0, "mpower", 1024, 20);

    return ev_startup((ev_t)&mv);
}

INIT_APP_EXPORT(my_ev_startup);
