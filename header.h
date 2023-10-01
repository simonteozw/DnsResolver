#include<stdint.h>

typedef struct {
  uint16_t xid;      /* Randomly chosen identifier */
  uint16_t flags;    /* Bit-mask to indicate request/response */
  uint16_t qdcount;  /* Number of questions */
  uint16_t ancount;  /* Number of answers */
  uint16_t nscount;  /* Number of authority records */
  uint16_t arcount;  /* Number of additional records */
} dns_header_t;