#include "../../lib/pm/aho-corasic.h"


void free_keywords(pm_keyword * keywords, unsigned * count)
{
	for(unsigned i = 0; i < *count; ++i)
	{
		free(keywords[i].text);
	}

	*count = 0;

}

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
	pm_root * root;
	unsigned count = 0;
	unsigned size = 10;
	unsigned length;
	pm_keyword * keywords = malloc(size * sizeof(pm_keyword));

	int debug = argc == 3 && strcmp(argv[2], "debug") == 0;

	root = init();

	while(fscanf(handle, "%20s %1024s %u %u", cmd, string, &length, &rule) == 4)
	{
		if(strcmp(cmd, "add") == 0)
		{
			fail = 0;

			if(count + 1 == size)
			{
				size += 10;
				keywords = realloc(keywords, size);
			}

			keywords[count].rule = rule;
			keywords[count].length = length;
			keywords[count].text = malloc(length);
			memcpy(keywords[count].text, string, length);
			++count;

			sprintf(line, "added  %s with rule %d\n", string, rule);

		}
		else if(strcmp(cmd, "commit") == 0)
		{
			add(root, keywords, count);
			fail = 0;
			free_keywords(keywords, &count);
			sprintf(line, "rules commited to search struture\n");
		}
		else if(strcmp(cmd, "match") == 0)
		{
			fail = 1;
			matches = NULL;
			match_count = match(root, string, length, &matches);

			for(unsigned i = 0; i < match_count; ++i)
			{
				if(matches[i] == rule)
				{
					fail = 0;
					//break;
				}
			}

			// test where rule should not be found
			if(rule == NONE) fail = 0;

			sprintf(line, "matched for %s %d - %s\n", string, rule, fail ? "FAIL" : "PASS");
		}
		else if(strcmp(cmd, "remove") == 0)
		{
			fail = 0;
			pm_remove(root, string, length);
			sprintf(line, "removed %s\n", string);
		}

		if(debug) printf("%s", line);
		if(fail)
		{
			break;
		}
	}

	free(keywords);

	destroy(root);
	fclose(handle);

	return fail;
}
