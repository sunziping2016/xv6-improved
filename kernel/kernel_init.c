#include "etypes.h"
#include "edefs.h"
#include "unix.h"
#include "inet.h"

typedef void (*init_hook_t)();

static init_hook_t init_hooks[] = {
    unix_dgram_module_init,
    iface_local_init,
    inet_dgram_module_init
};

void kernel_extra_init()
{
    for (size_t i=0;i<ARRAY_SIZE(init_hooks);i++)
        init_hooks[i]();
}
