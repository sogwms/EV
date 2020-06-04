/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "ev_monitor_bc.h"

#define DBG_SECTION_NAME  "ev_monitor_bc"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

#define IMBALANCE_ANGLE     45.0f
#define BALANCE_ANGLE       3.0f

struct ev_monitor_bc
{
    struct ev_object parent;
    void (*test_balance)(struct ev_monitor_bc *monitor_bc, struct msg_rpy *mrpy);
    void (*test_power)(struct ev_monitor_bc *monitor_bc, struct msg_power *mpower);
};

typedef struct ev_monitor_bc *ev_monitor_bc_t;

static struct ev_monitor_bc g_monitor_bc = {0};

static void __check_imbalance(struct ev_monitor_bc *monitor_bc, struct msg_rpy *mrpy);
static void __check_balance(struct ev_monitor_bc *monitor_bc, struct msg_rpy *mrpy);
static void __check_low_power(struct ev_monitor_bc *monitor_bc, struct msg_power *mpower);
static void __check_normal_power(struct ev_monitor_bc *monitor_bc, struct msg_power *mpower);

static void __cis(struct ev_monitor_bc *monitor_bc, int isLock, int signal)
{
    struct msg_lock_motor mlmotor;
    struct msg_signal msignal;
    mlmotor.bool_value = isLock;
    msignal.info = signal;
    EV_PUBLISH(monitor_bc, EV_TOPIC_LOCK_MOTOR, &mlmotor);
    EV_PUBLISH(monitor_bc, EV_TOPIC_SIGNAL, &msignal);
}

static void __check_imbalance(struct ev_monitor_bc *monitor_bc, struct msg_rpy *mrpy)
{
    if (mrpy->pitch > IMBALANCE_ANGLE || mrpy->pitch < -IMBALANCE_ANGLE)
    {
        LOG_I("Imbalance");

        monitor_bc->test_balance = __check_balance;
        __cis(monitor_bc, RT_TRUE, EV_SIGNAL_IMBALANCE);
    }
}
static void __check_balance(struct ev_monitor_bc *monitor_bc, struct msg_rpy *mrpy)
{
    if (mrpy->pitch > -BALANCE_ANGLE && mrpy->pitch < BALANCE_ANGLE)
    {
        LOG_I("Balance");

        monitor_bc->test_balance = __check_imbalance;
        __cis(monitor_bc, RT_FALSE, EV_SIGNAL_BALANCE);
    }
}

static void __check_low_power(struct ev_monitor_bc *monitor_bc, struct msg_power *mpower)
{
    if (mpower->current_power <= mpower->alert_power)
    {
        struct msg_signal msignal;

        LOG_I("Low power");

        monitor_bc->test_power = __check_normal_power;
        msignal.info = EV_SIGNAL_LOW_POWER;
        EV_PUBLISH(monitor_bc, EV_TOPIC_SIGNAL, &msignal);
    }
}
static void __check_normal_power(struct ev_monitor_bc *monitor_bc, struct msg_power *mpower)
{
    if (mpower->current_power > mpower->alert_power)
    {
        struct msg_signal msignal;

        LOG_I("Normal power");

        monitor_bc->test_power = __check_low_power;
        msignal.info = EV_SIGNAL_NORMAL_POWER;
        EV_PUBLISH(monitor_bc, EV_TOPIC_SIGNAL, &msignal);
    }
}

static void pssc_notify(pss_client_t pssc, int topic, void *msg)
{
    struct ev_monitor_bc *monitor_bc = (struct ev_monitor_bc *)pssc;

    if (msg == RT_NULL) return;

    if (IS_SAME_TOPIC(topic, EV_TOPIC_RPY))
    {
        monitor_bc->test_balance(monitor_bc, (struct msg_rpy *)msg);
    }
    else if (IS_SAME_TOPIC(topic, EV_TOPIC_POWER))
    {
        monitor_bc->test_power(monitor_bc, (struct msg_power *)msg);
    }
}

static int start(ev_object_t obj)
{
    obj->pssc.notify = pssc_notify;

    LOG_I("Start");

    return RT_EOK;
}

static int stop(ev_object_t obj)
{
    obj->pssc.notify = RT_NULL;

    LOG_I("Stop");

    return RT_EOK;
}

int ev_monitor_bc_install(void)
{
    ev_monitor_bc_t monitor_bc = &g_monitor_bc;

    if (ev_object_install(&monitor_bc->parent, EV_OBJ_MONITOR) != RT_EOK)
    {
        return RT_ERROR;
    }
    monitor_bc->parent.start = start;
    monitor_bc->parent.stop = stop;
    /* ! only after then can you do something 
         that involves the parent class
         such as subscribe and publish.*/

    monitor_bc->test_balance = __check_balance;
    monitor_bc->test_power = __check_low_power;

    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(monitor_bc, EV_TOPIC_RPY));
    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(monitor_bc, EV_TOPIC_POWER));

    LOG_I("Installed");

    return RT_EOK;
}
