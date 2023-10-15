#include <assert.h>
#include "../structs.h"

int main(int argc, char *argv[])
{
  static_assert(sizeof(String) == 16, "String is wrong size");
  static_assert(sizeof(Stream) == 16, "Stream is wrong size");
  static_assert(sizeof(dns_header_t) == 12, "Header is wrong size");
  static_assert(sizeof(dns_question_t) == 16, "Question is wrong size");
  static_assert(sizeof(dns_record_t) == 32, "Record is wrong size");
  static_assert(sizeof(dns_packet_t) == 48, "Packet is wrong size");
}