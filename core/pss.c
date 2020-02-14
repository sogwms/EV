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

#include <rtthread.h>
#include "pss.h"

#define DBG_SECTION_NAME  "pss"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

struct list_pss_client
{
    rt_list_t list;
    pss_client_t client;
};

struct pss_topic
{
    rt_list_t clients_head;
};

struct pss_broker
{
    int total_topics;
    int total_clients;

    struct pss_topic *topics[EV_TOTAL_TOPICS];
};

static struct pss_broker global_broker = {
    .total_topics = 0,
    .total_clients = 0,
    .topics = {RT_NULL},
};

int pss_publish(int topic, void *msg)
{
    rt_list_t *node;

    if (global_broker.topics[topic] == RT_NULL)
        return RT_ERROR;

    rt_list_for_each(node, &global_broker.topics[topic]->clients_head)
    {
        struct list_pss_client *lc = (struct list_pss_client *)node;
        void (*_notify)(pss_client_t pssc, int topic, void *msg) = lc->client->notify;
        if (_notify != RT_NULL)
            _notify(lc->client, topic, msg);
    }

    return RT_EOK;
}

int pss_subscribe(pss_client_t pssc, int topic)
{
    if (global_broker.topics[topic] == RT_NULL)
    {
        struct pss_topic *new_t = (struct pss_topic *)rt_malloc(sizeof(struct pss_topic));
        RT_ASSERT(new_t != RT_NULL);
        rt_list_init(&new_t->clients_head);
        global_broker.topics[topic] = new_t;
        global_broker.total_topics++;

        LOG_D("new topic id:%d", topic);
    }

    struct list_pss_client *new_c = (struct list_pss_client *)rt_malloc(sizeof(struct list_pss_client));
    new_c->client = pssc;
    rt_list_insert_after(&global_broker.topics[topic]->clients_head, &new_c->list);

    return RT_EOK;
}

int pss_unsubscribe(pss_client_t pssc, int topic)
{
    // TODO
    return RT_ERROR;
}

int pss_unsubscribe_all(pss_client_t pssc)
{
    // TODO
    return RT_ERROR;
}

int pss_init_client(pss_client_t pssc)
{
    pssc->id = global_broker.total_clients++;
    pssc->notify = RT_NULL;

    LOG_D("new client id:%d", pssc->id);

    return RT_EOK;
}

int pss_get_topics_number(void)
{
    return global_broker.total_topics;
}

int pss_get_clients_number(void)
{
    return global_broker.total_clients;
}
