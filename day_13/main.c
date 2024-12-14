#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aocCommon.h"

struct coordinate
{
	long int x;
	long int y;
};

struct entry
{
	struct coordinate a_button;
	struct coordinate b_button;
	struct coordinate prize;
};

/* Normally sscanf is a safer choice than fscanf, but for expediency I will
 * use the latter here. */
static AOC_STAT parseFile(FILE *input, struct entry **out, size_t *out_len)
{
	const char format_str[] = "Button A: X+%ld, Y+%ld\n"
		"Button B: X+%ld, Y+%ld\n"
		"Prize: X=%ld, Y=%ld\n";
	struct entry tmp = {0};
	struct entry *working = NULL;
	size_t len = 0;
	size_t max = 10;
	int num_parsed = 0;

	if ((input == NULL) || (out == NULL) || (out_len == NULL))
	{
		return AOC_FAILURE;
	}

	AOC_NEW_DYN_ARR(struct entry, working, max);

	while ((num_parsed = fscanf(input, format_str, 
		&tmp.a_button.x, &tmp.a_button.y, 
		&tmp.b_button.x, &tmp.b_button.y, 
		&tmp.prize.x, &tmp.prize.y)) == 6)
	{
		AOC_CAT_DYN_ARR(struct entry, working, len, max, tmp);
	}

	*out = working;
	*out_len = len;

	return AOC_SUCCESS;
}

/* The coordinate type for 'out' is being used for convenience with the 
 * understanding that x is 'A' presses and y is 'B' presses */
static AOC_BOOL cramersRule(const struct coordinate button_a, 
	const struct coordinate button_b, const struct coordinate prize,
	struct coordinate *out)
{
	const long int denominator 
		= (button_a.x * button_b.y - button_a.y * button_b.x);
	const long int numerator_a 
		= ((prize.x * button_b.y) - (prize.y * button_b.x));
	const long int numerator_b 
		= ((prize.y * button_a.x) - (prize.x * button_a.y));
	
	if ((numerator_a % denominator == 0)
	&& (numerator_b % denominator == 0))
	{
		out->x = numerator_a / denominator;
		out->y = numerator_b / denominator;

		return AOC_SUCCESS;
	}
	else
	{
		return AOC_FAILURE;
	}
}

static long int getTotalCoins(const struct entry * const arr, 
	const size_t len, const long int offset)
{
	long int total = 0;
	size_t i;

	for (i = 0; i < len; i++)
	{
		struct coordinate prize = arr[i].prize;
		struct coordinate tmp = {0};

		prize.x += offset;
		prize.y += offset;

		if (cramersRule(arr[i].a_button, arr[i].b_button, prize, &tmp) 
			== AOC_SUCCESS)
		{
			total += (tmp.x * 3 + tmp.y);
		}
	}

	return total;
}

#define PART_2_OFFSET (10000000000000)

int main(int argc, char **argv)
{
	size_t i;

	if (argc < 2)
	{
		fputs("Please provide a file or files to act upon\n", stderr);

		return AOC_FAILURE;
	}

	for (i = 1; i < argc; i++)
	{
		FILE *input = NULL;
		struct entry *entry_arr = NULL;
		size_t entry_len;

		if ((input = fopen(argv[i], "rb")) == NULL)
		{
			fprintf(stderr, "Failed to open file '%s'\n", argv[i]);

			continue;
		}

		if (parseFile(input, &entry_arr, &entry_len) == AOC_FAILURE)
		{
			fprintf(stderr, "Failed to parse file values, '%s'\n", 
				argv[i]);

			fclose(input);
			AOC_FREE(entry_arr);
		}

		fprintf(stdout, "Part 1: %ld\n", 
			getTotalCoins(entry_arr, entry_len, 0));
		fprintf(stdout, "Part 2: %ld\n", 
			getTotalCoins(entry_arr, entry_len, PART_2_OFFSET));
		fclose(input);
		AOC_FREE(entry_arr);
	}

	return AOC_SUCCESS;
}

