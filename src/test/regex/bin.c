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
	unsigned number;
	char string[1024] = {'\0'};
	char cmd[20] = {'\0'};
	char line[2048] = {'\0'};
	FILE * handle = fopen(argv[1], "r");
	unsigned length;
	int result;
	regex_pattern * pattern;


	int debug = argc == 3 && strcmp(argv[2], "debug") == 0;

	while(fscanf(handle, "%20s %1024s %u %u", cmd, string, &length, &number) == 4)
	{

		if(strcmp(cmd, "construct") == 0)
		{
			sprintf(line, "constructed %s\n", string);
			handle_escape(string);

			pattern = parse(string, length, number);
			fail = pattern == NULL;
			// if(!fail) regex_free(pattern);
		}

		if(strcmp(cmd, "match") == 0)
		{
			handle_escape(string);

			result = match(pattern, string, length);
			fail = result != number;
			sprintf(line, "matching againts %s with result %s\n", string, fail ? "FAIL" : "PASS");
		}

		if(debug) printf("%s", line);
		if(fail)
		{
			break;
		}
	}

	regex_free(pattern);
	fclose(handle);

	return fail;
}
