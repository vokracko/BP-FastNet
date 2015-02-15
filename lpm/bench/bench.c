#include "../lib/lpm.h"
#include <stdio.h>

unsigned ipv6;
_LPM_RULE default_rule;

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

void fillTable(char * source)
{
	FILE * f = fopen(source, "r");
	char destination[100];
	char next_hop[100];
	uint32_t next_hop4;
	// uint32_t next_hop6;
	uint8_t prefix_len;
	uint8_t rule_max = 1;
	list * start = NULL;
	list * item = NULL;

	while(fscanf(f, "%100s %d %100s", destination, &prefix_len, next_hop) == 3)
	{
		item = start;
		next_hop4 = ipv4num(next_hop);

		while(item != NULL)
		{
			if(item->ip == next_hop4) break;
			item = item->next;
		}

		if(item == NULL)
		{
			item = (list *) malloc(sizeof(list));
			item->rule = rule_max++;
			item->ip = next_hop4;
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

	fclose(f);
}

double lookup(char * file)
{
	char lookup_ip[100];
	uint32_t lookup_ip4;

	clock_t time_start, time_end, time_sum = 0;
	unsigned time_runs = 0;
	FILE * f = fopen(file, "r");

	while(fscanf(f, "%100s", lookup_ip) == 1)
	{

		lookup_ip4 = ipv4num(lookup_ip);
		time_start = clock();
		lpm_lookup(lookup_ip4);
		time_end = clock();
		time_sum += time_end - time_start;
		time_runs++;
	}

	return ((time_sum / (double) time_runs) * 1000) / CLOCKS_PER_SEC;
}

int main(int argc, char * argv[])
{

	if(argc != 5)
	{
		puts("./bench -v4/6 default_rule source_file lookup_file");
		return 1;
	}


	lpm_init(0, 0);
	ipv6 = strcmp("-v6", argv[1]);
	default_rule = atoi(argv[2]);
	fillTable(argv[3]);
	printf("avg %lfms\n", lookup(argv[4]));

	lpm_destroy();
}
