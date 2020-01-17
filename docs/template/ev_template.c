#include "ev_template.h"

#define DBG_SECTION_NAME  "ev_template"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

static void pssc_notify(pss_client_t pssc, int topic, void *msg)
{
    // TODO

    // struct ev_template *template = (struct ev_template *)pssc;

    // if (msg == RT_NULL) return;

    // if (IS_SAME_TOPIC(topic, EV_TOTAL_TOPICS))
    // {
    //     ...
    // }
}

static int start(ev_object_t obj)
{
    // TODO
    
    LOG_I("Start");

    return RT_EOK;
}

static int stop(ev_object_t obj)
{
    // TODO

    LOG_I("Stop");

    return RT_EOK;
}

/*! the parameter _ stands for (m, n, ...) that you need. */
int ev_template_install(ev_template_t template, void *_)
{
    // TODO
    if (ev_object_install(&template->parent, EV_OBJ_UNKNOWN) != RT_EOK)
    {
        return RT_ERROR;
    }
    template->parent.start = start;
    template->parent.stop = stop;
    /* ! only after then can you do something 
         that involves the parent class
         such as subscribe and publish.*/

    // EV_CHECK_EOK_RVL(EV_SUBSCRIBE(template, EV_TOTAL_TOPICS));
    // ... 

    LOG_I("Installed");

    return RT_EOK;
}
