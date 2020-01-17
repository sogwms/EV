/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 * 2019-01-16     sogwms       Add orderly startup support
 */

#include "ev.h"

#define DBG_SECTION_NAME  "ev"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

const int g_levels_table[] = EV_OBJ_LEVELS;
static ev_t global_ev = RT_NULL;

int ev_object_install(ev_object_t obj, int type)
{
    RT_ASSERT(global_ev != RT_NULL);

    obj->type = (enum ev_obj_type)type;
    pss_init_client(&obj->pssc);

    rt_list_insert_after(&global_ev->components_head, &obj->list);

    return RT_EOK;
}

enum {
    _OBJ_METHOD_START,
    _OBJ_METHOD_STOP,
};

static int __traverse_object_method(ev_t ev, int method)
{
    rt_list_t *node;
    struct ev_object *obj;

    for (int ix = 0; ix < sizeof(g_levels_table)/sizeof(g_levels_table[0]); ix++)
    {
        rt_list_for_each(node, &ev->components_head)
        {
            obj = rt_list_entry(node, struct ev_object, list);

            if (g_levels_table[ix] != obj->type) 
                continue;

            if (method == _OBJ_METHOD_START)
            {
                if (obj->flag & EV_FLAG_ACTIVATED)
                    return RT_ERROR;

                obj->flag |= EV_FLAG_ACTIVATED;

                if (obj->start(obj) != RT_EOK) 
                    return RT_ERROR; 

            }
            else if (method == _OBJ_METHOD_STOP)
            {
                if (!(obj->flag & EV_FLAG_ACTIVATED))
                    return RT_ERROR; 

                obj->flag &= (~EV_FLAG_ACTIVATED);

                if (obj->stop(obj) != RT_EOK)
                    return RT_ERROR; 
            }
        }
    }

    return RT_EOK;
}

int ev_startup(ev_t ev)
{
    int rvl;

    LOG_D("Starting...");
    rvl = __traverse_object_method(ev, _OBJ_METHOD_START);
    if (rvl == RT_EOK)
        LOG_D("OK!");
    else
        LOG_D("FAILED!");

    return rvl;
}

int ev_stop(ev_t ev)
{
    int rvl;

    LOG_D("Stopping...");
    rvl = __traverse_object_method(ev, _OBJ_METHOD_STOP);
    if (rvl == RT_EOK)
        LOG_D("OK!");
    else
        LOG_D("FAILED!");

    return rvl;
}

int ev_init(ev_t ev)
{
    RT_ASSERT(ev != RT_NULL);

    global_ev = ev;

    rt_list_init(&ev->components_head);
    ev->startup = ev_startup;
    ev->stop = ev_stop;
    
    return RT_EOK;
}
