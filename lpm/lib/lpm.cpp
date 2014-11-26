#include "lpm.h"

uint32_t calculate_hash(char * key, size_t len)
{
	uint32_t hash, i;

	for(hash = i = 0; i < len; ++i)
	{
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

void lpm_init(uint8_t lookup_function, uint8_t hash_function)
{

}
