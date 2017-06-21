#include "etypes.h"
#include "edefs.h"
#include "errno.h"
#include "inet.h"
#include "inet_routing.h"

#define LOCAL_BIND_IP_TABLE_CAPACITY 16
#define LOCAL_TP_PROTO_MAPPING_CAPACITY 16

struct local_bind_ip
{
    in_addr_t ip_addr;
    unsigned int subnet_size;
};

typedef int (*tp_proto_handler_t)(ip4_hdr_t*, unsigned int);

struct tp_proto_mapping_item
{  
    uint16_t proto;
    tp_proto_handler_t handler;
};

typedef struct
{   char* name;
    data_handler_t handler;
    struct local_bind_ip bind_ip_table[LOCAL_BIND_IP_TABLE_CAPACITY];
    unsigned int local_bind_ip_size;
    struct tp_proto_mapping_item tp_proto_mapping[LOCAL_TP_PROTO_MAPPING_CAPACITY];
    unsigned int tp_proto_mapping_size;
} local_iface_t;

static local_iface_t iface_local = {
    .name = "local",
    .local_bind_ip_size = 0,
    .tp_proto_mapping_size = 0
};

static unsigned int iface_local_index;

int local_reg_ip(local_iface_t* iface, in_addr_t ip_addr, unsigned int subnet_size)
{ 
    if (iface->local_bind_ip_size>=LOCAL_BIND_IP_TABLE_CAPACITY)
        return -1*ENOMEM;
    struct local_bind_ip new_ip = {
        .ip_addr = ip_addr,
        .subnet_size = subnet_size
    };
    iface->bind_ip_table[iface->local_bind_ip_size] = new_ip;
    iface->local_bind_ip_size++;

    return 0;
}

bool is_local_ip(local_iface_t* iface, in_addr_t ip_addr)
{   for (unsigned int i=0;i<iface->local_bind_ip_size;i++)
    {   struct local_bind_ip current_ip = iface->bind_ip_table[i];
        if ((current_ip.ip_addr^ip_addr)<<(current_ip.subnet_size)==0)
            return true;
    }
    return false;
}


int local_reg_tp_proto_handler(local_iface_t* iface, uint16_t proto_id, tp_proto_handler_t handler)
{ 
    if (iface->tp_proto_mapping_size>=LOCAL_TP_PROTO_MAPPING_CAPACITY)
        return -1*ENOMEM;
    struct tp_proto_mapping_item new_mapping = {
        .proto = proto_id,
        .handler = handler
    };
    iface->tp_proto_mapping[iface->tp_proto_mapping_size] = new_mapping;
    iface->tp_proto_mapping_size++;

    return 0;
}

static tp_proto_handler_t local_get_tp_proto_handler(local_iface_t* iface, uint16_t proto_id)
{
    for (unsigned int i=0;i<iface->tp_proto_mapping_size;i++)
        if (iface->tp_proto_mapping[i].proto==proto_id)
            return iface->tp_proto_mapping[i].handler;

    return NULL;
}


static int iface_local_recv(ip4_hdr_t* ip_packet, unsigned int from_iface)
{
    tp_proto_handler_t handler = local_get_tp_proto_handler(&iface_local, ip_packet->protocol);
    if (!handler)
        return -1*EPROTONOSUPPORT;
    return (*handler)(ip_packet, from_iface);
}

int iface_local_send(ip4_hdr_t* ip_packet)
{   return inet_routing_send(ip_packet, iface_local_index);
}

void iface_local_init()
{
    iface_local.handler = iface_local_recv;
    iface_local_index = inet_iface_add((struct inet_iface*)(&iface_local));
    local_reg_tp_proto_handler(&iface_local, IPP_UDP, inet_dgram_packet_handler);
    local_reg_ip(&iface_local, 0x7f000000, 24);
#ifdef ETHERNET_ACCESS
    local_reg_ip(&iface_local, 0x020014ac, 0);
#endif
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
