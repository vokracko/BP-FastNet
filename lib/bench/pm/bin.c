#include "../../src/pm/pm.h"
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define DATA_COUNT 1000
extern char data[][4738];

void free_keywords(pm_keyword * keywords, unsigned  index)
{
	for(unsigned i = 0; i < index; ++i)
	{
		free(keywords[i].content);
	}
}

void fillKeywords(pm_root * root, char * source)
{
	FILE * f = fopen(source, "r");
	char keyword[1024] = {0};
	pm_keyword * keywords;
	unsigned rule;
	unsigned length;
	unsigned index = 0;
	unsigned size = 0;

	keywords = malloc(size * sizeof(pm_keyword));

	while(fscanf(f, "%1024s\n", keyword) == 1)
	{
		if(index + 1 >= size)
		{
			size += 10;
			keywords = realloc(keywords, size * sizeof(pm_keyword));
		}

		keywords[index].length = strlen(keyword);
		keywords[index].rule = index+1;
		keywords[index].content = malloc(strlen(keyword)+1);
		keywords[index].content[strlen(keyword)] = '\0';

		memcpy(keywords[index].content, keyword, strlen(keyword));

		index++;

		bzero(keyword, 1024);
	}


	pm_add(root, keywords, index);
	free_keywords(keywords, index);
	free(keywords);

	fclose(f);
}

double match(pm_root * root, bool first_only)
{
	clock_t time_start, time_end, time_sum = 0;
	unsigned time_runs = 0;
	char input[1500];
	unsigned length;
	pm_result * result = pm_result_init();
	_Bool res;

	for(unsigned i = 0; i <DATA_COUNT; ++i)
	{
		length = (((unsigned char) data[i][0]) << 8) | ((unsigned char) data[i][1]);
		time_start = clock();
		res = pm_match(root, data[i]+2, length, result);
		time_end = clock();
		time_sum += time_end - time_start;

		if(!first_only)
		{
			while(res)
			{
				time_start = clock();
				res = pm_match_next(result);
				time_end = clock();
				time_sum += time_end - time_start;
			}
		}

		time_runs++;
	}

	pm_result_destroy(result);
	return ((time_sum / (double) time_runs) * 1000) / CLOCKS_PER_SEC;
}

int main(int argc, char * argv[])
{
	char input_file[100];
	char lookup_file[100];
	bool first_only;
	pm_root * root;

	if(argc != 3)
	{
		puts("./bench first/all keyword-file");
		return 1;
	}

	first_only = strcmp(argv[1], "first") == 0;
	root = pm_init();

	fillKeywords(root, argv[2]);
	printf("%lf\n", match(root, first_only));
	pm_destroy(root);
}
