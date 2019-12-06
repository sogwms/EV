/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "ev_imu.h"

#define DBG_SECTION_NAME  "ev_imu"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

/* thread */
#define THREAD_TIMESLICE            10

static void thread_entry(void *param)
{
    ev_imu_t eimu = (ev_imu_t)param;
    struct msg_imu msg;

    for(;;)
    {
        if (eimu->ops->get_imu_data(&msg) == RT_EOK)
        {
            EV_PUBLISH(eimu, EV_TOPIC_IMU, &msg);
        }
    }
}

static int start(ev_object_t obj)
{
    struct ev_imu * ev_imu = (struct ev_imu *)obj;

    RT_ASSERT(ev_imu->thread != RT_NULL);

    LOG_I("imu start");

    return rt_thread_resume(ev_imu->thread);
}

static int stop(ev_object_t obj)
{
    struct ev_imu * ev_imu = (struct ev_imu *)obj;

    RT_ASSERT(ev_imu->thread != RT_NULL);

    LOG_I("imu stop");

    return rt_thread_suspend(ev_imu->thread);
}

int ev_imu_install(ev_imu_t imu, char *thread_name, rt_uint32_t thread_stack_size, rt_uint8_t thread_priority)
{
    if (ev_object_install(&imu->parent, EV_OBJ_IMU) != RT_EOK)
    {
        return RT_ERROR;
    }
    imu->parent.start = start;
    imu->parent.stop = stop;

    rt_thread_t thread = rt_thread_create(thread_name,
                    thread_entry, imu,
                    thread_stack_size,
                    thread_priority, THREAD_TIMESLICE);
    if (thread == RT_NULL)
    {
        LOG_E("Failed to create thread.");
        return RT_ERROR;
    }
    imu->thread = thread;
    rt_enter_critical();
    rt_thread_startup(thread);
    rt_thread_suspend(thread);
    rt_exit_critical();
    
    return RT_EOK;
}
