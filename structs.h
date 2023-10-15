#include <stdint.h>

typedef struct
{
  uint16_t len;
  char *data;
} String;

typedef struct
{
  uint16_t len;
  uint16_t pos;
  char *data;
} Stream;

typedef struct
{
  uint16_t xid;     /* Randomly chosen identifier */
  uint16_t flags;   /* Bit-mask to indicate request/response */
  uint16_t qdcount; /* Number of questions */
  uint16_t ancount; /* Number of answers */
  uint16_t nscount; /* Number of authority records */
  uint16_t arcount; /* Number of additional records */
} dns_header_t;

typedef struct
{
  uint16_t dnstype;  /* The QTYPE (1 = A) */
  uint16_t dnsclass; /* The QCLASS (1 = IN) */
  const char *name;  /* Pointer to the domain name in memory */
} dns_question_t;

typedef struct
{
  uint16_t type_;
  uint16_t class_;
  uint32_t ttl;
  union
  {
    uint16_t data_len;
    String data;
  };
  const char *name;
} dns_record_t;

typedef struct
{
  dns_header_t header;
  dns_question_t *questions;
  dns_record_t *answers;
  dns_record_t *authorities;
  dns_record_t *additionals;
} dns_packet_t;