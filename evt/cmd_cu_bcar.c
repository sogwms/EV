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

#include <ev_cu_bcar.h>

static void __report_parameters(void)
{
    struct ev_cu_bcar_params params = ev_cu_bcar_get_params();

    rt_kprintf("!0.01! akp:%d.%03d akd:%d.%03d skp:%d.%03d ski:%d.%03d rkp:%d.%03d rkd:%d.%03d\n",
                (int)params.akp, (int)(params.akp * 1000) % 1000,
                (int)params.akd, (int)(params.akd * 1000) % 1000,
                (int)params.skp, (int)(params.skp * 1000) % 1000,
                (int)params.ski, (int)(params.ski * 1000) % 1000,
                (int)params.rkp, (int)(params.rkp * 1000) % 1000,
                (int)params.rkd, (int)(params.rkd * 1000) % 1000);
}

static void evt_cu_bcar(int argc, char *argv[])
{
    if (argc < 2)
    {
        rt_kprintf("ev_cu_bcar [OPTION] [PARAMS]\n");
        rt_kprintf("    read\n");
        rt_kprintf("    set <akp> <akd> <skp> <ski> <rkp> <rkd>\n");
    }
    else if (argc < 3)
    {
        if (!rt_strcmp("read", argv[1]))
        {
            __report_parameters();
        }
    }
    else if (argc < 9)
    {
        if (!rt_strcmp("set", argv[1]))
        {
            ev_cu_bcar_set_params(stof(argv[2]), stof(argv[3]),
                                   stof(argv[4]), stof(argv[5]),
                                   stof(argv[6]), stof(argv[7]));
            __report_parameters();
        }
    }
}
MSH_CMD_EXPORT(evt_cu_bcar, adjust parameters);

#endif
