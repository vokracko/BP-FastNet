#include "../../src/regex/regex.h"
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

extern char data[][4738];
#define DATA_COUNT 1000

#ifdef nfa
	#define MATCH regex_match_nfa
	#define DESTROY regex_destroy_nfa
	#define CONSTRUCT regex_construct_nfa
#endif

#ifdef dfa
	#define MATCH regex_match_dfa
	#define DESTROY regex_destroy_dfa
	#define CONSTRUCT regex_construct_dfa
#endif

void free_keywords(regex_pattern * keywords, unsigned  index)
{
	for(unsigned i = 0; i < index; ++i)
	{
		free(keywords[i].input);
	}
}

void * fillKeywords(char * source, unsigned * index)
{
	FILE * f = fopen(source, "r");
	char keyword[1024] = {0};
	regex_pattern * keywords = NULL;
	unsigned rule;
	unsigned length;
	unsigned size = 0;

	while(fscanf(f, "%1024s\n", keyword) == 1)
	{
		if(index + 1 >= size)
		{
			size += 10;
			keywords = realloc(keywords, size * sizeof(regex_pattern));
		}

		keywords[*index].length = strlen(keyword);
		keywords[*index].id = *index+1;
		keywords[*index].input = malloc(strlen(keyword)+1);
		keywords[*index].input[strlen(keyword)] = '\0';

		memcpy(keywords[*index].input, keyword, strlen(keyword));

		(*index)++;

		bzero(keyword, 1024);
	}

	fclose(f);

	return keywords;
}

double match(void * root)
{
	clock_t time_start, time_end, time_sum = 0;
	unsigned time_runs = 0;
	unsigned length;

	for(unsigned i = 0; i <DATA_COUNT; ++i)
	{
		length = (((unsigned char) data[i][0]) << 8) | ((unsigned char) data[i][1]);
		time_start = clock();
		MATCH(root, data[i]+2, length);
		time_end = clock();
		time_sum += time_end - time_start;

		time_runs++;
	}

	return ((time_sum / (double) time_runs) * 1000) / CLOCKS_PER_SEC;
}

int main(int argc, char * argv[])
{
	char input_file[100];
	char lookup_file[100];
	bool first_only;
	void * root;
	unsigned index = 0;
	regex_pattern * keywords;

	if(argc != 2)
	{
		puts("./bench keyword-file");
		return 1;
	}

	keywords = fillKeywords(argv[1], &index);
	root = CONSTRUCT(keywords, index);
	free_keywords(keywords, index);
	free(keywords);

	printf("%lf\n", match(root));

	DESTROY(root);
}
