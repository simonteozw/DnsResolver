// Helper functions to encode queries to bytes
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "question.h"

void build_header(dns_header_t* header) {
  // dns_header_t header;
  memset (header, 0, sizeof (dns_header_t));
  header->xid= htons (0x1234);    /* Randomly chosen ID */
  header->flags = htons (0x0100); /* Q=0, RD=1 */
  header->qdcount = htons (1);    /* Sending 1 question */
  return;
}

void build_question(dns_question_t* question, const char* hostname) {
  // dns_question_t question;
  question->dnstype = htons (1);  /* QTYPE 1=A */
  question->dnsclass = htons (1); /* QCLASS 1=IN */

  /* DNS name format requires two bytes more than the length of the
    domain name as a string */
  question->name = static_cast<char*>(calloc(strlen (hostname) + 2, sizeof (char)));

  /* Leave the first byte blank for the first field length */
  memcpy (question->name + 1, hostname, strlen (hostname));
  char *prev = question->name;
  uint8_t count = 0; /* Used to count the bytes in a field */

  /* Traverse through the name, looking for the . locations */
  for (size_t i = 0; i < strlen (hostname); i++)
    {
      /* A . indicates the end of a field */
      if (hostname[i] == '.')
        {
          /* Copy the length to the byte before this field, then
            update prev to the location of the . */
          *prev = count;
          prev = question->name + i + 1;
          count = 0;
        }
      else
        count++;
    }
  *prev = count;

  return;
}

void build_packet(uint8_t* packet, const char* hostname) {
  dns_header_t header;
  dns_question_t question;

  build_header(&header);
  build_question(&question, hostname);

  size_t packetlen = sizeof (header) + strlen (hostname) + 2 + sizeof (question.dnstype) + sizeof (question.dnsclass);
  packet = static_cast<uint8_t *>(calloc (packetlen, sizeof (uint8_t)));
  uint8_t *p = (uint8_t *)packet;

  uint16_t t = question.dnstype;

  /* Copy the header first */
  memcpy (p, &header, sizeof (header));
  p += sizeof (header);

  /* Copy the question name, QTYPE, and QCLASS fields */
  memcpy (p, question.name, strlen (hostname) + 1);
  p += strlen (hostname) + 2; /* includes 0 octet for end */
  memcpy (p, &question.dnstype, sizeof (question.dnstype));
  p += sizeof (question.dnstype);
  memcpy (p, &question.dnsclass, sizeof (question.dnsclass));

  return;
}

