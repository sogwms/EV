/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-16     sogwms       The first version
 */

#include "my_attitude.h"
#include <ahrs.h>

#define DEFAULT_AHRS_DT 0.005f

static float g_ahrs_dt = DEFAULT_AHRS_DT;

static void compute(const struct msg_imu *mimu, struct msg_rpy *mrpy)
{
    float rpy[3];
    ahrs_9dof_update(mimu->gyro_x, mimu->gyro_y, mimu->gyro_z, 
        mimu->acc_x, mimu->acc_y, mimu->acc_z, 0.0f,0.0f,0.0f, g_ahrs_dt, rpy);
    mrpy->roll = rpy[0];
    mrpy->pitch = rpy[1];
    mrpy->yaw = rpy[2];
}

static const struct ev_attitude_ops _ops = {
    .compute = compute,
};

static struct ev_attitude attitude_obj;

int my_attitude_install(float ahrs_dt)
{
    g_ahrs_dt = ahrs_dt;
    attitude_obj.ops = &_ops;
    return ev_attitude_install(&attitude_obj);
}
