// Helper functions to decode byte responses
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "structs.h"
#include "constants.h"

static void read_stream(void *dst, Stream *src, size_t n)
{
  memcpy(dst, src->data + src->pos, n);
  src->pos += n;
}

static uint16_t tell_stream(const Stream *stream)
{
  return stream->pos;
}

static void seek_stream(Stream *stream, uint16_t pos)
{
  stream->pos = pos;
}

static const char *decode_dns_name(Stream *stream);

static void parse_header(Stream *stream, dns_header_t *out)
{
  read_stream(out, stream, sizeof *out);
  // converts the unsigned short integer netshort from network byte order to host byte order
  *out = (dns_header_t){
      .xid = ntohs(out->xid),
      .flags = ntohs(out->flags),
      .qdcount = ntohs(out->qdcount),
      .ancount = ntohs(out->ancount),
      .nscount = ntohs(out->nscount),
      .arcount = ntohs(out->arcount),
  };
}

static const char *decode_compressed_name(Stream *stream, uint8_t field_len)
{
  uint8_t next_byte;
  read_stream(&next_byte, stream, 1);
  uint16_t ptr = ((field_len & 0x3f) << 8) | next_byte; // bottom 6 bits of length + next byte
  uint16_t cur = tell_stream(stream);
  if (ptr == cur)
    return NULL;
  seek_stream(stream, ptr);
  const char *out = decode_dns_name(stream);
  seek_stream(stream, cur);
  return out;
}

static const char *decode_dns_name(Stream *stream)
{
  char *out = NULL;
  int prev_len = 0;
  uint8_t field_len;
  read_stream(&field_len, stream, 1);
  while (field_len != 0)
  {
    if ((field_len & 0xc0) != 0) // check if first 2 bits are set, if so means compressed
    {
      const char *res = decode_compressed_name(stream, field_len);
      int res_len = strlen(res);
      out = (char *)realloc(out, prev_len + res_len + 1);
      if (!out)
      {
        perror("realloc failed");
        exit(1);
      }
      memcpy(&out[prev_len], res, res_len);
      out[prev_len + res_len] = '.';
      prev_len += res_len + 1;
      free((void *)res);
      break;
    }
    else
    {
      out = (char *)realloc(out, prev_len + field_len + 1);
      if (!out)
      {
        perror("realloc failed");
        exit(1);
      }
      read_stream(&out[prev_len], stream, field_len);
      out[prev_len + field_len] = '.';
      prev_len += field_len + 1;
    }
    read_stream(&field_len, stream, 1);
  }
  if (prev_len > 0)
    out[prev_len - 1] = '\0';
  return out;
}

static void parse_question(Stream *stream, dns_question_t *out)
{
  const char *name = decode_dns_name(stream);
  read_stream(out, stream, 4);
  *out = (dns_question_t){
      .dnstype = ntohs(out->dnstype),
      .dnsclass = ntohs(out->dnsclass),
      .name = name,
  };
}

static void parse_record(Stream *stream, dns_record_t *out)
{
  const char *name = decode_dns_name(stream);
  // the type, class, TTL, and data length together are 10 bytes (2 + 2 + 4 + 2 = 10) so we read 10 bytes
  read_stream(out, stream, 10);
  *out = (dns_record_t){
      .dnstype = ntohs(out->dnstype),
      .dnsclass = ntohs(out->dnsclass),
      .ttl = ntohl(out->ttl),
      .data.len = ntohs(out->data_len),
      .name = name,
  };
  if (out->dnstype == TYPE_NS || out->dnstype == TYPE_CNAME)
  {
    out->data.data = (char *)decode_dns_name(stream);
    out->data.len = strlen(out->data.data);
  }
  else
  {
    out->data.data = (char *)malloc(out->data.len);
    if (!out->data.data)
    {
      perror("malloc failed");
      exit(1);
    }
    read_stream(out->data.data, stream, out->data.len);
  }
}

static const dns_packet_t *parse_packet(Stream *stream)
{
#define READ_RECORD(parse_fn, field, field_len, typecast) ({                 \
  out->field = (typecast)malloc(out->header.field_len * sizeof *out->field); \
  if (!out->field)                                                           \
  {                                                                          \
    perror("malloc failed");                                                 \
    exit(1);                                                                 \
  }                                                                          \
  for (int i = 0; i < out->header.field_len; i++)                            \
    parse_fn(stream, &out->field[i]);                                        \
})
  dns_packet_t *out = (dns_packet_t *)malloc(sizeof *out);
  if (!out)
  {
    perror("malloc failed");
    exit(1);
  }
  parse_header(stream, &out->header);
  READ_RECORD(parse_question, questions, qdcount, dns_question_t *);
  READ_RECORD(parse_record, answers, ancount, dns_record_t *);
  READ_RECORD(parse_record, authorities, nscount, dns_record_t *);
  READ_RECORD(parse_record, additionals, arcount, dns_record_t *);
  return out;
#undef READ_RECORD
}
