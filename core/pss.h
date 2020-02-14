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
 */

#ifndef __PSS_H__
#define __PSS_H__

#include "ev_msg.h"

#define PSS_ENABLE_STATS

typedef struct pss_client *pss_client_t;

struct pss_client
{
    int id;
    void (*notify)(pss_client_t pssc, int topic, void *msg);
};

int pss_init_client(pss_client_t pssc);

int pss_get_topics_number(void);
int pss_get_clients_number(void);

int pss_publish(int topic, void *msg);
int pss_subscribe(pss_client_t pssc, int topic);
int pss_unsubscribe(pss_client_t pssc, int topic);
int pss_unsubscribe_all(pss_client_t pssc);

#endif /* __PSS_H__ */
