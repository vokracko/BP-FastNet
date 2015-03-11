#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "../../lib/lpm/lpm.h"

uint32_t ip2int(char * address)
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
			res = lpm_lookup(root, ip2int(ip));
			fail = res != second;

			sprintf(line, "lookup for %s %d (%d) %s\n", ip, second, res, fail ? "FAIL" : "PASS");

		}
		else if(strcmp(cmd, "add") == 0)
		{
			lpm_add(root, ip2int(ip), first, second);

			sprintf(line, "add rule %d for %s/%d\n", second, ip, first);
		}
		else if(strcmp(cmd, "remove") == 0)
		{
			lpm_remove(root, ip2int(ip), first);

			sprintf(line, "remove rule for %s/%d\n", ip, first);
		}
		else if(strcmp(cmd, "update") == 0)
		{
			lpm_update(root, ip2int(ip), first, second);

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
