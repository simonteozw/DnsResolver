#include <stdint.h>
#include <stdio.h>

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