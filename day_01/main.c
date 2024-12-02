#include <stdio.h>
#include <stdlib.h>

#include "../aocCommon.h"

#define BUFFER_LEN 24

enum
{
	LEFT_LIST = 0,
	RIGHT_LIST,
	NUM_LISTS
};

static int numCompare(const void *left, const void *right)
{
	return ((*(int *) left) - (*(int *) right));
}

int** slurpLists(const char *file_path, size_t *list_len)
{
	FILE *f_handle = NULL;
	int **ret = NULL;
	char line_buffer[BUFFER_LEN] = {0};
	size_t len = 0;
	size_t max = 10;

	if ((f_handle = fopen(file_path, "rb")) == NULL)
	{
		return NULL;	
	}

	if ((ret = malloc(sizeof(int *) * NUM_LISTS)) == NULL)
	{
		fclose(f_handle);

		return NULL;
	}

	AOC_NEW_DYN_ARR(int, ret[LEFT_LIST], max);
	AOC_NEW_DYN_ARR(int, ret[RIGHT_LIST], max);

	while (fgets(line_buffer, BUFFER_LEN, f_handle) != NULL)
	{
		int left, right;

		if (sscanf(line_buffer, "%d %d", &left, &right) != 2)
		{
			AOC_FREE(ret[LEFT_LIST]);
			AOC_FREE(ret[RIGHT_LIST]);
			AOC_FREE(ret);
			fclose(f_handle);

			return NULL;
		}

		if (len == max)
		{
			/* AOC_GROW_DYN_ARR modifies max so a temporary
			 * variable is needed to resize both to the same 
			 * length */
			size_t tmp_max = max;

			AOC_GROW_DYN_ARR(int, ret[LEFT_LIST], max);
			AOC_GROW_DYN_ARR(int, ret[RIGHT_LIST], tmp_max);
		}

		ret[LEFT_LIST][len] = left;
		ret[RIGHT_LIST][len] = right;
		len++;
	}

	*list_len = len;
	fclose(f_handle);
	qsort(ret[LEFT_LIST], len, sizeof(int), numCompare);
	qsort(ret[RIGHT_LIST], len, sizeof(int), numCompare);

	return ret;
}

static int differenceScore(int **lists, const size_t len)
{
	size_t i;
	int diff = 0;

	for (i = 0; i < len; i++)
	{
		diff += (AOC_ABS(lists[LEFT_LIST][i] - lists[RIGHT_LIST][i]));
	}

	return diff;
}

static int getRightCount(int *right_list, const int target, const size_t len)
{
	int count = 0;
	size_t found = leftBinSearch(right_list, len, &target, sizeof(int),
		numCompare);

	while ((found < len) 
	&& (right_list[found] == target))
	{
		count++;
		found++;
	}

	return count;	
}

static int similarityScore(int **lists, const size_t len)
{
	size_t i = 0;
	int running = 0;
	int count = 0;
	int current;

	if (len == 0)
	{
		return -1;
	}

	do
	{
		current = lists[LEFT_LIST][i];
		count = getRightCount(lists[RIGHT_LIST], current, len);

		while ((i < len) 
		&& (current == lists[LEFT_LIST][i]))
		{
			running += (current * count);	
			i++;
		}
	} while (i < len);

	return running;
}

int main(int argc, char **argv)
{
	int **vals = NULL;
	size_t len;

	if (argc < 2)
	{
		fprintf(stderr, "Please provide an input file\n");

		return AOC_FAILURE;
	}

	if ((vals = slurpLists(argv[1], &len)) == NULL)
	{
		fprintf(stderr, "Failed to slurp inputs\n");

		return AOC_FAILURE;
	}

	fprintf(stdout, "Cumulative difference: %u\n", 
		differenceScore(vals, len));
	fprintf(stdout, "Similarity score: %u\n",
		similarityScore(vals, len));

	AOC_FREE(vals[LEFT_LIST]);
	AOC_FREE(vals[RIGHT_LIST]);
	AOC_FREE(vals);

	return AOC_SUCCESS;
}

