#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../../lib/lpm/lpm.h"

struct in_addr ipv4num(char * address)
{
	struct in_addr addr;
	inet_pton(AF_INET, address, &addr);
	addr.s_addr = htonl(addr.s_addr);
	return addr;
}

void ipv6num(char * address)
{
	struct in6_addr addr;
	inet_pton(AF_INET6, address, &addr);
}

int main(int argc, char * argv[])
{
	unsigned res;
	unsigned fail = 0;
	unsigned first, second;
	char ip[20] = {'\0'};
	char cmd[20] = {'\0'};
	char line[1024] = {'\0'};
	FILE * handle = fopen(argv[1], "r");
	lpm_root * root = NULL;
	struct in_addr addr;

	int debug = argc == 3 && strcmp(argv[2], "debug") == 0;

	while(fscanf(handle, "%20s %20s %u %u", cmd, ip, &first, &second) == 4)
	{
		if(strcmp(cmd, "init") == 0)
		{
			if(root != NULL) lpm_destroy(root);
			sprintf(line, "init %d %d\n", first, second);
			root = lpm_init(first);
		}
		else if(strcmp(cmd, "lookup") == 0)
		{
			addr =  ipv4num(ip);
			res = lpm_lookup(root, &addr);
			fail = res != second;

			sprintf(line, "lookup for %s %d (%d) %s\n", ip, second, res, fail ? "FAIL" : "PASS");

		}
		else if(strcmp(cmd, "add") == 0)
		{
			addr =  ipv4num(ip);
			lpm_add(root, &addr, first, second);

			sprintf(line, "add rule %d for %s/%d\n", second, ip, first);
		}
		else if(strcmp(cmd, "remove") == 0)
		{
			addr =  ipv4num(ip);
			lpm_remove(root, &addr, first);

			sprintf(line, "remove rule for %s/%d\n", ip, first);
		}
		else if(strcmp(cmd, "update") == 0)
		{
			addr =  ipv4num(ip);
			lpm_update(root, &addr, first, second);

			sprintf(line, "update to rule %d for %s/%d\n", second, ip, first);
		}

		if(debug) printf("%s", line);
		if(fail)
		{
			break;
		}
	}

	lpm_destroy(root);
	fclose(handle);

	return fail;
}
