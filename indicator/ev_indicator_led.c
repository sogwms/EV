/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "ev_indicator_led.h"

#define DBG_SECTION_NAME  "ev_indicator_led"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

// thread
#define THREAD_TIMESLICE            10

enum {
    INDEX_SHOW_RUNNING = 0,
    INDEX_SHOW_FAULT,
    INDEX_SHOW_STOP,
};

static const int g_show_table[][2] = {
    {66, 930},
    {1600, 228},
    {500, 500},
};

static void thread_entry(void *param)
{
    struct ev_indicator_led *ind_led = (struct ev_indicator_led *)param;

    while(1)
    {
        ind_led->ops->open_led(RT_FALSE);
        rt_thread_mdelay(g_show_table[ind_led->index][0]);
        ind_led->ops->open_led(RT_TRUE);
        rt_thread_mdelay(g_show_table[ind_led->index][1]);
    }
}

static void pssc_notify(pss_client_t pssc, int topic, void *msg)
{
    // TODO

    struct ev_indicator_led *ind_led = (struct ev_indicator_led *)pssc;

    if (msg == RT_NULL) return;

    if (IS_SAME_TOPIC(topic, EV_TOPIC_SYS_STATUS))
    {
        struct msg_sys_status *sys_status = (struct msg_sys_status *)msg;
        
        if (sys_status->bl_fault)
        {
            ind_led->index = INDEX_SHOW_FAULT;
            LOG_I("show fault");
        }
        else
        {
            if (sys_status->bl_running)
            {
                ind_led->index = INDEX_SHOW_RUNNING;
                LOG_I("show running");
            }
            else
            {
                ind_led->index = INDEX_SHOW_STOP;
                LOG_I("show stop");
            }
        }
    }
}

static int start(ev_object_t obj)
{
    struct ev_indicator_led *ind_led = (struct ev_indicator_led *)obj;

    LOG_I("Start");

    obj->pssc.notify = pssc_notify;

    return rt_thread_resume(ind_led->thread);
}

static int stop(ev_object_t obj)
{
    struct ev_indicator_led *ind_led = (struct ev_indicator_led *)obj;

    LOG_I("Stop");

    obj->pssc.notify = RT_NULL;

    return rt_thread_suspend(ind_led->thread);
}

int ev_indicator_led_install(ev_indicator_led_t ind_led, char *thread_name, rt_uint32_t thread_stack_size, rt_uint8_t thread_priority)
{
    // TODO
    if (ev_object_install(&ind_led->parent, EV_OBJ_INDICATOR) != RT_EOK)
    {
        return RT_ERROR;
    }
    ind_led->parent.start = start;
    ind_led->parent.stop = stop;
    /* ! only after then can you do something 
         that involves the parent class
         such as subscribe and publish.*/
    ind_led->index = 0;

    rt_thread_t thread = rt_thread_create(thread_name,
                    thread_entry, ind_led,
                    thread_stack_size,
                    thread_priority, THREAD_TIMESLICE);
    if (thread == RT_NULL)
    {
        LOG_E("Failed to create thread.");
        return RT_ERROR;
    }
    ind_led->thread = thread;
    rt_enter_critical();
    rt_thread_startup(thread);
    rt_thread_suspend(thread);
    rt_exit_critical();

    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(ind_led, EV_TOPIC_SYS_STATUS));

    LOG_I("Installed");

    return RT_EOK;
}
