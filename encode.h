// Helper functions to encode queries to bytes
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "structs.h"

static void write_stream(Stream *dst, const void *src, size_t n)
{
  memcpy(dst->data + dst->pos, src, n);
  dst->pos += n;
}

static int find_encoding_length_domain_name(const char *domain_name)
{
  int encoding_len = 0;
  char *_domain_name = strdup(domain_name); // copy of domain name
  char *tok = strtok(_domain_name, ".");
  while (tok)
  {
    encoding_len += 1 + strlen(tok);
    tok = strtok(NULL, ".");
  }
  free(_domain_name);
  encoding_len += 1; // terminating NULL
  return encoding_len;
}

// DNS domain names are encoded as www.test.com --> 3www4test3com
static const char *encode_dns_name(const char *domain_name)
{
  int encoding_len = find_encoding_length_domain_name(domain_name);

  char *out = (char *)malloc(encoding_len);
  if (!out)
  {
    perror("malloc failed");
    exit(1);
  }

  char *_domain_name = strdup(domain_name);
  char *tok = strtok(_domain_name, ".");
  char *ptr = out; // think of pointer as array
  while (tok)
  {
    int len = strlen(tok);
    ptr[0] = len;
    memcpy(ptr + 1, tok, len);
    ptr += 1 + len;
    tok = strtok(NULL, ".");
  }

  return out;
}

static const char *build_query(const char *domain_name, int record_type, int dns_class, int *query_size)
{
  uint16_t id = random() & 0xffff;
  uint16_t flags = 0;
  dns_header_t header = {
      .xid = htons(id),
      .flags = htons(flags),
      .qdcount = htons(1),
  };
  dns_question_t question = {
      .dnstype = htons(record_type),
      .dnsclass = htons(dns_class),
  };
  const char *name = encode_dns_name(domain_name);
  int name_len = strlen(name) + 1;
  int size = sizeof header + name_len + 4;
  char *out = (char *)malloc(size);
  if (!out)
  {
    perror("malloc failed");
    exit(1);
  }
  Stream stream = {.len = size, .pos = 0, .data = out};
  write_stream(&stream, &header, sizeof header);
  write_stream(&stream, name, name_len);
  write_stream(&stream, &question, 4);
  free((void *)name);
  *query_size = size;
  return out;
}
