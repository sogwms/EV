/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "evt.h"
#if defined(EV_USING_MSH_CMD) && defined (FINSH_USING_MSH)

#include <stdlib.h>
#include <pss.h>

#define ABS(data) (data > 0 ? data : -data)

struct status
{
    struct pss_client pssc;

    struct msg_imu imu;
    struct msg_rpy rpy;
    struct msg_power power;
};

static void pssc_notify(pss_client_t pssc, int topic, void *msg)
{
    struct status* status = (struct status*)pssc;

    switch (topic)
    {
    case EV_TOPIC_IMU:
        if (msg != RT_NULL) 
        {
            struct msg_imu *mimu = (struct msg_imu *)msg;
            rt_memcpy(&status->imu, mimu, sizeof(struct msg_imu));
        }
        break;
    case EV_TOPIC_RPY:
        if (msg != RT_NULL) 
        {
            struct msg_imu *mrpy = (struct msg_imu *)msg;
            rt_memcpy(&status->rpy, mrpy, sizeof(struct msg_rpy));
        }
        break;
    case EV_TOPIC_POWER:
        if (msg != RT_NULL) 
        {
            struct msg_imu *mpower = (struct msg_imu *)msg;
            rt_memcpy(&status->power, mpower, sizeof(struct msg_power));
        }
        break;
    default:
        break;
    }
}

static struct status g_status = {0};

int evt_status_init(void)
{
    pss_init_client(&g_status.pssc);
    g_status.pssc.notify = pssc_notify;
    pss_subscribe(&g_status.pssc, EV_TOPIC_IMU);
    pss_subscribe(&g_status.pssc, EV_TOPIC_RPY);
    pss_subscribe(&g_status.pssc, EV_TOPIC_POWER);

    return RT_EOK;
}
INIT_APP_EXPORT(evt_status_init);

static void evt_status(int argc, char *argv[])
{
    if (argc < 2)
    {
        rt_kprintf("evt_status [OPTION] [PARAMS]\n");
        rt_kprintf("    power\n");
        rt_kprintf("    rpy <interval> <duration>\n");
        rt_kprintf("    imu <interval> <duration>\n");
 
    }
    else if (argc < 3)
    {
        if (!rt_strcmp("power", argv[1]))
        {
            rt_kprintf("current-power:%d  total-power:%d  alert-power:%d\n", 
                g_status.power.current_power, 
                g_status.power.total_power, 
                g_status.power.alert_power);
        }
    }
    else if (argc < 5)
    {
        int interval = atoi(argv[2]);
        int duration = atoi(argv[3]);

        if (!rt_strcmp("rpy", argv[1]))
        {
            struct msg_rpy mrpy;

            rt_memcpy(&mrpy, &g_status.rpy, sizeof(struct msg_rpy));
            for (int i = 0; i < duration/interval; i++)
            {
                rt_thread_mdelay(interval);
                mrpy.roll = (mrpy.roll + g_status.rpy.roll) / 2;
                mrpy.pitch = (mrpy.pitch + g_status.rpy.pitch) / 2;
                mrpy.yaw = (mrpy.yaw + g_status.rpy.yaw) / 2;
                rt_kprintf("[t%d] roll:%3d.%02d  pitch:%3d.%02d  yaw:%3d.%02d\n", i, 
                    (int)g_status.rpy.roll, ABS((int)(g_status.rpy.roll * 100) % 100),
                    (int)g_status.rpy.pitch, ABS((int)(g_status.rpy.pitch * 100) % 100),
                    (int)g_status.rpy.yaw, ABS((int)(g_status.rpy.yaw * 100) % 100));
            }
            rt_kprintf("[!average] roll:%3d.%02d  pitch:%3d.%02d  yaw:%3d.%02d\n",
                    (int)mrpy.roll, ABS((int)(mrpy.roll * 100) % 100),
                    (int)mrpy.pitch, ABS((int)(mrpy.pitch * 100) % 100),
                    (int)mrpy.yaw, ABS((int)(mrpy.yaw * 100) % 100));
        }
        else if (!rt_strcmp("imu", argv[1]))
        {
            struct msg_imu mimu;

            rt_memcpy(&mimu, &g_status.imu, sizeof(struct msg_imu));
            for (int i = 0; i < duration/interval; i++)
            {
                rt_thread_mdelay(interval);
                mimu.acc_x = (mimu.acc_x + g_status.imu.acc_x) / 2;
                mimu.acc_y = (mimu.acc_y + g_status.imu.acc_y) / 2;
                mimu.acc_z = (mimu.acc_z + g_status.imu.acc_z) / 2;
                mimu.gyro_x = (mimu.gyro_x + g_status.imu.gyro_x) / 2;
                mimu.gyro_y = (mimu.gyro_y + g_status.imu.gyro_y) / 2;
                mimu.gyro_z = (mimu.gyro_z + g_status.imu.gyro_z) / 2;
                mimu.mag_x = (mimu.mag_x + g_status.imu.mag_x) / 2;
                mimu.mag_y = (mimu.mag_y + g_status.imu.mag_y) / 2;
                mimu.mag_z = (mimu.mag_z + g_status.imu.mag_z) / 2;
                rt_kprintf("[t%d] acc-x-y-z:%4d %4d %4d  gyro-x-y-z:%4d %4d %4d  mag-x-y-z:%4d %4d %4d\n", i, 
                    g_status.imu.acc_x, g_status.imu.acc_y, g_status.imu.acc_z,
                    g_status.imu.gyro_x, g_status.imu.gyro_y, g_status.imu.gyro_z,
                    g_status.imu.mag_x, g_status.imu.mag_y, g_status.imu.mag_z);
            }

            rt_kprintf("[!average] acc-x-y-z:%4d %4d %4d  gyro-x-y-z:%4d %4d %4d  mag-x-y-z:%4d %4d %4d\n",
                    mimu.acc_x, mimu.acc_y, mimu.acc_z,
                    mimu.gyro_x, mimu.gyro_y, mimu.gyro_z,
                    mimu.mag_x, mimu.mag_y, mimu.mag_z);
        }
    }
}
MSH_CMD_EXPORT(evt_status, test status);

#endif
