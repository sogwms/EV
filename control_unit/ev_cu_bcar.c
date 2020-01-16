/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "ev_cu_bcar.h"

#define DBG_SECTION_NAME  "ev_cu_bcar"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

struct ev_cu_bcar_info
{
    struct msg_motion motion;
    struct msg_motor motor_speed;
    struct msg_imu  imu;
    struct msg_rpy  rpy;
};

struct ev_cu_bcar
{
    struct ev_object parent;
    struct ev_cu_bcar_info info; 
    struct ev_cu_bcar_params params;
};

typedef struct ev_cu_bcar *ev_cu_bcar_t;

static struct ev_cu_bcar g_cu_bcar = {0};

static void __compute(struct ev_cu_bcar_info *info, struct ev_cu_bcar_params *params, struct msg_motor *mmotor)
{
    long oangle = 0;
    long odir = 0;
    long ospeed = 0;
    long oleft;
    long oright;

    oangle = params->akp * (1.5f - info->rpy.pitch) + params->akd * (0.0f - info->imu.gyro_y);

    pss_publish(EV_TOPIC_REQUEST_MOTOR_SPEED, &info->motor_speed);
    params->serr = params->serr * 0.8f + (((info->motor_speed.m1 + info->motor_speed.m2) / 2) - 0) * 0.2f;
    params->sintegral += params->serr;
    params->sintegral -= info->motion.velocity_x;
    if (params->sintegral > EV_SCALE*20)
        params->sintegral = EV_SCALE*20;
    if (params->sintegral < -EV_SCALE*20)
        params->sintegral = -EV_SCALE*20;

    ospeed = params->skp * params->serr - params->ski * 0.1f * params->sintegral;
    odir = params->rkp * info->motion.rotation_z + params->rkd * info->imu.gyro_z;

    oleft = oangle + ospeed + odir;
    oright = oangle + ospeed - odir;

    if (oleft > EV_SCALE)
        oleft = EV_SCALE;
    if (oleft < -EV_SCALE)
        oleft = -EV_SCALE;
    
    if (oright > EV_SCALE)
        oright = EV_SCALE;
    if (oright < -EV_SCALE)
        oright = -EV_SCALE;

    mmotor->m1 = oleft;
    mmotor->m2 = oright;
}

static void pssc_notify(pss_client_t pssc, int topic, void *msg)
{
    struct ev_cu_bcar *cu_bcar = (struct ev_cu_bcar *)pssc;

    if (msg == RT_NULL) return;

    if (IS_SAME_TOPIC(topic, EV_TOPIC_IMU))
    {
        rt_memcpy(&cu_bcar->info.imu, msg, sizeof(struct msg_imu));
    }
    else if (IS_SAME_TOPIC(topic, EV_TOPIC_RPY))
    {
        rt_memcpy(&cu_bcar->info.rpy, msg, sizeof(struct msg_rpy));
        
        struct msg_motor mmotor;
        __compute(&cu_bcar->info, &cu_bcar->params, &mmotor);
        EV_PUBLISH(cu_bcar, EV_TOPIC_SET_MOTOR_SPEED, &mmotor);
    }
    else if (IS_SAME_TOPIC(topic, EV_TOPIC_RC))
    {
        rt_memcpy(&cu_bcar->info.motion, msg, sizeof(struct msg_motion));
    }
    else if (IS_SAME_TOPIC(topic, EV_TOPIC_SYS_STATUS))
    {
        struct msg_sys_status *msstatus = (struct msg_sys_status *)msg;
        if (!msstatus->bl_running)
        {
            cu_bcar->params.sintegral = 0;
            cu_bcar->params.serr = 0;
            LOG_I("clear cache");
        }
    }
}

static int start(ev_object_t obj)
{
    LOG_I("cu_bcar start");

    obj->pssc.notify = pssc_notify;

    return RT_EOK;
}

static int stop(ev_object_t obj)
{
    LOG_I("cu_bcar stop");

    obj->pssc.notify = RT_NULL;

    return RT_EOK;
}

struct ev_cu_bcar_params ev_cu_bcar_get_params(void)
{
    return g_cu_bcar.params;
}

void ev_cu_bcar_set_params(float akp, float akd, float skp, float ski, float rkp, float rkd)
{
    ev_cu_bcar_t cu_bcar = &g_cu_bcar;

    cu_bcar->params.akp = akp;
    cu_bcar->params.akd = akd;
    cu_bcar->params.skp = skp;
    cu_bcar->params.ski = ski;
    cu_bcar->params.rkp = rkp;
    cu_bcar->params.rkd = rkd;
}

int ev_cu_bcar_install(float akp, float akd, float skp, float ski, float rkp, float rkd)
{
    ev_cu_bcar_t cu_bcar = &g_cu_bcar;

    EV_CHECK_EOK_RVL(ev_object_install(&cu_bcar->parent, EV_OBJ_CONTROL_UNIT));
    cu_bcar->parent.start = start;
    cu_bcar->parent.stop = stop;
    /* ! only after then can you do something 
         that involves the parent class
         such as subscribe and publish.*/

    rt_memset(&cu_bcar->info, 0, sizeof(struct ev_cu_bcar_info));
    rt_memset(&cu_bcar->params, 0, sizeof(struct ev_cu_bcar_params));

    cu_bcar->params.akp = akp;
    cu_bcar->params.akd = akd;
    cu_bcar->params.skp = skp;
    cu_bcar->params.ski = ski;
    cu_bcar->params.rkp = rkp;
    cu_bcar->params.rkd = rkd;

    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(cu_bcar, EV_TOPIC_IMU));
    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(cu_bcar, EV_TOPIC_RPY));
    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(cu_bcar, EV_TOPIC_RC));
    EV_CHECK_EOK_RVL(EV_SUBSCRIBE(cu_bcar, EV_TOPIC_SYS_STATUS));

    return RT_EOK;
}
