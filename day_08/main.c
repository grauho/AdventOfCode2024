#include <stdio.h>
#include <stdlib.h>

#include "../aocCommon.h"

#define BUFFER_LEN (1024)

struct nodeMap
{
	char **data;
	size_t rows;
	size_t cols;
};

struct coordinate
{
	long int x_pos;
	long int y_pos;
};

struct nodeIndex
{
	char id;
	struct coordinate *coords;
	size_t coords_len;
	size_t coords_max;
};

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

static int compareNodeTypes(const void *foo, const void *bar)
{
	const struct nodeIndex * const left = (struct nodeIndex *) foo;
	const struct nodeIndex * const right = (struct nodeIndex *) bar; 

	return left->id - right->id;
}

static size_t populateNodeIndex(struct nodeIndex **nodes, 
	const struct nodeMap * const map)
{
	size_t len = 0;
	size_t max = 10;
	size_t i, j;

	AOC_NEW_DYN_ARR(struct nodeIndex, *nodes, max);

	for (j = 0; j < map->rows; j++)
	{
		for (i = 0; i < map->cols; i++)
		{
			const char marker = map->data[j][i];
			struct coordinate tmp_coord = {0};
			struct nodeIndex tmp_type = {0};
			struct nodeIndex *found = NULL;

			tmp_coord.x_pos = i;
			tmp_coord.y_pos = j;

			if (map->data[j][i] == '.')
			{
				continue;
			}

			tmp_type.id = marker;

			/* ID not already represented */
			if ((found = bsearch(&tmp_type, *nodes, len, 
				sizeof(struct nodeIndex), compareNodeTypes)) 
					== NULL)
			{
				/* Do operations on the new node type prior to
				 * concatination! */
				tmp_type.coords_len = 0;
				tmp_type.coords_max = 10;
				AOC_NEW_DYN_ARR(struct coordinate, 
					tmp_type.coords, tmp_type.coords_max);
				AOC_INSERT_CAT(struct coordinate, 
					tmp_type.coords, tmp_type.coords_len, 
					tmp_type.coords_max, tmp_coord, 
					compareCoordinates);
				AOC_INSERT_CAT(struct nodeIndex, *nodes, len, 
					max, tmp_type, compareNodeTypes);
			}
			else
			{
				/* Concatinate just the new coordinate to the
				 * existing header structure */
				AOC_INSERT_CAT(struct coordinate, 
					found->coords, found->coords_len, 
					found->coords_max, tmp_coord, 
					compareCoordinates);
			}
		}
	}

	return len;
}

static void freeNodeIndex(struct nodeIndex *data, const size_t len)
{
	if (data != NULL)
	{
		size_t i;

		for (i = 0; i < len; i++)
		{
			AOC_FREE(data[i].coords);
		}

		AOC_FREE(data);
	}
}

static AOC_STAT parseInput(FILE *input, struct nodeMap *map)
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

static void freeMap(struct nodeMap *map)
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

static long int testAntinodePairs(const struct coordinate left, 
	const struct coordinate right, struct nodeMap * const map)
{
	int ret = 0;
	struct coordinate diff = {0};

	diff.x_pos = right.x_pos - left.x_pos;
	diff.y_pos = right.y_pos - left.y_pos;

	if ((left.x_pos - diff.x_pos >= 0)
	&& (left.y_pos - diff.y_pos >= 0)
	&& (left.x_pos - diff.x_pos < map->cols)
	&& (left.y_pos - diff.y_pos < map->rows))
	{
		const size_t anti_x = left.x_pos - diff.x_pos;
		const size_t anti_y = left.y_pos - diff.y_pos;

		if (map->data[anti_y][anti_x] != '#')
		{
			map->data[anti_y][anti_x] = '#';
			ret++;
		}
	}

	if ((diff.x_pos + right.x_pos >= 0)
	&& (diff.y_pos + right.y_pos >= 0)
	&& (diff.x_pos + right.x_pos < map->cols)
	&& (diff.y_pos + right.y_pos < map->rows))
	{
		const size_t anti_x = right.x_pos + diff.x_pos;
		const size_t anti_y = right.y_pos + diff.y_pos;

		if (map->data[anti_y][anti_x] != '#')
		{
			map->data[anti_y][anti_x] = '#';
			ret++;
		}
	}

	return ret;
}

#define COORD_IN_BOUNDS(coord, map)                   \
	(((coord).x_pos >= 0) && ((coord).y_pos >= 0) \
		&& ((coord).x_pos < (map)->cols)      \
		&& ((coord).y_pos < (map)->rows))

static long int testAntinodeRepeating(const struct coordinate left, 
	const struct coordinate right, struct nodeMap *map)
{
	int ret = 0;
	struct coordinate diff = {0};
	struct coordinate anti = {0};

	diff.x_pos = right.x_pos - left.x_pos;
	diff.y_pos = right.y_pos - left.y_pos;
	anti = left;
	
	while (COORD_IN_BOUNDS(anti, map))
	{
		if (map->data[anti.y_pos][anti.x_pos] != '?')
		{
			map->data[anti.y_pos][anti.x_pos] = '?';
			ret++;
		}

		anti.x_pos -= diff.x_pos;
		anti.y_pos -= diff.y_pos;
	}

	anti = right;

	while (COORD_IN_BOUNDS(anti, map))
	{
		if (map->data[anti.y_pos][anti.x_pos] != '?')
		{
			map->data[anti.y_pos][anti.x_pos] = '?';
			ret++;
		}

		anti.x_pos += diff.x_pos;
		anti.y_pos += diff.y_pos;
	}

	return ret;
}

static size_t calculateAntinodes(const struct nodeIndex * const nodes,
	const size_t num_nodes, struct nodeMap *map, 
	long int (*Callback)(const struct coordinate, const struct coordinate,
		struct nodeMap * const))
{
	size_t i, j, k;
	size_t antinodes = 0;

	for (i = 0; i < num_nodes; i++)
	{
		for (j = 0; j < nodes[i].coords_len; j++)
		{
			const struct coordinate left = nodes[i].coords[j];

			for (k = j + 1; k < nodes[i].coords_len; k++)
			{
				const struct coordinate right 
					= nodes[i].coords[k];

				antinodes += Callback(left, right, map);

			}
		}
	}

	return antinodes;
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
		struct nodeMap map = {0};
		struct nodeIndex *nodes = NULL;
		size_t num_nodes = 0;

		if (input == NULL)
		{
			fprintf(stderr, "Failed to open file '%s'\n", argv[i]);

			continue;
		}

		if (parseInput(input, &map) != AOC_SUCCESS)
		{
			fprintf(stderr, "Failed to parse file '%s' to map\n", 
				argv[i]);
			freeMap(&map);
			fclose(input);

			continue;
		}

		num_nodes = populateNodeIndex(&nodes, &map);
		fprintf(stdout, "Part 1: %lu anti-nodes\n", 
			calculateAntinodes(nodes, num_nodes, &map, 
			testAntinodePairs));
		fprintf(stdout, "Part 2: %lu anti-nodes\n", 
			calculateAntinodes(nodes, num_nodes, &map, 
			testAntinodeRepeating));

		freeNodeIndex(nodes, num_nodes);
		freeMap(&map);
		fclose(input);
	}

	return AOC_SUCCESS;
}

