#include "etypes.h"
#include "inet.h"

static uint16_t checksum(const uint16_t* buf, int size)
{
  uint32_t sum = 0;
  while (size > 1) {
    sum += *buf++;
    size -= 2;
  }
  if (size > 0)
    sum += *(uint8_t*)buf;
  sum = (sum & 0xFFFF) + (sum >> 16);
  sum = (sum & 0xFFFF) + (sum >> 16);
  return (uint16_t)(~sum);
}

void ip4_checksum(ip4_hdr_t* hdr)
{
  hdr->checksum = 0;
  hdr->checksum = checksum((uint16_t*)hdr, (hdr->ver_ihl & 0xF) * 4);
}

void udp_checksum(ip4_hdr_t* ip, udp_hdr_t* udp, uint16_t* data)
{
  int i;
  uint32_t addr;
  uint32_t sum = 0;
  int size;

  udp->checksum = 0;

  addr = *(uint32_t*)ip->src;
  sum += (addr >> 16) & 0xFFFF;
  sum += addr & 0xFFFF;
  addr = *(uint32_t*)ip->dst;
  sum += (addr >> 16) & 0xFFFF;
  sum += addr & 0xFFFF;
  sum += HTONS(ip->protocol);
  sum += udp->length;

  for (i = 0; i < 4; ++i)
    sum += ((uint16_t*)udp)[i];

  size = NTOHS(udp->length) - sizeof(*udp);
  while (size > 1) {
    sum += *data++;
    size -= 2;
  }
  if (size)
    sum += *(uint8_t*)data;

  sum = (sum & 0xFFFF) + (sum >> 16);
  sum = (sum & 0xFFFF) + (sum >> 16);
  udp->checksum = (uint16_t)~sum;
}
