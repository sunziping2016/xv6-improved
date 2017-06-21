/** @file
 * @brief Xv6 Internet routing module implementation
 * @author Qifan Lu
 * @date April 12, 2016
 * @version 1.0.0
 */

//[ Includes ]
//Xv6 kernel extra
#include "etypes.h"
#include "edefs.h"
#include "errno.h"
#include "inet_routing.h"

//[ Constants ]
/**
 * Interface table capacity
 */
#define INET_IFACE_TABLE_CAPACITY 16

/**
 * Routing table capacity
 */
#define INET_ROUTING_TABLE_CAPACITY 256

//[ Variables ]
/**
 * Interface table
 */
static struct inet_iface* inet_iface_table[INET_IFACE_TABLE_CAPACITY] = {0};

/**
 * Routing table
 */
static struct inet_routing_item inet_routing_table[INET_ROUTING_TABLE_CAPACITY];

/**
 * Routing table curent size
 */
static unsigned int inet_routing_table_size = 0;

//[ Functions ]
//* Routing module internal functions
/**
 * Routing table item comparison
 *
 * @param item1 Routing table item to be compared.
 * @param item2 Routing table item to be compared.
 * @return Relation between two routing table items.
 */
static int inet_rt_cmp(struct inet_routing_item* item1, struct inet_routing_item* item2)
{   //Compare priority
    if (item1->priority<item2->priority)
        return -1;
    if (item1->priority>item2->priority)
        return 1;

    //Compare source internet interface
    if ((item1->source_iface_id==INET_IFACE_ANY)&&(item2->source_iface_id!=INET_IFACE_ANY))
        return -1;
    if ((item1->source_iface_id!=INET_IFACE_ANY)&&(item2->source_iface_id==INET_IFACE_ANY))
        return 1;

    //Compare source IP subnet size
    if (item1->source_subnet_size>item2->source_subnet_size)
        return -1;
    if (item1->source_subnet_size<item2->source_subnet_size)
        return 1;

    //Compare destination IP subnet size
    if (item1->dest_subnet_size>item2->dest_subnet_size)
        return -1;
    if (item1->dest_subnet_size<item2->dest_subnet_size)
        return 1;

    return 0;
}

/**
 * Check if given IP packet matches given routing item
 *
 * @param ip_packet IP packet
 * @param source_iface_id Source interface ID
 * @param rt_item Routing item
 * @return "true" for matching and "false" for not matching.
 */
static bool inet_rt_match(ip4_hdr_t* ip_packet, unsigned int source_iface_id, struct inet_routing_item* rt_item)
{   //Source internet interface
    if ((rt_item->source_iface_id!=source_iface_id)&&(rt_item->source_iface_id!=INET_IFACE_ANY))
        return false;
    //Source IP address
    if ((IP_A2I(ip_packet->src)^rt_item->source_ip)<<(rt_item->source_subnet_size)!=0)
        return false;
    //Destination IP address
    if ((IP_A2I(ip_packet->dst)^rt_item->dest_ip)<<(rt_item->dest_subnet_size)!=0)
        return false;
    return true;
}

//* Interface / routing management API
//Add a new network interface
int inet_iface_add(struct inet_iface* iface)
{   //Null pointer not allowed
    if (iface==NULL)
        return -1*EINVAL;

    //Naively find a null pointer to insert given interface
    for (unsigned int i=0;i<INET_IFACE_TABLE_CAPACITY;i++)
        if (inet_iface_table[i]==NULL)
        {   inet_iface_table[i] = iface;
            return i;
        }
    //Full interface table, failed
    return -1*ENOMEM;
}

//Get interface from name
int inet_iface_from_name(const char* name, struct inet_iface** iface)
{   //Naively look up interface by name
    for (unsigned int i=0;i<INET_IFACE_TABLE_CAPACITY;i++)
    {   struct inet_iface* current_iface = inet_iface_table[i];
        if ((current_iface)&&(strcmp(current_iface->name, name)==0))
        {   if (iface)
                *iface = current_iface;
            return i;
        }
    }
    //Not found
    if (iface)
        *iface = NULL;
    return -1;
}

//* Routing API
//Add a new routing table item
int inet_routing_add(struct inet_routing_item rt_item)
{   //Full routing table, failed
    if (inet_routing_table_size>=INET_ROUTING_TABLE_CAPACITY)
        return -1;
    //Insert item by order
    int i;
    for (i=inet_routing_table_size-1;i>0;i--)
    {   if (inet_rt_cmp(&rt_item, inet_routing_table+i)>0)
            inet_routing_table[i+1] = inet_routing_table[i];
        else
            break;
    }
    inet_routing_table[i+1] = rt_item;
    //Update routing table size
    inet_routing_table_size++;
    return 0;
}

//Send IP packet through router
int inet_routing_send(ip4_hdr_t* ip_packet, unsigned int source_iface_id)
{   while (true)
    {   //Routing table matching
        unsigned int i;
        bool rt_item_found = false;
        for (i=0;i<INET_ROUTING_TABLE_CAPACITY;i++)
            if (inet_rt_match(ip_packet, source_iface_id, inet_routing_table+i))
            {   rt_item_found = true;
                break;
            }
        if (!rt_item_found)
            return -1*EHOSTUNREACH;
        //Perform IP packet forwarding
        unsigned int dest_iface_id = inet_routing_table[i].dest_iface_id;
        //Normal interface
        if (inet_iface_table[dest_iface_id]->handler)
            return inet_iface_table[dest_iface_id]->handler(ip_packet, source_iface_id);
        //"Direct" interface
        else
            source_iface_id = i;
    }
    return 0;
}

//Get interface by index
struct inet_iface* inet_iface_by_index(unsigned int index)
{   if ((index>=0)&&(index<INET_IFACE_TABLE_CAPACITY))
        return inet_iface_table[index];
    else
        return NULL;
}
