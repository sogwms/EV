/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#ifndef __EV_CU_BCAR_H__
#define __EV_CU_BCAR_H__

#include <ev.h>

struct ev_cu_bcar_params
{
    float akp;
    float akd;
    float rkp;
    float rkd;

    int rcnt;

    float skp;
    float ski;
    float scount;
    int32_t serr;
    int32_t sintegral;
    int32_t sout_old, sout_err;

    int scnt;
};

struct ev_cu_bcar_params ev_cu_bcar_get_params(void);
void ev_cu_bcar_set_params(float akp, float akd, float skp, float ski, float rkp, float rkd);
int ev_cu_bcar_install(float akp, float akd, float skp, float ski, float rkp, float rkd);

#endif /* __EV_CU_BCAR_H__ */
