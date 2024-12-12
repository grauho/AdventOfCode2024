#include <stdio.h>
#include <stdlib.h>

#include "../aocCommon.h"

/* Rules governing stone behavior, proceeding in order:
 * 	1) If a stone's value is 0 replace it with 1
 * 	2) If a stone's value has an even number of digits, eg 1122 (4 digits), 
 * 	   split that stone into two stones, eg: 11 and 22 
 * 	3) default, the stone's value is replaced with 2024
 *
 * I wonder if it's worth caching the results for a given number in part two
 * and returning its state at its first non-part 2 branch. This would also
 * require there to be enough steps remaining to do all the intermediate 
 * splits. 
 */

static long int getLeftValue(const long int val, const unsigned int digits)
{
	unsigned long int lim = digits >> 1;
	unsigned long int tmp = 1;

	while (lim > 0)
	{
		tmp *= 10;
		lim--;
	}

	return val % tmp;
}

static long int getRightValue(const long int val, const unsigned int digits)
{
	unsigned long int lim = digits >> 1;
	unsigned long int tmp = 1;

	while (lim > 0)
	{
		tmp *= 10;
		lim--;
	}

	return val / tmp;
}

static unsigned int numDigits(const long int input)
{
	const unsigned long int target = AOC_ABS(input);
	unsigned int digits = 1;
	unsigned long int tmp = 10;

	while (tmp <= target)
	{
		digits++;
		tmp *= 10;
	}

	return digits;
}

/* The obvious solution, works fine for smallish number of steps but is 
 * wholely inadequate for part 2 */
static long int getStoneValue(const long int val, const int step)
{
	long int ret = 0;
	unsigned int digits = 0;

	if (step == 0)
	{
		ret++;
	}
	else if (val == 0)
	{
		if (step == 1)
		{
			ret++;
		}
		else
		{
			ret += getStoneValue(2024, step - 2);
		}
	}
	else if (((digits = numDigits(val)) % 2) == 0)
	{
		ret += getStoneValue(getLeftValue(val, digits), step - 1);
		ret += getStoneValue(getRightValue(val, digits), step - 1);
	}
	else
	{
		ret += getStoneValue(val * 2024, step - 1);
	}

	return ret;
}

/* XXX: Expects only positive values in the input file, additional handling 
 * could be added for negatives denoted with a leading minus (-) sign but
 * that is outside the purview of this particular problem */

/* One possibly use calculate the number of stones here directly from the 
 * parsed running values without pushing them to an array */
static AOC_STAT parseInput(FILE *input, int **out, size_t *out_len)
{
	int *working = NULL;
	size_t len = 0;
	size_t max = 10;
	int running = 0;
	int ch;
	AOC_BOOL active = AOC_FALSE;

	if ((input == NULL) || (out == NULL) || (out_len == NULL))
	{
		return AOC_FAILURE;
	}

	AOC_NEW_DYN_ARR(int, working, max);

	while ((ch = fgetc(input)) != EOF)
	{
		if ((ch >= '0') && (ch <= '9'))
		{
			running *= 10;
			running += ch - '0';
			active = AOC_TRUE;
		}
		else if (ch == ' ')
		{
			AOC_CAT_DYN_ARR(int, working, len, max, running);
			running = 0;
			active = AOC_FALSE;
		}
		else
		{
			break;
		}
	}

	/* Flush any remaining running value to the array */
	if (active == AOC_TRUE)
	{
		AOC_CAT_DYN_ARR(int, working, len, max, running);
		running = 0;
	}

	*out = working;
	*out_len = len;

	return AOC_SUCCESS;
}

static void dumpArray(const int * const array, const size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
	{
		fprintf(stdout, "%d (%d) ", array[i], numDigits(array[i]));
	}

	fputc('\n', stdout);
}

static size_t obviousSolution(const int * const array, const size_t len, 
	const size_t steps)
{
	size_t i, stones = 0;

	for (i = 0; i < len; i++)
	{
		stones += getStoneValue(array[i], steps);	
	}

	return stones;
}

int main(int argc, char **argv)
{
	size_t i;

	if (argc < 2)
	{
		fputs("Please provide an input file or files to act upon\n",
			stderr);

		return AOC_FAILURE;
	}

	for (i = 1; i < argc; i++)
	{
		FILE *input = NULL;
		int *vals = NULL;
		size_t val_len = 0;

		if ((input = fopen(argv[i], "rb")) == NULL)
		{
			fprintf(stderr, "Failed to open file '%s'\n", argv[i]);

			continue;
		}

		if (parseInput(input, &vals, &val_len) == AOC_FAILURE)
		{
			fprintf(stderr, "Failed to parse file '%s'\n", 
				argv[i]);

			fclose(input);
			AOC_FREE(vals);

			continue;
		}

		fprintf(stdout, "Part 1: %ld stones\n", 
			obviousSolution(vals, val_len, 25));
		/*
		fprintf(stdout, "Part 2: %ld stones\n", 
			obviousSolution(vals, val_len, 75));
		*/

		fclose(input);
		AOC_FREE(vals);
	}

	return AOC_SUCCESS;
}

