#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aocCommon.h"

#define BUFFER_LEN (1024)

enum direction
{
	DIR_NORTH = 0,
	DIR_EAST,
	DIR_SOUTH,
	DIR_WEST,
	DIR_NUM
};

struct coordinate
{
	size_t x_pos;
	size_t y_pos;
};

/* Directional unit vectors */
const struct coordinate dir_map[] =
{
	{0, -1}, /* NORTH */
	{1,  0}, /* EAST */
	{0,  1}, /* SOUTH */
	{-1, 0}  /* WEST */
};

struct guardMap
{
	char **data;
	size_t rows;
	size_t cols;
};

struct coordArray
{
	struct coordinate *data;
	size_t len;
};

static AOC_STAT parseInput(FILE *input, struct guardMap *map)
{
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

	return AOC_SUCCESS;
}

/* XXX: This assumes that the guard is always starting facing the northward
 * direction. This seems to be the case in the puzzle example as well as in
 * the input but the question description does not explicitly state that this 
 * is a requirement */
static AOC_STAT findGuard(const struct guardMap map, 
	struct coordinate *guard_pos)
{
	size_t i; 

	for (i = 0; i < map.rows; i++)
	{
		char *found = strstr(map.data[i], "^");

		if (found != NULL)
		{
			(*guard_pos).x_pos = found - map.data[i];
			(*guard_pos).y_pos = i;

			return AOC_SUCCESS;
		}
	}

	return AOC_FAILURE;
}

static long int calculatePart1(struct guardMap map, struct coordinate guard)
{
	long int unique = 0;
	size_t curr_dir = DIR_NORTH; 
	size_t i, j;

	while ((guard.x_pos < map.cols) && (guard.y_pos < map.rows))
	{
		const size_t new_x = guard.x_pos + dir_map[curr_dir].x_pos;
		const size_t new_y = guard.y_pos + dir_map[curr_dir].y_pos;

		if ((new_x < map.cols) && (new_y < map.rows)
		&& (map.data[new_y][new_x] == '#'))
		{
			curr_dir = ((curr_dir + 1) % DIR_NUM);
		}
		else
		{
			map.data[guard.y_pos][guard.x_pos] = '%';
			guard.x_pos = new_x;
			guard.y_pos = new_y;
		}
	}

	for (i = 0; i < map.rows; i++)
	{
		for (j = 0; j < map.cols; j++)
		{
			unique += (map.data[i][j] == '%');
		}
	}

	return unique;
}

struct pivotPoint 
{
	struct coordinate position;
	enum direction heading;
}; 

static int comparePivotPoints(const void *left, const void *right)
{
	const struct pivotPoint *tmp_l = (struct pivotPoint *) left;
	const struct pivotPoint *tmp_r = (struct pivotPoint *) right;

	if (tmp_l->heading != tmp_r->heading)
	{
		return tmp_l->heading - tmp_r->heading;	
	}
	else if (tmp_l->position.y_pos != tmp_r->position.y_pos)
	{
		return tmp_l->position.y_pos - tmp_r->position.y_pos;	
	}
	else
	{
		return tmp_l->position.x_pos - tmp_r->position.x_pos;	
	}
}

/* Proceed until the edge of the map is reached or the hypothetical guard 
 * visits a pivot point they have already been at with the same heading. Saves
 * a little bit of time by having the guard always start at the current 
 * position instead of all the way back at the beginning each time */

/* One bit of easy savings would be to manage visited's allocation outside
 * of this function so that it only has to get initially allocated and 
 * dealloced the once, only len would need to be reset each time */
static AOC_BOOL isPosInfinite(struct guardMap map, 
	struct coordinate guard, enum direction heading)
{
	const size_t obs_x = guard.x_pos + dir_map[heading].x_pos;
	const size_t obs_y = guard.y_pos + dir_map[heading].y_pos;
	struct pivotPoint *visited = NULL;
	size_t len = 0;
	size_t max = 10;
	AOC_BOOL is_infinite = AOC_FALSE;

	map.data[obs_y][obs_x] = '#';
	AOC_NEW_DYN_ARR(struct pivotPoint, visited, max);

	for (;;)
	{
		const size_t new_x = guard.x_pos + dir_map[heading].x_pos;
		const size_t new_y = guard.y_pos + dir_map[heading].y_pos;

		if ((new_x >= map.cols) || (new_y >= map.rows))
		{
			break;
		}

		if (map.data[new_y][new_x] == '#')
		{
			struct pivotPoint tmp = {0};
			struct pivotPoint *found = NULL;

			tmp.position = guard;
			tmp.heading = heading;
			heading = ((heading + 1) % DIR_NUM);

			if ((found = bsearch(&tmp, visited, len, 
				sizeof(struct pivotPoint), comparePivotPoints)) 
					!= NULL)
			{
				is_infinite = AOC_TRUE;

				break;
			}
			else
			{
				AOC_CAT_DYN_ARR(struct pivotPoint, visited, 
					len, max, tmp);
				qsort(visited, len, sizeof(struct pivotPoint), 
					comparePivotPoints);
			}
		}
		else
		{
			guard.x_pos = new_x;
			guard.y_pos = new_y;
		}
	}

	map.data[obs_y][obs_x] = '.';
	AOC_FREE(visited);

	return is_infinite;
}

static long int calculatePart2(struct guardMap map, struct coordinate guard)
{
	struct pivotPoint *tested = NULL;
	enum direction curr_dir = DIR_NORTH;
	long int infinite_positions = 0;
	size_t len = 0;
	size_t max = 10;

	/* Heading is a dummy value for these */
	AOC_NEW_DYN_ARR(struct pivotPoint, tested, max);
	tested[0].position = guard;
	tested[0].heading = 0;

	for (;;)
	{
		const size_t new_x = guard.x_pos + dir_map[curr_dir].x_pos;
		const size_t new_y = guard.y_pos + dir_map[curr_dir].y_pos;

		if ((new_x >= map.cols) || (new_y >= map.rows))
		{
			break;
		}

		if (map.data[new_y][new_x] == '#')
		{
			curr_dir = ((curr_dir + 1) % DIR_NUM);
		}
		else
		{
			struct pivotPoint key;
			struct pivotPoint *found = NULL;

			key.position.x_pos = new_x;
			key.position.y_pos = new_y;
			key.heading = 0;

			if ((found = bsearch(&key, tested, len, 
				sizeof(struct pivotPoint), comparePivotPoints)) 
					== NULL)
			{
				infinite_positions 
					+= isPosInfinite(map, guard, curr_dir);
				AOC_CAT_DYN_ARR(struct pivotPoint, tested, 
					len, max, key);
				qsort(tested, len, sizeof(struct pivotPoint),
					comparePivotPoints);
			}

			guard = key.position;
		}
	}

	AOC_FREE(tested);

	return infinite_positions;
}

static void freeMap(struct guardMap *map)
{
	if (map != NULL)
	{
		size_t i;

		for (i = 0; i < map->rows; i++)
		{
			AOC_FREE(map->data[i]);
		}

		AOC_FREE(map->data);
	}
}

int main(int argc, char **argv)
{
	size_t i;

	if (argc < 2)
	{
		fprintf(stderr, 
			"Please provide a file or files to act upon\n");

		return AOC_FAILURE;
	}

	for (i = 1; i < argc; i++)
	{
		FILE *input = NULL;
		struct guardMap map = {0};
		struct coordinate guard = {0};

		if ((input = fopen(argv[i], "rb")) == NULL)
		{
			return AOC_FAILURE;
		}

		if ((parseInput(input, &map) != AOC_SUCCESS)
		|| (findGuard(map, &guard) != AOC_SUCCESS))
		{
			return AOC_FAILURE;
		}

		fprintf(stdout, "Part 1: %lu\n", calculatePart1(map, guard));
		fprintf(stdout, "Part 2: %lu\n", calculatePart2(map, guard));

		freeMap(&map);
		fclose(input);
	}

	return AOC_SUCCESS;
}

