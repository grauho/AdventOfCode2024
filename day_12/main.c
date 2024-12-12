#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aocCommon.h"

#define BUFFER_LEN (1024)

/* This big trick here is finding the area and perimeter of a region in a grid,
 * there is no requirement that the region be a rectangle */

/* One potential way to do this would be to use a recursive 'flood-fill' 
 * algorithm and incriment the area by one for each tile of the given 'plant'
 * visited and incriment perimeter by one for each adjastent tile that is
 * not that particular 'plant' including tiles abutting the edge of the map.
 * One would then need to also mark the visited tile somehow to avoid counting
 * the region more than once */

struct gardenMap
{
	signed char **data;
	size_t cols;
	size_t rows;
};

const struct coordinate
{
	long int x_pos;
	long int y_pos;
} dir_map[] =
{
	{0, -1}, /* NORTH */
	{1,  0}, /* EAST */
	{0,  1}, /* SOUTH */
	{-1, 0}  /* WEST */
};
const size_t dir_map_len = sizeof(dir_map) / sizeof(dir_map[0]);

static int compareCoordinates(const void *foo, const void *bar)
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

static AOC_STAT parseInput(FILE *input, struct gardenMap *map)
{
	char buffer[BUFFER_LEN] = {0};
	size_t len = 0;
	size_t max = 10;
	
	if ((input == NULL) || (map == NULL))
	{
		return AOC_FAILURE;
	}

	AOC_NEW_DYN_ARR(signed char *, map->data, max);

	while (fgets(buffer, BUFFER_LEN, input) != NULL)
	{
		/* No need for the newline character */
		map->cols = strlen(buffer) - 1;

		if (len == max)
		{
			AOC_GROW_DYN_ARR(signed char *, map->data, max);
		}

		/* Accomidate a null terminator to make printing simple */
		if ((map->data[len] = malloc(
			sizeof(signed char) * (map->cols + 1))) == NULL)
		{
			return AOC_FAILURE;
		}

		memcpy(map->data[len], buffer, map->cols);
		map->data[len][map->cols] = '\0';
		len++;
	}

	map->rows = len;

	return AOC_SUCCESS;
}

static void freeMap(struct gardenMap *map)
{
	if ((map != NULL)
	&& (map->data != NULL))
	{
		size_t i;
	
		for (i = 0; i < map->rows; i++)
		{
			AOC_FREE(map->data[i]);
		}

		AOC_FREE(map->data);
	}
}

struct floodInfo
{
	char symbol;
	size_t area;
	size_t peri;
	struct coordinate *visited;
	size_t visit_len;
	size_t visit_max;
};

static struct floodInfo* prepareInfo(struct floodInfo *info, const char symbol)
{
	if (info == NULL)
	{
		return NULL;
	}

	if (info->visited == NULL)
	{
		info->visit_max = 10;
		AOC_NEW_DYN_ARR(struct coordinate, info->visited, 
			info->visit_max);
	}

	info->symbol = symbol;
	info->area = 0;
	info->peri = 0;
	info->visit_len = 0;

	return info;
}

#define POS_AS_CHAR(pos) (AOC_ABS(pos))
#define MARK_POS_FINISHED(pos) ((pos) * (-1))
#define IS_POS_FINISHED(pos) ((pos) < 0)

static void floodFind(struct gardenMap * const map, const size_t y_pos,
	const size_t x_pos, struct floodInfo *info)
{
	size_t i;
	struct coordinate key = {0};
	struct coordinate *found = NULL;

	key.x_pos = (long int) x_pos;
	key.y_pos = (long int) y_pos;

	if ((found = bsearch(&key, info->visited, info->visit_len,
		sizeof(struct coordinate), compareCoordinates)) == NULL)
	{
		AOC_INSERT_CAT(struct coordinate, info->visited, 
			info->visit_len, info->visit_max, key, 
			compareCoordinates);
	}

	if ((y_pos >= map->cols) 
	|| (x_pos >= map->rows)
	|| (POS_AS_CHAR(map->data[y_pos][x_pos]) != info->symbol))
	{
		info->peri++;
	}
	else if (found == NULL)
	{
		info->area++;
		map->data[y_pos][x_pos] 
			= MARK_POS_FINISHED(map->data[y_pos][x_pos]);

		for (i = 0; i < dir_map_len; i++)
		{
			floodFind(map, y_pos + dir_map[i].y_pos, 
				x_pos + dir_map[i].x_pos, info);
		}
	}
}

static void dumpMap(const struct gardenMap * const map);

static long int calculateCostPartI(struct gardenMap * const map)
{
	struct floodInfo info = {0};
	size_t i, j;
	long int total_cost = 0;

	for (j = 0; j < map->rows; j++)
	{
		for (i = 0; i < map->cols; i++)
		{
			const char symbol = map->data[j][i];

			if (IS_POS_FINISHED(map->data[j][i]))
			{
				continue;
			}

			floodFind(map, j, i, prepareInfo(&info, symbol));
			total_cost += (info.area * info.peri);
		}
	}

	AOC_FREE(info.visited);

	return total_cost;
}

/* Now counting the number of sides instead of the number of individual units 
 * of perimeter fencing. Perhaps there's an algorithm for determining the
 * number of sides of an aribitrary polygon but it would also have to account
 * for internal 'islands'. It's possible this could still be accomplished 
 * with the flood algorithm and when an adjacent portion is encountered that
 * does not match the currently symbol one could try to follow that edge 
 * until that direction of adjacency is broken, marking the entire time */
static long int calculateCostPartII(struct gardenMap * const map)
{
	return 0;
}

static void dumpMap(const struct gardenMap * const map)
{
	if (map != NULL)
	{
		size_t i, j;

		for (i = 0; i < map->rows; i++)
		{
			for (j = 0; j < map->cols; j++)
			{
				const char tmp = (map->data[i][j] < 0)
					? '.'
					: POS_AS_CHAR(map->data[i][j]);

				fputc(tmp, stdout);
			}

			fputc('\n', stdout);
		}
	}
}

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
		FILE *input = fopen(argv[i], "rb");
		struct gardenMap map = {0};

		if (input == NULL)
		{
			fprintf(stderr, "Failure to open '%s' for input\n",
				argv[i]);

			continue;
		}

		if (parseInput(input, &map) == AOC_FAILURE)
		{
			fprintf(stderr, "Failed to parse input file '%s'\n",
				argv[i]);
			fclose(input);

			continue;
		}

		fprintf(stdout, "Part 1: %ld\n", calculateCostPartI(&map));
		fprintf(stdout, "Part 2: %ld\n", calculateCostPartII(&map));
		fclose(input);
		freeMap(&map);
	}

	return AOC_SUCCESS;
}

