#include <stdio.h>
#include "../encode.h"

int main(int argc, char const *argv[])
{
  uint8_t *packet;
  size_t len = sizeof(packet);
  char *hostname = "www.example.com";
  build_packet(packet, hostname);

  printf("%lu\n", len);

  printf("%s\n", packet);
}