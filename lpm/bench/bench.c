#include "../lib/lpm.h"

typedef struct _list
{
	struct _list * next;
	uint32_t ip;
	uint8_t rule;
} list;

uint32_t ipv4num(char * address)
{
	uint32_t ip = 0;
	uint8_t octet = 0;

	for(int i = 0; i < 4; ++i)
	{
		octet = atoi(address);
		ip |= octet << ((3 - i) * 8);
		address = strchr(address, '.') + 1;

	}

	return ip;
}

/**
 * @todo
 */
uint32_t ipv6num(char * address)
{
	return 0;
}

void fillTable()
{
	char destination[100];
	char next_hop[100];
	uint32_t next_hop4;
	// uint32_t next_hop6;
	uint8_t prefix_len;
	uint8_t rule_max = 1;
	list * start = NULL;
	list * item = NULL;

	while(scanf("%100s %d %100s", destination, &prefix_len, next_hop) == 3)
	{
		item = start;
		next_hop4 = ipv4num(next_hop);

		while(item != NULL)
		{
			if(item->ip == next_hop_ip) break;
			item = item->next;
		}

		if(item == NULL)
		{
			item = (list *) malloc(sizeof(list));
			item->rule = rule_max++;
			item->ip = next_hop_ip;
			item->next = start;
			start = item;
		}

		lpm_add(ipv4num(destination), prefix_len, item->rule);

	}

	while(start != NULL)
	{
		item = start;
		start = start->next;
		free(item);
	}
}

int main(int argc, char * argv[])
{

	char lookup_ip[100];

	clock_t time_start, time_end, time_sum = 0;
	unsigned time_runs = 0;

	lpm_init(0, 0);
	fillTable();

	while(scanf("%100s", lookup_ip) == 1)
	{

		lookup_ip4 = ipv4num(lookup_ip);
		time_start = clock();
		lpm_lookup(lookup_ip4);
		time_end = clock();
		time_sum += time_end - time_start;
		time_runs++;
	}

	printf("%lf\n", ((time_sum / (double) time_runs) * 1000) / CLOCKS_PER_SEC);

	lpm_destroy();
}
