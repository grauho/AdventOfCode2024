#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aocCommon.h"

#define LINE_BUFFER_LEN (1024)

struct searchGrid
{
	char **data;
	size_t rows;
	size_t cols;
};

enum gridDirection
{
	GRID_HOLD  = (0),
	GRID_WEST  = (-1),
	GRID_EAST  = (1),
	GRID_NORTH = (-1),
	GRID_SOUTH = (1),
	GRID_NUM_DIRS
};

static AOC_BOOL checkHeading(const struct searchGrid * const grid, 
	const size_t x_pos, const size_t y_pos, const enum gridDirection x_dir, 
	const enum gridDirection y_dir)
{
	const char target[] = "XMAS";
	size_t i, j, curs = 0;

	/* This is relying on underflow of the unsigned size_t values to then
	 * be out of bounds of the grid and cause the for loop to break */
	for (i = x_pos, j = y_pos; (i < grid->cols) && (j < grid->rows); 
		i += x_dir, j += y_dir)
	{
		if (grid->data[j][i] != target[curs++])
		{
			break;
		}

		if (target[curs] == '\0')
		{
			return AOC_TRUE;
		}
	}

	return AOC_FALSE;
}

static unsigned long int searchPart1(const struct searchGrid * const grid)
{
	const struct 
	{
		const enum gridDirection x_dir;
		const enum gridDirection y_dir;
	} heading_map[] =
	{
		{GRID_HOLD, GRID_NORTH}, /* Due North */
		{GRID_EAST, GRID_NORTH}, /* North East */
		{GRID_EAST, GRID_HOLD},  /* Due East */
		{GRID_EAST, GRID_SOUTH}, /* South East */
		{GRID_HOLD, GRID_SOUTH}, /* Due South */
		{GRID_WEST, GRID_SOUTH}, /* South West */
		{GRID_WEST, GRID_HOLD},  /* Due West */
		{GRID_WEST, GRID_NORTH}  /* North West */
	};
	const size_t num_headings 
		= sizeof(heading_map) / sizeof(heading_map[0]);
	unsigned long int num_found = 0;
	size_t i, j, k;

	for (j = 0; j < grid->rows; j++)
	{
		for (i = 0; i < grid->cols; i++)
		{
			if (grid->data[j][i] != 'X')
			{
				continue;
			}

			for (k = 0; k < num_headings; k++)
			{
				num_found += checkHeading(grid, i, j, 
					heading_map[k].x_dir,
					heading_map[k].y_dir);
			}
		}
	}

	return num_found;
}

/* This is a little sloppy but workable */
static AOC_BOOL checkDiagonal(const struct searchGrid * const grid,
	const size_t x_pos, const size_t y_pos, const AOC_BOOL is_left)
{
	int m_count = 0;
	int s_count = 0;

	/* Does the diagonal line go from the upper-lefthand corner to the 
	 * lower right or not */
	if (is_left == AOC_TRUE)
	{
		m_count += grid->data[y_pos - 1][x_pos - 1] == 'M';
		m_count += grid->data[y_pos + 1][x_pos + 1] == 'M';
		s_count += grid->data[y_pos - 1][x_pos - 1] == 'S';
		s_count += grid->data[y_pos + 1][x_pos + 1] == 'S';
	}
	else
	{
		m_count += grid->data[y_pos - 1][x_pos + 1] == 'M';
		m_count += grid->data[y_pos + 1][x_pos - 1] == 'M';
		s_count += grid->data[y_pos + 1][x_pos - 1] == 'S';
		s_count += grid->data[y_pos - 1][x_pos + 1] == 'S';
	}

	return ((m_count == 1) && (s_count == 1));
}

static unsigned long int searchPart2(const struct searchGrid * const grid)
{
	unsigned long int num_found = 0;
	size_t i, j;

	/* Only looking for centers in these loops and these more limited 
	 * bounds ensure that there won't be out of bound accesses when 
	 * checking the diagonals */
	for (j = 1; j < grid->rows - 1; j++)
	{
		for (i = 1; i < grid->cols - 1; i++)
		{
			if (grid->data[j][i] != 'A')
			{
				continue;
			}

			if ((checkDiagonal(grid, i, j, AOC_TRUE) == AOC_TRUE)
			&& (checkDiagonal(grid, i, j, AOC_FALSE) == AOC_TRUE))
			{
				num_found++;
			}
		}
	}

	return num_found;
}

static void freeGrid(struct searchGrid *grid)
{
	size_t i;

	if (grid != NULL)
	{
		for (i = 0; i < grid->rows; i++)
		{
			AOC_FREE(grid->data[i]);
		}

		AOC_FREE(grid->data);
		AOC_FREE(grid);
	}
}

/* It might be easier to have fgets read directly into a prepared buffer in
 * the grid, as all the rows are the same length, but this is just cleaner at
 * the moment and doesn't require trimming a trailing prepared but unused 
 * buffer */
static struct searchGrid* slurpFileToGrid(FILE *input)
{
	char buffer[LINE_BUFFER_LEN] = {0};
	struct searchGrid *grid = calloc(1, sizeof(struct searchGrid));
	size_t max = 10;

	if ((input == NULL) || (grid == NULL))
	{
		return NULL;
	}

	AOC_NEW_DYN_ARR(char *, grid->data, max);

	while (fgets(buffer, LINE_BUFFER_LEN, input) != NULL)
	{
		/* There is no reason to include the newline character */
		const size_t line_len = strlen(buffer) - 1;		

		/* Realistically this should be checked for each row input as
		 * they all need to be the same length */
		grid->cols = line_len;

		if (grid->rows == max)
		{
			AOC_GROW_DYN_ARR(char *, grid->data, max);
		}

		grid->data[grid->rows] = malloc(sizeof(char) * (line_len + 1));

		if (grid->data[grid->rows] == NULL)
		{
			freeGrid(grid);

			return NULL;
		}

		memcpy(grid->data[grid->rows], buffer, line_len);
		grid->data[grid->rows][line_len] = '\0';
		grid->rows++;
	}

	return grid;
}

int main(int argc, char **argv)
{
	FILE *input;
	struct searchGrid *grid = NULL;

	if (argc < 2)
	{
		fputs("Please provide a file to act upon\n", stderr);
		
		return AOC_FAILURE;
	}

	if ((input = fopen(argv[1], "rb")) == NULL)
	{
		fprintf(stderr, "Unable to open file '%s' for reading\n",
			argv[1]);

		return AOC_FAILURE;
	}

	if ((grid = slurpFileToGrid(input)) == NULL)
	{
		fputs("Failed to parse input into search grid\n", stderr);
		fclose(input);

		return AOC_FAILURE;
	}

	fprintf(stdout, "Part 1: %lu matches\n", searchPart1(grid));
	fprintf(stdout, "Part 2: %lu matches\n", searchPart2(grid));
	freeGrid(grid);
	fclose(input);

	return AOC_SUCCESS;
}

