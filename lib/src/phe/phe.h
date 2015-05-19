#include <netinet/in.h>
#include <inttypes.h>

typedef union
{
	uint32_t number;
	uint16_t number16;
	char character;
	struct in_addr addr;
} phe_item;

enum
{
	IPv4_VERSION = 0,
	IPv4_SRC = 12,
	IPv4_DST = 16
	TCP_SRC_PORT = 0,
	TCP_DST_PORT = 2
};


_Bool phe_get(char * input, phe_item * items, ...);

