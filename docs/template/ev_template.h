#ifndef __EV_TEMPLATE_H__
#define __EV_TEMPLATE_H__

#include <ev.h>

/* operations that need to be implemented by sub-class*/
struct ev_template_ops
{
    // ...
};

struct ev_template
{
    struct ev_object parent;
    const struct ev_template_ops *ops;
    // ...
};

typedef struct ev_template *ev_template_t;

// ...

#endif /* __EV_TEMPLATE_H__ */
