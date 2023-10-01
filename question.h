#include<stdint.h>

typedef struct {
  char *name;        /* Pointer to the domain name in memory */
  uint16_t dnstype;  /* The QTYPE (1 = A) */
  uint16_t dnsclass; /* The QCLASS (1 = IN) */
} dns_question_t;