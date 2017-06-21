#include "etypes.h"
#include "edefs.h"
#include "errno.h"
#include "inet_routing.h"

#define INET_IFACE_TABLE_CAPACITY 16
#define INET_ROUTING_TABLE_CAPACITY 256

static struct inet_iface* inet_iface_table[INET_IFACE_TABLE_CAPACITY] = {0};
static struct inet_routing_item inet_routing_table[INET_ROUTING_TABLE_CAPACITY];

static unsigned int inet_routing_table_size = 0;
static int inet_rt_cmp(struct inet_routing_item* item1, struct inet_routing_item* item2)
{ 
    if (item1->priority<item2->priority)
        return -1;
    if (item1->priority>item2->priority)
        return 1;
    if ((item1->source_iface_id==INET_IFACE_ANY)&&(item2->source_iface_id!=INET_IFACE_ANY))
        return -1;
    if ((item1->source_iface_id!=INET_IFACE_ANY)&&(item2->source_iface_id==INET_IFACE_ANY))
        return 1;

    if (item1->source_subnet_size>item2->source_subnet_size)
        return -1;
    if (item1->source_subnet_size<item2->source_subnet_size)
        return 1;
    if (item1->dest_subnet_size>item2->dest_subnet_size)
        return -1;
    if (item1->dest_subnet_size<item2->dest_subnet_size)
        return 1;

    return 0;
}

static bool inet_rt_match(ip4_hdr_t* ip_packet, unsigned int source_iface_id, struct inet_routing_item* rt_item)
{ 
    if ((rt_item->source_iface_id!=source_iface_id)&&(rt_item->source_iface_id!=INET_IFACE_ANY))
        return false;
    if ((IP_A2I(ip_packet->src)^rt_item->source_ip)<<(rt_item->source_subnet_size)!=0)
        return false;
    if ((IP_A2I(ip_packet->dst)^rt_item->dest_ip)<<(rt_item->dest_subnet_size)!=0)
        return false;
    return true;
}

int inet_iface_add(struct inet_iface* iface)
{
    if (iface==NULL)
        return -1*EINVAL;
    for (unsigned int i=0;i<INET_IFACE_TABLE_CAPACITY;i++)
        if (inet_iface_table[i]==NULL)
        {   inet_iface_table[i] = iface;
            return i;
        }
    return -1*ENOMEM;
}

int inet_iface_from_name(const char* name, struct inet_iface** iface)
{
    for (unsigned int i=0;i<INET_IFACE_TABLE_CAPACITY;i++)
    {   struct inet_iface* current_iface = inet_iface_table[i];
        if ((current_iface)&&(strcmp(current_iface->name, name)==0))
        {   if (iface)
                *iface = current_iface;
            return i;
        }
    }
    if (iface)
        *iface = NULL;
    return -1;
}

int inet_routing_add(struct inet_routing_item rt_item)
{
    if (inet_routing_table_size>=INET_ROUTING_TABLE_CAPACITY)
        return -1;
    int i;
    for (i=inet_routing_table_size-1;i>0;i--)
    {   if (inet_rt_cmp(&rt_item, inet_routing_table+i)>0)
            inet_routing_table[i+1] = inet_routing_table[i];
        else
            break;
    }
    inet_routing_table[i+1] = rt_item;
    inet_routing_table_size++;
    return 0;
}

int inet_routing_send(ip4_hdr_t* ip_packet, unsigned int source_iface_id)
{   while (true)
    {  
        unsigned int i;
        bool rt_item_found = false;
        for (i=0;i<INET_ROUTING_TABLE_CAPACITY;i++)
            if (inet_rt_match(ip_packet, source_iface_id, inet_routing_table+i))
            {   rt_item_found = true;
                break;
            }
        if (!rt_item_found)
            return -1*EHOSTUNREACH;
        unsigned int dest_iface_id = inet_routing_table[i].dest_iface_id;
        if (inet_iface_table[dest_iface_id]->handler)
            return inet_iface_table[dest_iface_id]->handler(ip_packet, source_iface_id);
        else
            source_iface_id = i;
    }
    return 0;
}

struct inet_iface* inet_iface_by_index(unsigned int index)
{   if ((index>=0)&&(index<INET_IFACE_TABLE_CAPACITY))
        return inet_iface_table[index];
    else
        return NULL;
}
