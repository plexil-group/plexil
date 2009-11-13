#ifndef __LCM_INTERNAL_H__
#define __LCM_INTERNAL_H__

#include <glib.h>
#include "lcm.h"

typedef struct _lcm_provider_t lcm_provider_t;
typedef struct _lcm_provider_info_t lcm_provider_info_t;
typedef struct _lcm_provider_vtable_t lcm_provider_vtable_t;

struct _lcm_provider_info_t {
    char * name;
    lcm_provider_vtable_t * vtable;
};

struct _lcm_provider_vtable_t {
    lcm_provider_t * (*create)(lcm_t *, const char *target,
            const GHashTable *args);
    void (*destroy)(lcm_provider_t *);
    int (*subscribe)(lcm_provider_t *, const char *channel);
    int (*publish)(lcm_provider_t *, const char *, const void *, 
            unsigned int);
    int (*handle)(lcm_provider_t *);
    int (*get_fileno)(lcm_provider_t *);
};

int
lcm_parse_url (const char * url, char ** provider, char ** target,
        GHashTable * args);
int
lcm_has_handlers (lcm_t * lcm, const char * channel);
int
lcm_dispatch_handlers (lcm_t * lcm, lcm_recv_buf_t * buf, const char *channel);

#endif
