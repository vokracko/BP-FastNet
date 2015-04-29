#include <netinet/in.h>
#include <inttypes.h>

typedef union
{
	uint32_t number;
	uint16_t number16;
	char character;
	in_addr addr;
} phe_item;

typedef union
{
	uint32_t number;
	uint16_t number16;
	char character;
	in6_addr addr;
} phe6_item;

_Bool phe_get(char * input, phe_item * items, ...);
_Bool phe6_get(char * input, phe6_item * items, ...);

