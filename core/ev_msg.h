/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#ifndef __EV_MSG_H__
#define __EV_MSG_H__

#define IS_SAME_TOPIC(topica, topicb)  (topica == topicb)

#define DEF_EV_TOPICS(mytopics)  \
    enum ev_topic                               \
    {                                           \
        EV_TOPIC_IMU = 0,                       \
        EV_TOPIC_RPY,                           \
        EV_TOPIC_MOTION,                        \
        EV_TOPIC_POWER,                         \
        EV_TOPIC_SET_MOTOR_SPEED,               \
        EV_TOPIC_LOCK_MOTOR,                    \
        EV_TOPIC_RC,                            \
        EV_TOPIC_SYS_STATE,                     \
        EV_TOPIC_MOTOR_SPEED,                   \
        EV_TOPIC_REQUEST_MOTOR_SPEED,           \
                                                \
        mytopics                                \
        EV_TOTAL_TOPICS                         \
    };

#include <mytopics.h>

/**
 * unit: mg, mdps, mGauss
 */
struct msg_imu
{
    long acc_x;
    long acc_y;
    long acc_z;
    long gyro_x;
    long gyro_y;
    long gyro_z;
    long mag_x;
    long mag_y;
    long mag_z;
};

struct msg_rpy
{
    float roll;
    float pitch;
    float yaw;
};

/**
 * 放大100倍后的值. 如 current_power = 720 表示 7.20v
 */
struct msg_power
{
    int current_power;
    int total_power;
    int alert_power;
};

#define EV_SCALE    10000

/**
 * 右手坐标系，以‘头’方向为X轴正方向, Z轴朝向地面
 * 旋转正方向，遵循右手法则
 * !归一化处理 (EV_SCALE)
 */
struct msg_motion
{
    int velocity_x;     /* 在参考坐标系下 */
    int velocity_y;
    int velocity_z;
    int rotation_x;     /* 在载体坐标系下 */
    int rotation_y;
    int rotation_z;
};

/**
 * 从x轴向始，逆时针依次编号为 m1,m2,...
 * !归一化处理 (EV_SCALE)
 */
struct msg_motor
{
    int m1;
    int m2;
    int m3;
    int m4;
};

struct msg_lock_motor
{
    int bool_value;     // TRUE <--> lock;  FALSE <--> unlock
};

enum ev_sys_state{
    EV_STATE_RUNNING,
    EV_STATE_SLEEP,
    EV_STATE_FAULT,
};

struct msg_sys_state
{
    int state;
};

#endif /* __EV_MSG_H__ */
