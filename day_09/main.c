#include <stdio.h>
#include <stdlib.h>

#include "../aocCommon.h"

/* TODO: This is currently only a partial solution as the deque approach only
 * really works for part 1 */

struct deque
{
	int *data;
	size_t f_index; /* initial value 0 */
	size_t b_index; /* initial value len */
	size_t len;
};

/* post-incriment */
static AOC_STAT dealTop(struct deque *queue, int *out)
{
	*out = queue->data[queue->f_index++];

	return (queue->f_index <= queue->b_index) ? AOC_SUCCESS : AOC_FAILURE;
}

/* pre-decriment, avoiding underflow */
static AOC_STAT dealBottom(struct deque *queue, int *out)
{
	if (queue->b_index == 0)
	{
		return AOC_FAILURE;
	}

	*out = queue->data[--queue->b_index];

	return (queue->b_index >= queue->f_index) ? AOC_SUCCESS : AOC_FAILURE;
}

static char* slurpInput(FILE *input, size_t *out_len)
{
	char *out_arr = NULL;
	size_t len = 0;
	size_t max = 10;
	int ch;

	if ((input == NULL) || (out_len == NULL))
	{
		return NULL;
	}

	AOC_NEW_DYN_ARR(char, out_arr, max);

	while (((ch = fgetc(input)) != EOF) && (ch != '\n'))
	{
		AOC_CAT_DYN_ARR(char, out_arr, len, max, ch);
	}

	*out_len = len;

	return out_arr;
}

static AOC_STAT parseBufferedInput(const char * const input, const size_t len, 
	struct deque **out)
{
	struct deque *working = malloc(sizeof(struct deque));
	size_t i, j, max, file_id = 0;
	AOC_BOOL active = AOC_TRUE;

	if ((input == NULL) || (out == NULL) || (working == NULL))
	{
		return AOC_FAILURE;
	}

	max = 10;
	working->f_index = 0;
	working->b_index = 0;

	AOC_NEW_DYN_ARR(int, working->data, max);

	for (i = 0; i < len; i++)
	{
		if (active == AOC_TRUE)
		{
			const size_t count = input[i] - '0';

			for (j = 0; j < count; j++)
			{
				AOC_CAT_DYN_ARR(int, working->data, 
					working->b_index, max, file_id);
			}

			file_id++;
			active = AOC_FALSE;
		}
		else
		{
			active = AOC_TRUE;
		}
	}

	working->len = working->b_index;
	*out = working;

	return AOC_SUCCESS;
}

/* Only works for part 1 */
static size_t calculateChecksum(const char * const buffer, const size_t len, 
	struct deque *queue)
{
	size_t pos = 0;
	size_t checksum = 0;
	AOC_BOOL active = AOC_TRUE;
	size_t i;

	if ((buffer == NULL) || (queue == NULL))
	{
		return AOC_FAILURE;
	}

	for (i = 0; i < len; i++)
	{
		const size_t count = buffer[i] - '0';
		size_t j;

		for (j = 0; j < count; j++)
		{
			int file_id;

			if (active == AOC_TRUE)
			{
				if (dealTop(queue, &file_id) == AOC_FAILURE)
				{
					break;
				}
			}
			else
			{
				if (dealBottom(queue, &file_id) == AOC_FAILURE)
				{
					break;
				}
			}

			checksum += (pos * file_id);
			pos++;
		}

		active = (active == AOC_TRUE) ? AOC_FALSE : AOC_TRUE;

		/* All values have been processed */
		if (queue->f_index >= queue->b_index)
		{
			break;
		}
	}

	return checksum;
}

static void freeDeque(struct deque *entries)
{
	if (entries != NULL)
	{
		AOC_FREE(entries->data);
		AOC_FREE(entries);
	}
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
		char *buffer = NULL;
		struct deque *entries = NULL;
		size_t buffer_len;

		if ((input = fopen(argv[i], "rb")) == NULL)
		{
			fprintf(stderr, "Failed to open file '%s'\n", argv[i]);

			continue;
		}

		if (((buffer = slurpInput(input, &buffer_len)) == NULL)
		|| (parseBufferedInput(buffer, buffer_len, &entries) 
			== AOC_FAILURE))
		{
			fprintf(stderr, "Failed to parse file '%s'\n", 
				argv[i]);
			freeDeque(entries);
			fclose(input);

			continue;
		}

		fprintf(stdout, "Part 1: %lu\n",
			calculateChecksum(buffer, buffer_len, entries));
		freeDeque(entries);
		fclose(input);
		AOC_FREE(buffer);
	}

	return AOC_SUCCESS;
}

