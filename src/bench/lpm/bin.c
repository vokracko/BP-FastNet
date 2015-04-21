#include "../../lib/lpm/lpm.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

#ifdef IPv4
	#define ADDR struct in_addr
	#define PTON_FLAG AF_INET
	#define ADD lpm_add
	#define INIT lpm_init
	#define LOOKUP lpm_lookup
#endif

#ifdef IPv6
	#define ADDR struct in6_addr
	#define PTON_FLAG AF_INET6
	#define ADD lpm6_add
	#define INIT lpm6_init
	#define LOOKUP lpm6_lookup
#endif

unsigned ipv;
_LPM_RULE default_rule;

typedef struct _list
{
	struct _list * next;
	ADDR ip;
	_LPM_RULE rule;
} list;

ADDR ip2num(char * address)
{
	ADDR addr;
	inet_pton(PTON_FLAG, address, &addr);
	return addr;
}

void list_insert(list ** start, ADDR addr, _LPM_RULE * rule_max, _LPM_RULE * rule)
{
	list * item = *start;

	while(item != NULL)
	{
		if(item->ip.s_addr == addr.s_addr) break;
		item = item->next;
	}

	if(item == NULL)
	{
		item = (list *) malloc(sizeof(list));
		item->rule = (*rule_max)++;
		item->ip = addr;
		item->next = *start;
		*start = item;
	}

	*rule = item->rule;
}

void fillTable(lpm_root * root, char * source)
{
	FILE * f = fopen(source, "r");
	char destination[100];
	char next_hop[100];

	uint8_t prefix_len;
	_LPM_RULE rule_max = 1;
	_LPM_RULE rule;
	list * start = NULL;
	list * item = NULL;

	ADDR addr;

	while(fscanf(f, "%100s %d %100s", destination, &prefix_len, next_hop) == 3)
	{
		item = start;
		addr = ip2num(next_hop);
		list_insert(&start, addr, &rule_max, &rule);

		addr = ip2num(destination);
		ADD(root, &addr, prefix_len, rule);
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
	ADDR lookup_ip_num;

	clock_t time_start, time_end, time_sum = 0;
	unsigned time_runs = 0;
	FILE * f = fopen(file, "r");

	while(fscanf(f, "%100s", lookup_ip) == 1)
	{
		lookup_ip_num = ip2num(lookup_ip);
		time_start = clock();
		LOOKUP(root, &lookup_ip_num);
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
		puts("./alg -v4/6 default_rule input_file");
		return 1;
	}

	ipv = strcmp("-v6", argv[1]) == 0 ? 6 : 4;

	sprintf(input_file, "./input/IPv%d/%s", ipv, argv[3]);
	sprintf(lookup_file, "./input/IPv%d/lookup", ipv);

	default_rule = atoi(argv[2]);
	root = INIT(default_rule);
	fillTable(root, input_file);
	printf("%lf\n", lookup(root, lookup_file));

	lpm_destroy(root);
}
