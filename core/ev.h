/**
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * @version 0.1.0
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 * 2019-01-16     sogwms       Add orderly startup support
 */

#ifndef __EV_H__
#define __EV_H__

#include <rtthread.h>
#include "pss.h"
    
#define DEFAULT_EV_OBJ_LEVELS {\
    EV_OBJ_IMU,         \
    EV_OBJ_MOTOR,\
    EV_OBJ_MOTOR_INFO,\
    EV_OBJ_POWER,\
    EV_OBJ_RC,\
    \
    EV_OBJ_ATTITUDE,\
    EV_OBJ_STATUS,\
    EV_OBJ_INDICATOR,\
    EV_OBJ_MONITOR,\
    \
    EV_OBJ_UNKNOWN,\
    EV_OBJ_CONTROL_UNIT\
}

#if defined(PKG_EV_ENABLING_CUSTOM)
    #include <my_ev_custom.h>
    #ifdef MY_EV_OBJ_LEVELS
        #define EV_OBJ_LEVELS MY_EV_OBJ_LEVELS
    #else
        #define EV_OBJ_LEVELS DEFAULT_EV_OBJ_LEVELS
    #endif
#else
    #define EV_OBJ_LEVELS DEFAULT_EV_OBJ_LEVELS
#endif

/* syntactic sugar */
#define EV_SUBSCRIBE(obj, topic)        pss_subscribe((pss_client_t)obj, topic)
#define EV_PUBLISH(obj, topic, msg)     pss_publish(topic, msg)
#define EV_CHECK_RVL(func, rvl)            if (func != rvl) { return RT_ERROR; }
#define EV_CHECK_EOK_RVL(func)             EV_CHECK_RVL(func, RT_EOK)

/**/
#define EV_FLAG_ACTIVATED        0x0001

enum ev_obj_type
{
    EV_OBJ_UNKNOWN,
    EV_OBJ_IMU,
    EV_OBJ_MOTOR,
    EV_OBJ_MOTOR_INFO,
    EV_OBJ_ATTITUDE,
    EV_OBJ_STATUS,
    EV_OBJ_CONTROL_UNIT,
    EV_OBJ_RC,
    EV_OBJ_MONITOR,
    EV_OBJ_INDICATOR,
    EV_OBJ_POWER,
};

typedef struct ev_object *ev_object_t;

struct ev_object
{
    struct pss_client pssc;
    enum ev_obj_type type;
    int flag;
    
    int (*start)(ev_object_t obj);
    int (*stop)(ev_object_t obj);

    rt_list_t list;
};

typedef struct ev *ev_t;

struct ev
{
    rt_list_t components_head;

    int (*startup)(ev_t ev);
    int (*stop)(ev_t ev);
};

int ev_object_install(ev_object_t obj, int type);

int ev_startup(ev_t ev);
int ev_stop(ev_t ev);

int ev_init(ev_t ev);

#endif /* __EV_H__ */
