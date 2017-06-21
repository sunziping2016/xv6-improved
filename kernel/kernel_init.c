/** @file
 * @brief Kernel initialization hooks for different modules
 * @author Qifan Lu
 * @date April 11, 2016
 * @version 1.0.0
 */

//[ Header Files ]
//Xv6 kernel extra
#include "etypes.h"
#include "edefs.h"
//Init hook functions
#include "unix.h"
#include "inet.h"

//[ Types ]
/**
 * Initialization hook type
 */
typedef void (*init_hook_t)();

//[ Variables ]
/**
 * Initialization hooks
 */
static init_hook_t init_hooks[] = {
    unix_dgram_module_init,
    iface_local_init,
    inet_dgram_module_init
};

//[ Functions ]
/**
 * Do initialization for kernel extra part
 */
void kernel_extra_init()
{   //Call all initialization hooks
    for (size_t i=0;i<ARRAY_SIZE(init_hooks);i++)
        init_hooks[i]();
}
