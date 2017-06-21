/** @file
 * @brief Local internet interface module
 * @author Qifan Lu
 * @date April 22, 2016
 * @version 1.0.0
 */

//[ Header Files ]
//Xv6 kernel extra
#include "etypes.h"
#include "edefs.h"
#include "errno.h"
#include "inet.h"
#include "inet_routing.h"

//[ Constants ]
/**
 * Local binded IP table capacity
 */
#define LOCAL_BIND_IP_TABLE_CAPACITY 16
/**
 * Transport layer protocol handler mapping capacity
 */
#define LOCAL_TP_PROTO_MAPPING_CAPACITY 16

//[ Types ]
/**
 * Binded IP address type
 */
struct local_bind_ip
{   /**
     * IP address
     */
    in_addr_t ip_addr;
    /**
     * Subnet size
     */
    unsigned int subnet_size;
};

/**
 * Transport layer protocol data handler type
 */
typedef int (*tp_proto_handler_t)(ip4_hdr_t*, unsigned int);

/**
 * Transport layer protocol mapping item type
 */
struct tp_proto_mapping_item
{   /**
     * Protocol number
     */
    uint16_t proto;
    /**
     * Protocol data receive handler
     */
    tp_proto_handler_t handler;
};

/**
 * Local interface type
 */
typedef struct
{   char* name;
    data_handler_t handler;

    /**
     * Binded address table
     */
    struct local_bind_ip bind_ip_table[LOCAL_BIND_IP_TABLE_CAPACITY];
    /**
     * Binded address size
     */
    unsigned int local_bind_ip_size;

    /**
     * Transport layer protocol handler mapping
     */
    struct tp_proto_mapping_item tp_proto_mapping[LOCAL_TP_PROTO_MAPPING_CAPACITY];
    /**
     * Transport layer protocol handler mapping size
     */
    unsigned int tp_proto_mapping_size;
} local_iface_t;

//[ Variables ]
/**
 * Local interface
 */
static local_iface_t iface_local = {
    .name = "local",
    .local_bind_ip_size = 0,
    .tp_proto_mapping_size = 0
};

/**
 * Local interface index
 */
static unsigned int iface_local_index;

//[ Functions ]
/**
 * Register new local IP
 *
 * @param ip_addr IP address
 * @param subnet_size Subnet size
 * @return Negative number for error and 0 for success
 */
int local_reg_ip(local_iface_t* iface, in_addr_t ip_addr, unsigned int subnet_size)
{   //Too many IP records
    if (iface->local_bind_ip_size>=LOCAL_BIND_IP_TABLE_CAPACITY)
        return -1*ENOMEM;
    //Insert new IP record
    struct local_bind_ip new_ip = {
        .ip_addr = ip_addr,
        .subnet_size = subnet_size
    };
    iface->bind_ip_table[iface->local_bind_ip_size] = new_ip;
    iface->local_bind_ip_size++;

    return 0;
}

/**
 * Check if given IP is a local IP
 *
 * @param iface Local internet interface
 * @param ip_addr IP address
 * @return Boolean value
 */
bool is_local_ip(local_iface_t* iface, in_addr_t ip_addr)
{   for (unsigned int i=0;i<iface->local_bind_ip_size;i++)
    {   struct local_bind_ip current_ip = iface->bind_ip_table[i];
        if ((current_ip.ip_addr^ip_addr)<<(current_ip.subnet_size)==0)
            return true;
    }
    return false;
}

/**
 * Register transport layer protocol packet handler
 *
 * @param Local internet interface
 * @param proto_id Protocol ID
 * @param handler Packet handler
 * @return Negative number for error and 0 for success
 */
int local_reg_tp_proto_handler(local_iface_t* iface, uint16_t proto_id, tp_proto_handler_t handler)
{   //Too many handlers
    if (iface->tp_proto_mapping_size>=LOCAL_TP_PROTO_MAPPING_CAPACITY)
        return -1*ENOMEM;
    //Insert new handler
    struct tp_proto_mapping_item new_mapping = {
        .proto = proto_id,
        .handler = handler
    };
    iface->tp_proto_mapping[iface->tp_proto_mapping_size] = new_mapping;
    iface->tp_proto_mapping_size++;

    return 0;
}

/**
 * Fetch transport layer protocol data handler
 *
 * @param iface Local interface
 * @param proto_id Transport layer protocol ID
 * @return Data handler function
 */
static tp_proto_handler_t local_get_tp_proto_handler(local_iface_t* iface, uint16_t proto_id)
{   //Naively look up for desired hander
    for (unsigned int i=0;i<iface->tp_proto_mapping_size;i++)
        if (iface->tp_proto_mapping[i].proto==proto_id)
            return iface->tp_proto_mapping[i].handler;
    //Not found
    return NULL;
}

/**
 * Receive IP packet from routing module
 *
 * @param ip_packet IP packet
 * @param from_iface Last interface index
 * TODO: Return value
 */
static int iface_local_recv(ip4_hdr_t* ip_packet, unsigned int from_iface)
{   //Fetch transport layer protocol data handler
    tp_proto_handler_t handler = local_get_tp_proto_handler(&iface_local, ip_packet->protocol);
    //Handler not found
    if (!handler)
        return -1*EPROTONOSUPPORT;

    //Forward data to handler
    return (*handler)(ip_packet, from_iface);
}

/**
 * Send packet to routing module
 *
 * @param ip_packet IP packet
 * @return TODO: Return value
 */
int iface_local_send(ip4_hdr_t* ip_packet)
{   return inet_routing_send(ip_packet, iface_local_index);
}

//[ Functions ]
//Local internet interface module initialization
void iface_local_init()
{   //Set incoming data handler
    iface_local.handler = iface_local_recv;
    //Register local interface
    iface_local_index = inet_iface_add((struct inet_iface*)(&iface_local));

    //* Register transport layer protocol handlers
    //UDP
    local_reg_tp_proto_handler(&iface_local, IPP_UDP, inet_dgram_packet_handler);

    //* Add local IPs
    //"127.0.0.0/8"
    local_reg_ip(&iface_local, 0x7f000000, 24);
#ifdef ETHERNET_ACCESS
    //"172.20.0.1/32"
    local_reg_ip(&iface_local, 0x020014ac, 0);
#endif

    //* Add routes
    //"lo 127.0.0.0/8 -> 127.0.0.0/8 lo" (Priority: 16)
    struct inet_routing_item next_route = {
        .source_iface_id = iface_local_index,
        .source_ip = 0x7f,
        .source_subnet_size = 24,
        .dest_ip = 0x7f,
        .dest_subnet_size = 24,
        .dest_iface_id = iface_local_index,
        .priority = 16
    };
    inet_routing_add(next_route);
#ifdef ETHERNET_ACCESS
#endif
}
