#include "edefs.h"
#include "inet.h"

typedef int (*data_handler_t)(ip4_hdr_t*, unsigned int);

struct inet_iface
{ 
    char* name;
    data_handler_t handler;
};

struct inet_routing_item
{  
    unsigned int source_iface_id;
    in_addr_t source_ip;
    unsigned int source_subnet_size;
    in_addr_t dest_ip;
    unsigned int dest_subnet_size;
    unsigned int dest_iface_id;
    unsigned int priority;
};

static const unsigned int INET_IFACE_ANY = 0xffff;

extern int inet_iface_add(struct inet_iface* iface);
extern struct inet_iface* inet_iface_by_index(unsigned int index);
extern int inet_iface_from_name(const char* name, struct inet_iface** iface);
extern int inet_routing_add(struct inet_routing_item rt_item);
extern int inet_routing_send(ip4_hdr_t* ip_packet, unsigned int source_iface_id);
