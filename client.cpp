// client side c++ program to using Socket to connect to Google public DNS
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "encode.h"
#include "decode.h"
#include "structs.h"
#include "helper.h"

#define IPV4_ADDR(a, b, c, d) (((d) << 24) | ((c) << 16) | ((b) << 8) | ((a) << 0))
#define ROOT_NS IPV4_ADDR(198, 41, 0, 4)

#define _cleanup_(f) __attribute__((cleanup(f)))

static const dns_packet_t *send_query(const char *domain_name, in_addr_t ns_addr)
{
  char reply_buf[1024];
  int query_size, num_read;

  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_fd == -1)
  {
    perror("socket creation failed");
    exit(1);
  }

  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(DNS_PORT),
      .sin_addr.s_addr = ns_addr,
  };

  const char *query = build_query(domain_name, TYPE_A, CLASS_IN, &query_size);
  if (sendto(sock_fd, query, query_size, 0, (const struct sockaddr *)&addr, sizeof addr) != query_size)
  {
    perror("sendto failed");
    exit(1);
  }
  free((void *)query);
  if ((num_read = recvfrom(sock_fd, reply_buf, sizeof reply_buf, 0, NULL, NULL)) == -1)
  {
    perror("recvfrom failed");
    exit(1);
  }
  if (close(sock_fd) == -1)
  {
    perror("socket close failed");
    exit(1);
  }

  Stream stream = {.len = num_read, .pos = 0, .data = reply_buf};
  const dns_packet_t *packet = parse_packet(&stream);

  return packet;
}

#define DEFINE_GETTER(fn_name, field, field_len, field_type) \
  static char *get_##fn_name(const dns_packet_t *packet)     \
  {                                                          \
    for (int i = 0; i < packet->header.field_len; i++)       \
    {                                                        \
      if (packet->field[i].dnstype == field_type)            \
        return packet->field[i].data.data;                   \
    }                                                        \
    return NULL;                                             \
  }

DEFINE_GETTER(answer, answers, ancount, TYPE_A)
DEFINE_GETTER(cname, answers, ancount, TYPE_CNAME)
DEFINE_GETTER(ns_ip, additionals, arcount, TYPE_A)
DEFINE_GETTER(ns, authorities, nscount, TYPE_NS)

#undef DEFINE_GETTER

static in_addr_t resolve(const char *domain_name)
{
  _cleanup_(free_charp) char *cname = NULL;
  in_addr_t ns_addr = ROOT_NS;

  while (1)
  {
    char *data = NULL;
    _cleanup_(free_packetp) const dns_packet_t *packet = NULL;

    printf("Querying ");
    print_dotted((const uint8_t *)&ns_addr, 4);
    printf(" for %s\n", domain_name);
    packet = send_query(domain_name, ns_addr);
    if ((data = get_answer(packet)))
    {
      in_addr_t out;
      memcpy(&out, data, 4);
      return out;
    }
    else if ((data = get_cname(packet)))
    {
      free(cname);
      cname = strdup(data);
      domain_name = cname;
      ns_addr = ROOT_NS;
    }
    else if ((data = get_ns_ip(packet)))
    {
      memcpy(&ns_addr, data, 4);
    }
    else if ((data = get_ns(packet)))
    {
      ns_addr = resolve(data);
    }
    else
    {
      fprintf(stderr, "ERROR: no recognized DNS record found in packet.\n");
      print_packet(packet);
      return -1;
    }
  }
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <domain_name>\n", argv[0]);
    exit(1);
  }
  const char *domain_name = argv[1];

  srandom(time(NULL));

  in_addr_t addr = resolve(domain_name);
  print_dotted((const uint8_t *)&addr, 4);
  printf("\n");
}
