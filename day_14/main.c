#include <stdio.h>
#include <stdlib.h>

#include "../aocCommon.h"

#define TIME_STEPS    (100)
#define TIME_STEP_MAX (10000)

struct coordinate
{
	long int x_pos;
	long int y_pos;
};

struct robot
{
	struct coordinate position;
	struct coordinate velocity;
};

static AOC_STAT parseInput(FILE *input, struct robot **out, size_t *out_len)
{
	const char format[] = "p=%ld,%ld v=%ld,%ld\n";
	struct robot *working = NULL;
	struct robot tmp;
	size_t len = 0;
	size_t max = 10;

	if ((input == NULL) || (out == NULL) || (out_len == NULL))
	{
		return AOC_FAILURE;
	}

	AOC_NEW_DYN_ARR(struct robot, working, max);

	while (fscanf(input, format, &tmp.position.x_pos, &tmp.position.y_pos, 
		&tmp.velocity.x_pos, &tmp.velocity.y_pos) == 4)
	{
		AOC_CAT_DYN_ARR(struct robot, working, len, max, tmp);
	}

	*out = working;
	*out_len = len;

	return AOC_SUCCESS;
}

static void dumpArr(const struct robot * const arr, const size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
	{
		fprintf(stdout, "%4lu: p=%4ld,%4ld v=%4ld,%4ld\n", i,
			arr[i].position.x_pos, arr[i].position.y_pos,
			arr[i].velocity.x_pos, arr[i].velocity.y_pos);
	}
}

static int getQuadrant(const struct coordinate final, 
	const struct coordinate bounds)
{
	const AOC_BOOL x_even = ((bounds.x_pos % 2) == 0);
	const AOC_BOOL y_even = ((bounds.y_pos % 2) == 0);
	const long int x_mid = bounds.x_pos >> 1;
	const long int y_mid = bounds.y_pos >> 1;
	const int x_res = (final.x_pos > x_mid);
	const int y_res = (final.y_pos > y_mid) * 2;

	if (((x_even == AOC_FALSE) && (final.x_pos == x_mid))
	|| ((y_even == AOC_FALSE) && (final.y_pos == y_mid)))
	{
		return -1;
	}

	return x_res + y_res;
}

static long int calculatePart1(const struct robot * const arr, 
	const size_t len, const struct coordinate bounds)
{
	long int total = -1;
	int quadrants[4] = {0};
	size_t i;

	if (arr == NULL)
	{
		return -1;
	}

	for (i = 0; i < len; i++)
	{
		struct coordinate final = {0};
		int tmp;

		final.x_pos = (arr[i].position.x_pos 
			+ (arr[i].velocity.x_pos * TIME_STEPS)) % bounds.x_pos;
		final.y_pos = (arr[i].position.y_pos 
			+ (arr[i].velocity.y_pos * TIME_STEPS)) % bounds.y_pos;

		if (final.x_pos < 0)
		{
			final.x_pos += bounds.x_pos;
		}

		if (final.y_pos < 0)
		{
			final.y_pos += bounds.y_pos;
		}

		if ((tmp = getQuadrant(final, bounds)) >= 0)
		{
			quadrants[tmp]++;
		}
	}

	for (i = 0; i < 4; i++)
	{
		if (total == -1)
		{
			total = quadrants[i];
		}
		else
		{
			total *= quadrants[i];
		}
	}

	return total;
}

static int compareRobots(const void *foo, const void *bar)
{
	const struct robot *left = (struct robot *) foo;
	const struct robot *right = (struct robot *) bar;

	if (left->position.y_pos != right->position.y_pos)
	{
		return left->position.y_pos - right->position.y_pos;
	}
	else
	{
		return left->position.x_pos - right->position.x_pos;
	}
}

static void dumpImage(struct robot *arr, const size_t len, 
	const struct coordinate bounds)
{
	size_t i, j;

	if (arr == NULL)
	{
		return;
	}

	qsort(arr, len, sizeof(struct robot), compareRobots);

	for (j = 0; j < bounds.y_pos; j++)
	{
		for (i = 0; i < bounds.x_pos; i++)
		{
			struct robot key;

			key.position.x_pos = i;
			key.position.y_pos = j;

			if (bsearch(&key, arr, len, sizeof(struct robot),
				compareRobots) != NULL)
			{
				fputc('#', stdout);
			}
			else
			{
				fputc(' ', stdout);
			}
		}

		fputc('\n', stdout);
	}

	fputc('\n', stdout);
}

static long int estimatePart2(struct robot *arr, const size_t len, 
	const struct coordinate bounds)
{	
	long int lowest = -1;
	size_t i, j, best;

	if (arr == NULL)
	{
		return -1;
	}

	for (j = 0; j < TIME_STEP_MAX; j++)
	{
		int quadrants[4] = {0};
		int total = -1;

		for (i = 0; i < len; i++)
		{
			struct coordinate *final = &arr[i].position;
			int tmp;

			final->x_pos = (arr[i].position.x_pos 
				+ arr[i].velocity.x_pos) % bounds.x_pos;
			final->y_pos = (arr[i].position.y_pos 
				+ arr[i].velocity.y_pos) % bounds.y_pos;

			if (final->x_pos < 0)
			{
				final->x_pos += bounds.x_pos;
			}

			if (final->y_pos < 0)
			{
				final->y_pos += bounds.y_pos;
			}

			if ((tmp = getQuadrant(*final, bounds)) >= 0)
			{
				quadrants[tmp]++;
			}
		}

		for (i = 0; i < 4; i++)
		{
			if (total == -1)
			{
				total = quadrants[i];
			}
			else
			{
				total *= quadrants[i];
			}
		}

		if ((lowest == -1) || (total < lowest))
		{
			dumpImage(arr, len, bounds);
			lowest = total;
			best = j;
		}
	}

	return best + 1;
}

int main(int argc, char **argv)
{
	const struct coordinate bounds = {101, 103};
	int ret = 0;
	size_t i;

	if (argc < 2)
	{
		fputs("Please provide a file or files to act upon\n", stderr);

		return AOC_FAILURE;
	}

	for (i = 1; i < argc; i++)
	{
		FILE *input = fopen(argv[i], "rb");
		struct robot *arr;
		size_t len;
		long int total, check;

		if (input == NULL)
		{
			fprintf(stderr, "Failed to open file '%s'\n", argv[i]);
			ret++;

			continue;
		}

		if (parseInput(input, &arr, &len) == AOC_FAILURE)
		{
			fprintf(stderr, "Faled to parse file '%s'\n", argv[i]);
			ret++;

			continue;
		}

		dumpArr(arr, len);
		fprintf(stdout, "Part 1: %ld\n", 
			calculatePart1(arr, len, bounds));
		fprintf(stdout, "Part 2: likely %ld\n", 
			estimatePart2(arr, len, bounds));
		AOC_FREE(arr);
		fclose(input);
	}

	return ret;
}

