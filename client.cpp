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
// #include "structs.h"
#include "helper.h"

#define IPV4_ADDR(a, b, c, d) (((d) << 24) | ((c) << 16) | ((b) << 8) | ((a) << 0))
#define ROOT_NS IPV4_ADDR(198, 41, 0, 4)

#define _cleanup_(f) __attribute__((cleanup(f)))

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

static void *send_query(const char *domain_name, in_addr_t ns_addr)
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
  // const dns_packet_t *packet = parse_packet(&stream);

  // return packet;
}

// static in_addr_t resolve(const char *domain_name)
// {
//   _cleanup_(free_charp) char *cname = NULL;
//   in_addr_t ns_addr = ROOT_NS;

//   while (1)
//   {
//     char *data = NULL;
//     _cleanup_(free_packetp) const dns_packet_t *packet = NULL;

//     printf("Querying ");
//     print_dotted((const uint8_t *)&ns_addr, 4);
//     printf(" for %s\n", domain_name);
//     packet = send_query(domain_name, ns_addr);
//     if ((data = get_answer(packet)))
//     {
//       in_addr_t out;
//       memcpy(&out, data, 4);
//       return out;
//     }
//     else if ((data = get_cname(packet)))
//     {
//       free(cname);
//       cname = strdup(data);
//       domain_name = cname;
//       ns_addr = ROOT_NS;
//     }
//     else if ((data = get_ns_ip(packet)))
//     {
//       memcpy(&ns_addr, data, 4);
//     }
//     else if ((data = get_ns(packet)))
//     {
//       ns_addr = resolve(data);
//     }
//     else
//     {
//       fprintf(stderr, "ERROR: no recognized DNS record found in packet.\n");
//       print_packet(packet);
//       return -1;
//     }
//   }
// }

// int main(int argc, char const *argv[])
// {
//   int status, valread, client_fd;
//   const char *dns_ip = "8.8.8.8";
//   // char buffer[MAXLINE];
//   struct sockaddr_in serv_addr;
//   if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
//   {
//     perror("\n Socket creation error \n");
//     exit(EXIT_FAILURE);
//   }

//   // set server address to be all 0s
//   memset(&serv_addr, 0, sizeof(serv_addr));

//   serv_addr.sin_family = AF_INET;
//   if (inet_pton(AF_INET, dns_ip, &serv_addr.sin_addr) <= 0)
//   {
//     perror("\n Invalid address/ Address not supported \n");
//     exit(EXIT_FAILURE);
//   }
//   // serv_addr.sin_addr.s_addr = htonl (0xd043dede);
//   // htons() function converts the unsigned short integer hostshort
//   // from host byte order to network byte order
//   serv_addr.sin_port = htons(PORT);

//   printf("TEST\n");
//   uint8_t *packet;
//   const char *hostname = "www.example.com";
//   build_packet(packet, hostname);
//   sendto(client_fd, packet, sizeof(packet), 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
//   printf("SENT\n");
//   socklen_t length = 0;
//   uint8_t response[512];
//   memset(&response, 0, 512);

//   /* Receive the response into a local buffer */
//   ssize_t bytes = recvfrom(client_fd, response, 512, MSG_WAITALL, (struct sockaddr *)&serv_addr, &length);
//   printf("RECVD\n");
//   // int n;
//   // socklen_t len;
//   // n = recvfrom(client_fd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &serv_addr, &len);
//   // // printf("can_receive\n");
//   // buffer[n] = '\0';
//   // printf("Server : %s", buffer);

//   close(client_fd);
// }

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
