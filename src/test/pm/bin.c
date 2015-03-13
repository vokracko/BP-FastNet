#include "../../lib/pm/aho-corasic.h"


int main(int argc, char * argv[])
{
	unsigned fail = 0;
	unsigned rule;;
	char string[1024] = {'\0'};
	char cmd[20] = {'\0'};
	char line[2048] = {'\0'};
	unsigned match_count;
	_AC_RULE * matches = NULL;
	FILE * handle = fopen(argv[1], "r");

	int debug = argc == 3 && strcmp(argv[2], "debug") == 0;

	init();

	while(fscanf(handle, "%20s %1024s %u", cmd, string, &rule) == 3)
	{
		if(strcmp(cmd, "add") == 0)
		{
			add(string, rule);
			fail = 0;

			sprintf(line, "added  %s with rule  %d\n", string, rule);

		}
		else if(strcmp(cmd, "match") == 0)
		{
			fail = 1;
			matches = NULL;
			match_count = match(string, &matches);

			for(unsigned i = 0; i < match_count; ++i)
			{
				if(matches[i] == rule)
				{
					fail = 0;
					break;
				}
			}

			sprintf(line, "matched for %s %d - %s\n", string, rule, fail ? "FAIL" : "PASS");
		}

		if(debug) printf("%s", line);
		if(fail)
		{
			break;
		}
	}

	destroy();
	fclose(handle);

	return fail;
}
