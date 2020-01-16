/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-16     sogwms       The first version
 */

#include "my_imu.h"

#define GYRO_SCOPE      250

static rt_int32_t g_sample_time = 5;

struct _3axis
{
    long x;
    long y;
    long z;
};

static int get_imu_data(struct msg_imu *imu_data)
{
    static struct _3axis acc, gyro, mag;
    static uint32_t cnt = 0;

    cnt++;

    acc.x = 0;
    acc.y = 0;
    acc.z = 0;
    gyro.x = 0;

    gyro.y += cnt%1000? 0: 1;
    if (gyro.y > GYRO_SCOPE) 
        gyro.y = -GYRO_SCOPE;

    gyro.z = 0;
    mag.x = 0;
    mag.y = 0;
    mag.z = 0;

    imu_data->acc_x = acc.x;
    imu_data->acc_y = acc.y;
    imu_data->acc_z = acc.z;
    imu_data->gyro_x = gyro.x;
    imu_data->gyro_y = gyro.y;
    imu_data->gyro_z = gyro.z;
    imu_data->mag_x = mag.x;
    imu_data->mag_y = mag.y;
    imu_data->mag_z = mag.z;

    rt_thread_mdelay(g_sample_time);

    return RT_EOK;
}

static const struct ev_imu_ops _ops = {
    .get_imu_data = get_imu_data,
};

static struct ev_imu imu_obj;

int my_imu_install(int sample_time, char *thread_name, rt_uint32_t thread_stack_size, rt_uint8_t thread_priority)
{
    g_sample_time = sample_time;

    imu_obj.ops = &_ops;

    return ev_imu_install(&imu_obj, thread_name, thread_stack_size, thread_priority);
}
