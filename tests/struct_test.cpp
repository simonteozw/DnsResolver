#include <assert.h>

static_assert(sizeof(String) == 16, "wrong size");
static_assert(sizeof(Stream) == 16, "wrong size");
static_assert(sizeof(struct dns_header) == 12, "wrong size");
static_assert(sizeof(struct dns_question) == 16, "wrong size");
static_assert(sizeof(struct dns_record) == 32, "wrong size");
static_assert(sizeof(struct dns_packet) == 48, "wrong size");