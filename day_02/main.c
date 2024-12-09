#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../aocCommon.h"

#define BUFFER_LEN (128)

static long int extractValue(const char * const str)
{
	long int ret;

	errno = 0;
	ret = strtol(str, NULL, 0);
	AOC_ASSERT(errno == 0);

	return ret;
}

static AOC_BOOL isSafe(const int dir, const long int last, const long int curr)
{
	const long int diff = AOC_ABS(curr - last);

	if (((dir < 0) && (curr > last))
	|| ((dir > 0) && (curr < last))
	|| (diff < 1)
	|| (diff > 3))
	{
		return AOC_FALSE;
	}
	else
	{
		return AOC_TRUE;
	}
}

/* In a real application it would be better to write a threadsafe tokenizer */
static int parseLevels(FILE *input, const AOC_BOOL use_dampener)
{
	char line_buffer[BUFFER_LEN] = {0};
	int num_safe = 0;

	while (fgets(line_buffer, BUFFER_LEN, input) != NULL)
	{
		long int last, curr;
		int direction = 0;
		char *token = NULL;
		AOC_BOOL early_exit = AOC_FALSE;
		AOC_BOOL dampener = use_dampener;

		/* First prime last, curr, and direction using the first 
		 * two values of the report */
		last = extractValue(strtok(line_buffer, " "));

		/* List contains only one value which isn't well defined by
		 * the problem but here is assumed to be safe */
		if ((token = strtok(NULL, " ")) == NULL)
		{
			num_safe++;

			continue;
		}

		curr = extractValue(token);
		direction = curr - last;

		/* XXX: If curr value is invalidated by the dampener
		 * there is the possibilty of the direction needing to change.
		 * But the problem doesn't specify what the behavior for this
		 * situation should be and it does not show up in the input */
		if (isSafe(direction, last, curr) == AOC_FALSE)
		{
			if (dampener == AOC_FALSE)
			{
				continue;
			}

			dampener = AOC_FALSE;
			curr = last;
		}

		while ((early_exit == AOC_FALSE) 
		&& ((token = strtok(NULL, " ")) != NULL))
		{
			last = curr;
			curr = extractValue(token);

			if (isSafe(direction, last, curr) == AOC_FALSE)
			{
				if (dampener == AOC_TRUE)
				{
					dampener = AOC_FALSE;
					curr = last;
				}
				else
				{
					early_exit = AOC_TRUE;
				}
			}
		}

		if (early_exit == AOC_FALSE)
		{
			num_safe++;
		}
	}

	return num_safe;
}

int main(int argc, char **argv)
{
	size_t i;

	if (argc < 2)
	{
		fprintf(stderr, "Please provide an input file\n");

		return AOC_FAILURE;
	}

	for (i = 1; i < argc; i++)
	{
		FILE *f_handle = NULL;

		if ((f_handle = fopen(argv[i], "rb")) == NULL)
		{
			fprintf(stderr, "Failed to open file '%s' for reading\n",
				argv[i]);

			return AOC_FAILURE;
		}

		fprintf(stdout, "undampened %d levels are safe\n", 
			parseLevels(f_handle, AOC_FALSE));
		rewind(f_handle);
		fprintf(stdout, "dampened %d levels are safe\n", 
			parseLevels(f_handle, AOC_TRUE));
		fclose(f_handle);
	}

	return AOC_SUCCESS;
}

