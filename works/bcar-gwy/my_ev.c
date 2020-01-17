/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "my_ev.h"
#include <ev_monitor_bc.h>
#include <my_ind_led.h>
#include <my_imu.h>
#include <my_attitude.h>
#include <my_motor.h>
#include <my_motor_speed.h>
#include <my_power.h>
#include <ev_cu_bcar.h>
#include <ps2.h>
#include "my_ev.h"

#define PS2_CS_PIN      12 // A12
#define PS2_CLK_PIN     11 // A11
#define PS2_DO_PIN      13 // A13
#define PS2_DI_PIN      14 // A14

struct my_vehicle mv;

void my_ev_startup(void)
{
    ev_init((ev_t)&mv);

    ev_status_install(&mv.status);
    ev_monitor_bc_install();
    ev_cu_bcar_install(179, 0.85, 
                        1.90, 0.012,
                        0.5, 0);
    my_ind_led_install("mled", 256, 20);
    my_imu_install(5, "mimu", 1024, 19);
    my_attitude_install(0.005);
    my_motor_speed_install(5, 520, 510, "mmsp", 256, 18);
    my_motor_install(0.8);
    my_power_install(0, "mpower", 512, 20);
    rc_ps2_install(PS2_CS_PIN,
                   PS2_CLK_PIN,
                   PS2_DO_PIN,
                   PS2_DI_PIN,
                   0.3, 0.5);

    ev_startup((ev_t)&mv);
}
