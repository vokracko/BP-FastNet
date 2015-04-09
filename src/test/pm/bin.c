#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "../../lib/pm/pm.h"


void free_keywords(pm_keyword * keywords, unsigned * count)
{
	for(unsigned i = 0; i < *count; ++i)
	{
		free(keywords[i].content);
	}

	*count = 0;

}

void handle_escape(char * input)
{
	unsigned pos = 0;
	unsigned len = strlen(input);
	unsigned i = 0;
	unsigned hexnum;

	while(i < len)
	{
		if(input[i] == '\\')
		{
			switch(input[i + 1])
			{
				case 'x':
					sscanf(input + i + 2, "%2x", &hexnum);
					input[pos] = hexnum;
					i += 3;
					break;

				case '\\':
					input[pos] = '\\';
					++i;
					break;

				case 'n':
					input[pos] = '\n';
					++i;
					break;

				case 'r':
					input[pos] = '\n';
					++i;
					break;

				case 't':
					input[pos] = '\n';
					++i;
					break;

				case '0':
					input[pos] = '\0';
					++i;
					break;

				default:
					input[pos] = '\\';
					--i;
					break;
			}

		}
		else
		{
			input[pos] = input[i];
		}

		++pos;
		++i;
	}
}

int main(int argc, char * argv[])
{
	unsigned fail = 0;
	unsigned rule;;
	char string[1024] = {'\0'};
	char cmd[20] = {'\0'};
	char line[2048] = {'\0'};
	pm_result * result = NULL;
	FILE * handle = fopen(argv[1], "r");
	pm_root * root;
	unsigned count = 0;
	unsigned size = 10;
	unsigned length;
	pm_keyword * keywords = malloc(size * sizeof(pm_keyword));

	int debug = argc == 3 && strcmp(argv[2], "debug") == 0;

	root = pm_init();

	while(fscanf(handle, "%20s %1024s %u %u", cmd, string, &length, &rule) == 4)
	{
		handle_escape(string);

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
			keywords[count].content = malloc(length);
			memcpy(keywords[count].content, string, length);
			++count;

			sprintf(line, "added %s with rule %d\n", string, rule);

		}
		else if(strcmp(cmd, "commit") == 0)
		{
			pm_add(root, keywords, count);
			fail = 0;
			free_keywords(keywords, &count);
			sprintf(line, "rules commited to search structure\n");
		}
		else if(strcmp(cmd, "match") == 0)
		{
			fail = 1;
			result = pm_match(root, string, length);

			// test where rule should not be found
			if(rule == PM_RULE_NONE && result == NULL) fail = 0;

			while(result != NULL && fail == 1)
			{
				if(debug) puts("match_next");
				for(unsigned i = 0; i < result->count; ++i)
				{
					if(result->rule[i] == rule)
					{
						fail = 0;
					}
				}

				result = pm_match_next(root);
			}

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

	pm_destroy(root);
	fclose(handle);

	return fail;
}
