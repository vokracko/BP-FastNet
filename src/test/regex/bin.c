#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "../../lib/regex/regex.h"

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

void free_patterns(regex_pattern patterns[], unsigned * count)
{
	for(unsigned i = 0; i < *count; ++i)
	{
		free(patterns[i].input);
	}

	*count = 0;
}

int main(int argc, char * argv[])
{
	char line[2048] = {'\0'};
	char string[1024] = {'\0'};
	char cmd[20] = {'\0'};

	int id;
	unsigned length;
	unsigned fail = 0;
	unsigned count = 0;
	unsigned size = 10;

	FILE * handle = fopen(argv[1], "r");
	int result;

	regex_root * root;
	regex_pattern * patterns = malloc(sizeof(regex_pattern) * size);


	int debug = argc == 3 && strcmp(argv[2], "debug") == 0;

	while(fscanf(handle, "%20s %1024s %u %u", cmd, string, &length, &id) == 4)
	{
		handle_escape(string);

		if(cmd[0] == '#') continue;

		if(strcmp(cmd, "add") == 0)
		{
			if(count + 1 == size)
			{
				size += 10;
				patterns = realloc(patterns, sizeof(regex_pattern) * size);
			}

			patterns[count].input = malloc(length);
			memcpy(patterns[count].input, string, length);
			patterns[count].length = length;
			patterns[count].id = id;
			count++;

			sprintf(line, "added %s with id %d\n", string, id);
		}

		if(strcmp(cmd, "commit") == 0)
		{
			root = regex_construct(patterns, count);
			fail = root == NULL && id == -1;
			free_patterns(patterns, &count);

			sprintf(line, "commit %s\n", fail ? "FAIL" : "PASS");
		}

		if(strcmp(cmd, "match") == 0)
		{
			result = regex_match(root, string, length);
			fail = result != id;
			sprintf(line, "matching againts %s with result %s\n", string, fail ? "FAIL" : "PASS");
		}

		if(debug) printf("%s", line);
		if(fail)
		{
			break;
		}
	}

	regex_destroy(root);
	free(patterns);
	fclose(handle);

	return fail;
}
