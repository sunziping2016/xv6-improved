/** @file
 * @brief Xv6 Internet routing module declaration
 * @author Qifan Lu
 * @date April 12, 2016
 * @version 1.0.0
 */

//[ Header Files ]
//Xv6 kernel extra
#include "edefs.h"
#include "inet.h"

//[ Types ]
/**
 * Data handler type
 */
typedef int (*data_handler_t)(ip4_hdr_t*, unsigned int);

/**
 * Network interface type
 */
struct inet_iface
{   /**
     * Network interface name
     */
    char* name;
    /**
     * Incoming data handler ("NULL" represents "direct" interface)
     */
    data_handler_t handler;
};

/**
 * Routing table item type
 */
struct inet_routing_item
{   /**
     * Source interface
     */
    unsigned int source_iface_id;
    /**
     * Source IP address
     */
    in_addr_t source_ip;
    /**
     * Source IP address subnet size
     */
    unsigned int source_subnet_size;
    /**
     * Destination IP address
     */
    in_addr_t dest_ip;
    /**
     * Destination IP address subnet size
     */
    unsigned int dest_subnet_size;
    /**
     * Destination interface index
     */
    unsigned int dest_iface_id;
    /**
     * Priority
     */
    unsigned int priority;
};

//[ Constants ]
/**
 * Any interface
 */
static const unsigned int INET_IFACE_ANY = 0xffff;

//[ Functions ]
//* Interface management API
/**
 * Add a new network interface.
 *
 * @param iface Interface to be added.
 * @return positive interface index for success, negative number for failure.
 */
extern int inet_iface_add(struct inet_iface* iface);

/**
 * Get interface by index
 *
 * @param index Index of interface
 * @return Interface reference on success, or NULL on failure.
 */
extern struct inet_iface* inet_iface_by_index(unsigned int index);

/**
 * Get interface index from name
 *
 * @param name Network interface name
 * @param iface Used to store network interface reference
 * @return Non-negative index on success, or -1 when interface not found.
 */
extern int inet_iface_from_name(const char* name, struct inet_iface** iface);

//* Routing API
/**
 * Add a new routing table item.
 *
 * @param rt_item Routing table item.
 * @return 0 for success, -1 for failure.
 */
extern int inet_routing_add(struct inet_routing_item rt_item);

/**
 * Send IP packet through router
 *
 * @param ip_header IP packet
 * @param source_iface_id Source interface ID
 * @return 0 for success, non-zero value for failure.
 */
extern int inet_routing_send(ip4_hdr_t* ip_packet, unsigned int source_iface_id);
