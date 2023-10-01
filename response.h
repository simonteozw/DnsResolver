/* Structure of the bytes for an IPv4 answer */
#include<stdint.h>

typedef struct {
  uint16_t compression;
  uint16_t type;
  uint16_t response_class;
  uint32_t ttl;
  uint16_t length;
  struct in_addr addr;
} __attribute__((packed)) dns_record_a_t;