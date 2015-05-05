#include <netinet/in.h>
#include <inttypes.h>

typedef union
{
	uint32_t number;
	uint16_t number16;
	char character;
	struct in_addr addr;
} phe_item;

typedef union
{
	uint32_t number;
	uint16_t number16;
	char character;
	struct in6_addr addr;
} phe6_item;

enum
{
	VERSION = 0,
};

enum
{
	IPv4_TOS = 1,
	IPv4_LENGTH = 2,
	IPv4_TTL = 8,
	IPv4_PROTOCOL = 9,
	IPv4_CRC = 10,
	IPv4_SRC = 12,
	IPv4_DST = 16
} IPv4;

enum
{
	IPv6_LENGTH = 4,
	IPv6_NEXT_HEADER = 6,
	IPv6_HOP_LIMIT = 7,
	IPv6_SRC = 8,
	IPv6_DST = 24
} IPv6;

_Bool phe_get(char * input, phe_item * items, ...);
_Bool phe6_get(char * input, phe6_item * items, ...);

