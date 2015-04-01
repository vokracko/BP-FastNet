#include "../../lib/pm/pm.h"
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

void free_keywords(pm_keyword * keywords, unsigned  index)
{
	for(unsigned i = 0; i <= index; ++i)
	{
		free(keywords[i].content);
	}
}

void fillKeywords(pm_root * root, char * source)
{
	FILE * f = fopen(source, "r");
	char keyword[1024];
	pm_keyword * keywords;
	unsigned rule;
	unsigned length;
	unsigned index = 0;
	unsigned size = 0;

	keywords = malloc(size * sizeof(pm_keyword));

	while(fscanf(f, "%1024s %d\n", keyword, &length, &rule) == 1)
	{
		if(index + 1 == size)
		{
			size += 10;
			keywords = realloc(keywords, size * sizeof(pm_keyword));
		}

		keywords[index].length = length;
		keywords[index].rule = rule;
		keywords[index].content = malloc(length);

		memcpy(keywords[index].content, keyword, length);

		index++;
	}

	pm_add(root, keywords, index);
	free_keywords(keywords, index);
	free(keywords);

	fclose(f);
}

double match(pm_root * root, char * file, bool first_only)
{
	clock_t time_start, time_end, time_sum = 0;
	unsigned time_runs = 0;
	char input[1500];
	unsigned length;
	pm_result * result = NULL;

	FILE * f = fopen(file, "r");

	while(fscanf(f, "%1500s", input, &length) == 2)
	{
		time_start = clock();
		result = pm_match(root, input, length);
		time_end = clock();
		time_sum += time_end - time_start;

		if(!first_only)
		{
			while(result != NULL)
			{
				time_start = clock();
				result = pm_match_next(root);
				time_end = clock();
				time_sum += time_end - time_start;
			}
		}

		time_runs++;
	}

	fclose(f);

	return ((time_sum / (double) time_runs) * 1000) / CLOCKS_PER_SEC;
}

int main(int argc, char * argv[])
{
	char input_file[100];
	char lookup_file[100];
	bool first_only;
	pm_root * root;

	if(argc != 4)
	{
		puts("./bench first/all keywords input");
		return 1;
	}

	first_only = strcmp(argv[1], "first") == 0;
	root = pm_init();

	fillKeywords(root, argv[2]);
	printf("%lf\n", match(root, argv[3], first_only));
	pm_destroy(root);
}
