#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "structs.h"
#include "constants.h"

static void free_charp(char **ptr)
{
  free(*ptr);
}

static void free_question(dns_question_t *q)
{
  free((void *)q->name);
}

static void free_record(dns_record_t *r)
{
  free((void *)r->name);
  free(r->data.data);
}

static void free_packetp(const dns_packet_t **p)
{
#define FREE_RECORD(free_fn, field, field_len) ({  \
  for (int i = 0; i < (*p)->header.field_len; i++) \
    free_fn(&(*p)->field[i]);                      \
  free((void *)(*p)->field);                       \
})
  FREE_RECORD(free_question, questions, qdcount);
  FREE_RECORD(free_record, answers, ancount);
  FREE_RECORD(free_record, authorities, nscount);
  FREE_RECORD(free_record, additionals, arcount);
  free((void *)(*p));
#undef FREE_RECORD
}

static void print_question(const dns_question_t *q)
{
  printf("(struct dns_question){ "
         ".name = \"%s\", "
         ".type = %d, "
         ".class = %d }\n",
         q->name, q->dnstype, q->dnsclass);
}

static void print_dotted(const uint8_t *data, int len)
{
  if (len == 0)
    return;
  printf("%d", data[0]);
  for (int i = 1; i < len; i++)
    printf(".%d", data[i]);
}

static void print_hex(const uint8_t *data, int len)
{
  for (int i = 0; i < len; i++)
    printf("%02x", data[i]);
}

static void print_record(const dns_record_t *record)
{
  printf("(struct dns_record){ "
         ".name = \"%s\", "
         ".type = %d, "
         ".class = %d, "
         ".ttl = %d, "
         ".data = ",
         record->name, record->dnstype, record->dnsclass,
         record->ttl);
  if (record->dnstype == TYPE_A)
  {
    print_dotted((const uint8_t *)record->data.data, record->data.len);
  }
  else if (record->dnstype == TYPE_NS || record->dnstype == TYPE_CNAME)
  {
    printf("\"%s\"", record->data.data);
  }
  else
  {
    print_hex((const uint8_t *)record->data.data, record->data.len);
  }
  printf(" }\n");
}

static void print_packet(const dns_packet_t *packet)
{
#define PRINT_RECORD(print_fn, field, field_len, field_header) ({ \
  printf("%s:\n", field_header);                                  \
  for (int i = 0; i < packet->header.field_len; i++)              \
  {                                                               \
    print_fn(&packet->field[i]);                                  \
  }                                                               \
})
  PRINT_RECORD(print_question, questions, qdcount, "Questions");
  PRINT_RECORD(print_record, answers, ancount, "Answers");
  PRINT_RECORD(print_record, authorities, nscount, "Authorities");
  PRINT_RECORD(print_record, additionals, arcount, "Additionals");
#undef PRINT_RECORD
}