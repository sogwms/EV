/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include <rtdevice.h>
#include "my_motor.h"
#include <board.h>

#define DBG_SECTION_NAME  "my_motor"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

#define ABS(data) (data > 0 ? data : -data)

#define PWM_PERIOD              50*1000

#define DEFAULT_MAX_OUTPUT_RATIO  0.9f

#define MOTOR1_PWM_DEV_NAME     "pwm4"
#define MOTOR2_PWM_DEV_NAME     "pwm4"

#define MOTOR1_PWM_CH           4
#define MOTOR2_PWM_CH           3

#define MOTOR1_PIN_A            GET_PIN(C, 2)
#define MOTOR1_PIN_B            GET_PIN(C, 3)
#define MOTOR2_PIN_A            GET_PIN(C, 1)
#define MOTOR2_PIN_B            GET_PIN(C, 0)

static struct rt_device_pwm *motor1_pwmdev = RT_NULL;
static struct rt_device_pwm *motor2_pwmdev = RT_NULL;
static float g_ratio_max_output = DEFAULT_MAX_OUTPUT_RATIO;

static void set_speed(const struct msg_motor *mmotor)
{
    rt_uint32_t m1_pluse, m2_pluse;

    m1_pluse = (PWM_PERIOD * g_ratio_max_output) * ((float)ABS(mmotor->m1) / EV_SCALE);
    m2_pluse = (PWM_PERIOD * g_ratio_max_output) * ((float)ABS(mmotor->m2) / EV_SCALE);

    m1_pluse += 300;
    m2_pluse += 300;

    if (mmotor->m1 > 0)
    {
        rt_pin_write(MOTOR1_PIN_A, PIN_HIGH);
        rt_pin_write(MOTOR1_PIN_B, PIN_LOW);
    }
    else
    {
        rt_pin_write(MOTOR1_PIN_A, PIN_LOW);
        rt_pin_write(MOTOR1_PIN_B, PIN_HIGH);
    }
    
    if (mmotor->m2 > 0)
    {
        rt_pin_write(MOTOR2_PIN_A, PIN_HIGH);
        rt_pin_write(MOTOR2_PIN_B, PIN_LOW);
    }
    else
    {
        rt_pin_write(MOTOR2_PIN_A, PIN_LOW);
        rt_pin_write(MOTOR2_PIN_B, PIN_HIGH);
    } 
    
    rt_pwm_set(motor1_pwmdev, MOTOR1_PWM_CH, PWM_PERIOD, m1_pluse);
    rt_pwm_set(motor2_pwmdev, MOTOR2_PWM_CH, PWM_PERIOD, m2_pluse);

    // LOG_I("set_mortors:%d %d", m1_pluse, m2_pluse);
}

void lock_motors(int boolvalue)
{
    static int islocked = RT_TRUE;

    if (boolvalue && islocked)
        return;

    if (boolvalue)
    {
        islocked = RT_TRUE;
        rt_pwm_disable(motor1_pwmdev, MOTOR1_PWM_CH);
        rt_pwm_disable(motor2_pwmdev, MOTOR2_PWM_CH);
        LOG_I("lock motors");
    }
    else
    {
        islocked = RT_FALSE;
        rt_pwm_enable(motor1_pwmdev, MOTOR1_PWM_CH);
        rt_pwm_enable(motor2_pwmdev, MOTOR2_PWM_CH);
        LOG_I("unlock motors");
    }
}

static const struct ev_motor_ops _ops = {
    .set_speed = set_speed,
    .lock_motors = lock_motors,
};

static struct ev_motor motor_obj;

int my_motor_install(float ratio)
{
    if (ratio > 0.01f)
        g_ratio_max_output = ratio;

    rt_pin_mode(MOTOR1_PIN_A, PIN_MODE_OUTPUT);
    rt_pin_mode(MOTOR1_PIN_B, PIN_MODE_OUTPUT);
    rt_pin_mode(MOTOR2_PIN_A, PIN_MODE_OUTPUT);
    rt_pin_mode(MOTOR2_PIN_B, PIN_MODE_OUTPUT);

    motor1_pwmdev = (struct rt_device_pwm *)rt_device_find(MOTOR1_PWM_DEV_NAME);
    motor2_pwmdev = (struct rt_device_pwm *)rt_device_find(MOTOR2_PWM_DEV_NAME);

    lock_motors(RT_FALSE);

    motor_obj.ops = &_ops;

    return ev_motor_install(&motor_obj);
}
