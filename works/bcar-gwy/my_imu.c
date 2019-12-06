/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "my_imu.h"
#include <sensor_inven_mpu6xxx.h>

static struct mpu6xxx_device *g_mpudev = RT_NULL;
static rt_int32_t g_sample_time = 5;

#define MPU6XXX_DEVICE_NAME  "i2c1"

static int get_imu_data(struct msg_imu *imu_data)
{
    struct mpu6xxx_3axes accel, gyro;

    EV_CHECK_EOK_RVL(mpu6xxx_get_accel(g_mpudev, &accel));
    EV_CHECK_EOK_RVL(mpu6xxx_get_gyro(g_mpudev, &gyro));
    imu_data->acc_x = accel.x;
    imu_data->acc_y = -accel.y;
    imu_data->acc_z = accel.z;
    imu_data->gyro_x = -gyro.x;
    imu_data->gyro_y = gyro.y;
    imu_data->gyro_z = -(gyro.z - 9.0f);
    imu_data->mag_x = 0;
    imu_data->mag_y = 0;
    imu_data->mag_z = 0;

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
    g_mpudev = mpu6xxx_init(MPU6XXX_DEVICE_NAME, RT_NULL);

    imu_obj.ops = &_ops;

    return ev_imu_install(&imu_obj, thread_name, thread_stack_size, thread_priority);
}
