#include "../lib/lpm/lpm.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

unsigned ipv;
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

void fillTable(lpm_root * root, char * source)
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

		lpm_add(root, ipv4num(destination), prefix_len, item->rule);

	}

	while(start != NULL)
	{
		item = start;
		start = start->next;
		free(item);
	}

	fclose(f);
}

double lookup(lpm_root * root, char * file)
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
		lpm_lookup(root, lookup_ip4);
		time_end = clock();
		time_sum += time_end - time_start;
		time_runs++;
	}

	fclose(f);

	return ((time_sum / (double) time_runs) * 1000) / CLOCKS_PER_SEC;
}

int main(int argc, char * argv[])
{
	char input_file[100];
	char lookup_file[100];

	lpm_root * root = NULL;

	if(argc != 4)
	{
		puts("./bench -v4/6 default_rule input_size");
		return 1;
	}

	ipv = strcmp("-v6", argv[1]) == 0 ? 6 : 4;

	sprintf(input_file, "./input/IPv%d/%s", ipv, argv[3]);
	sprintf(lookup_file, "./input/IPv%d/lookup", ipv);


	default_rule = atoi(argv[2]);
	root = lpm_init(default_rule);
	fillTable(root, input_file);
	printf("%lf\n", lookup(root, lookup_file));

	lpm_destroy(root);
}
