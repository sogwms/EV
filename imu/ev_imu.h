/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#ifndef __EV_IMU_H__
#define __EV_IMU_H__

#include <ev.h>
#include <ev_msg.h>

struct ev_imu_ops
{
    int (*get_imu_data)(struct msg_imu *imu_data);
};

struct ev_imu
{
    struct ev_object parent;
    const struct ev_imu_ops *ops;

    rt_thread_t thread;
};

typedef struct ev_imu *ev_imu_t;

int ev_imu_install(ev_imu_t imu, char *thread_name, rt_uint32_t thread_stack_size, rt_uint8_t thread_priority);

#endif /* __EV_IMU_H__ */
