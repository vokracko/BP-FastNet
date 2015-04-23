#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../../lib/lpm/lpm.h"

#define ADD ipv6 ? lpm6_add(root6, &addr6, first, second) : lpm_add(root, &addr, first, second)
#define INIT if(ipv6) root6 = lpm6_init(first); else root = lpm_init(first)
#define LOOKUP ipv6 ? lpm6_lookup(root6, &addr6) : lpm_lookup(root, &addr)
#define UPDATE ipv6 ? lpm6_update(root6, &addr6, first, second) : lpm_update(root, &addr, first, second)
#define REMOVE ipv6 ? lpm6_remove(root6, &addr6, first) : lpm_remove(root, &addr, first)
#define DESTROY ipv6 ? lpm6_destroy(root6) : lpm_destroy(root)
#define CONVERT if(ipv6) addr6 = ip2num6(ip); else addr = ip2num4(ip)

struct in6_addr ip2num6(char * address)
{
	struct in6_addr addr;
	uint32_t * ptr = (uint32_t *) &addr;
	_Bool res = inet_pton(AF_INET6, address, &addr);

	for(unsigned i = 0; i < 4; ++i) ptr[i] = htonl(ptr[i]);

	return addr;
}

struct in_addr ip2num4(char * address)
{
	struct in_addr addr;
	inet_pton(AF_INET, address, &addr);
	addr.s_addr = htonl(addr.s_addr);
	return addr;

}

int main(int argc, char * argv[])
{
	unsigned res;
	unsigned fail = 0;
	unsigned first, second;
	char ip[100] = {'\0'};
	char cmd[20] = {'\0'};
	char line[1024] = {'\0'};
	FILE * handle = fopen(argv[1], "r");
	lpm_root * root = NULL;
	lpm6_root * root6 = NULL;
	struct in_addr addr;
	struct in6_addr addr6;
	_Bool ipv6 = 0;

	int debug = argc == 3 && strcmp(argv[2], "debug") == 0;

	while(fscanf(handle, "%20s %99s %u %u", cmd, ip, &first, &second) == 4)
	{
		if(strcmp(cmd, "init") == 0)
		{
			if(second == 6) ipv6 = 1;
			DESTROY;
			sprintf(line, "init %d %d\n", first, second);
			INIT;
		}
		else if(strcmp(cmd, "lookup") == 0)
		{
			CONVERT;
			res = LOOKUP;
			fail = res != second;
			sprintf(line, "lookup for %s %d (%d) %s\n", ip, second, res, fail ? "FAIL" : "PASS");
		}
		else if(strcmp(cmd, "add") == 0)
		{
			CONVERT;
			ADD;
			sprintf(line, "add rule %d for %s/%d\n", second, ip, first);
		}
		else if(strcmp(cmd, "remove") == 0)
		{
			CONVERT;
			REMOVE;
			sprintf(line, "remove rule for %s/%d\n", ip, first);
		}
		else if(strcmp(cmd, "update") == 0)
		{
			CONVERT;
			UPDATE;
			sprintf(line, "update to rule %d for %s/%d\n", second, ip, first);
		}

		if(debug) printf("%s", line);
		if(fail)
		{
			break;
		}
	}

	DESTROY;
	fclose(handle);

	return fail;
}
