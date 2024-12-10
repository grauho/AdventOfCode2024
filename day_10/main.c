#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aocCommon.h"

#define BUFFER_LEN (1024)

struct trailMap
{
	char **data;
	size_t rows;
	size_t cols;
};

struct coordinate
{
	size_t x_pos;
	size_t y_pos;
}; 

struct visisted
{
	struct coordinate *coord;
	size_t len;
	size_t max;
};

const struct dirMap
{
	const size_t delta_x;
	const size_t delta_y;
} dir_map[] = 
{
	{0, -1}, /* NORTH */
	{1,  0}, /* EAST */
	{0,  1}, /* SOUTH */
	{-1, 0}  /* WEST */
};
const size_t dir_len = sizeof(dir_map) / sizeof(dir_map[0]);

static int compareCoords(const void *foo, const void *bar)
{
	const struct coordinate * const left = (struct coordinate *) foo;
	const struct coordinate * const right = (struct coordinate *) bar;

	if (left->y_pos != right->y_pos)
	{
		return left->y_pos - right->y_pos;
	}
	else
	{
		return left->x_pos - right->x_pos;
	}
}

static AOC_STAT slurpInput(FILE *input, struct trailMap **out)
{
	struct trailMap *map = malloc(sizeof(struct trailMap));
	char buffer[BUFFER_LEN] = {0};
	size_t len = 0;
	size_t max = 10;
	
	if ((input == NULL) || (map == NULL))
	{
		return AOC_FAILURE;
	}

	AOC_NEW_DYN_ARR(char *, map->data, max);

	while (fgets(buffer, BUFFER_LEN, input) != NULL)
	{
		/* No need for the newline character */
		map->cols = strlen(buffer) - 1;

		if (len == max)
		{
			AOC_GROW_DYN_ARR(char *, map->data, max);
		}

		/* Accomidate a null terminator to make printing simple */
		if ((map->data[len] = malloc(sizeof(char) * (map->cols + 1))) 
			== NULL)
		{
			return AOC_FAILURE;
		}

		memcpy(map->data[len], buffer, map->cols);
		map->data[len][map->cols] = '\0';
		len++;
	}

	map->rows = len;
	*out = map;

	return AOC_SUCCESS;
}

static void freeMap(struct trailMap *map)
{
	if (map != NULL)
	{
		size_t i;
		
		for (i = 0; i < map->rows; i++)
		{
			AOC_FREE(map->data[i]);
		}

		AOC_FREE(map->data);
		AOC_FREE(map);
	}
}

static long int testTrails(const struct trailMap * const map, 
	const size_t x_pos, const size_t y_pos, struct visisted *nines)
{
	size_t i, ret = 0;

	if (map->data[y_pos][x_pos] == '9')
	{
		struct coordinate key = {0};
		struct coordinate *found = NULL;

		key.x_pos = x_pos;
		key.y_pos = y_pos;

		if ((found = bsearch(&key, nines->coord, nines->len, 
			sizeof(struct coordinate), compareCoords)) != NULL)
		{
			/* Already visisted */
			return 0;
		}

		AOC_INSERT_CAT(struct coordinate, nines->coord, nines->len,
			nines->max, key, compareCoords);

		return 1;
	}

	for (i = 0; i < dir_len; i++)
	{
		const size_t new_x = x_pos + dir_map[i].delta_x;
		const size_t new_y = y_pos + dir_map[i].delta_y;

		/* Relying on unsigned underflow */
		if (((new_x < map->cols) && (new_y < map->rows))
		&& (map->data[new_y][new_x] - map->data[y_pos][x_pos] == 1))
		{
			ret += testTrails(map, new_x, new_y, nines);
		}
	}

	return ret;
}

/* Counting just how many unique nines a given zero position can reach, doesn't
 * care if there are two ways to get to the same nine */
static long int getTrailScores(struct trailMap *map)
{
	struct visisted nines = {0};
	long int score = 0;
	size_t i, j;

	if (map == NULL)
	{
		return -1;
	}

	AOC_NEW_DYN_ARR(struct coordinate, nines.coord, nines.max);

	for (j = 0; j < map->rows; j++)
	{
		for (i = 0; i < map->cols; i++)
		{
			if (map->data[j][i] == '0')
			{
				score += testTrails(map, i, j, &nines);

				/* Reset the visisted positions */
				nines.len = 0;
			}
		}
	}

	AOC_FREE(nines.coord);

	return score;
}

static long int testRatings(const struct trailMap * const map, 
	const size_t x_pos, const size_t y_pos)
{
	size_t i, ret = 0;

	if (map->data[y_pos][x_pos] == '9')
	{
		return 1;
	}

	for (i = 0; i < dir_len; i++)
	{
		const size_t new_x = x_pos + dir_map[i].delta_x;
		const size_t new_y = y_pos + dir_map[i].delta_y;

		/* Relying on unsigned underflow */
		if (((new_x < map->cols) && (new_y < map->rows))
		&& (map->data[new_y][new_x] - map->data[y_pos][x_pos] == 1))
		{
			ret += testRatings(map, new_x, new_y);
		}
	}

	return ret;
}

/* Like part 1 but now multiple ways to get to the same nine can be counted 
 * in the 'rating' of a trail */
static long int getTrailRatings(struct trailMap *map)
{
	long int score = 0;
	size_t i, j;

	if (map == NULL)
	{
		return -1;
	}

	for (j = 0; j < map->rows; j++)
	{
		for (i = 0; i < map->cols; i++)
		{
			if (map->data[j][i] == '0')
			{
				score += testRatings(map, i, j);
			}
		}
	}

	return score;
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
		FILE *input;
		struct trailMap *map = NULL;

		if ((input = fopen(argv[i], "rb")) == NULL)
		{
			fprintf(stderr, "Failed to open '%s' for input\n",
				argv[i]);

			continue;
		}

		if (slurpInput(input, &map) != AOC_SUCCESS)
		{
			fprintf(stderr, "Failed to parse '%s' to map\n",
				argv[i]);
			fclose(input);
			freeMap(map);

			continue;
		}

		fprintf(stdout, "Part 1: %ld\n", getTrailScores(map));
		fprintf(stdout, "Part 2: %ld\n", getTrailRatings(map));
		fclose(input);
		freeMap(map);
	}

	return AOC_SUCCESS;
}

