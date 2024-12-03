#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aocCommon.h"

const char mul_str[]  = "mul(";
const char do_str[]   = "do()";
const char dont_str[] = "don't()";
const size_t mul_len  = sizeof(mul_str) - 1;
const size_t do_len   = sizeof(do_str) - 1;
const size_t dont_len = sizeof(dont_str) - 1;

/* Slurps the entire file into a null terminated buffer for later use
 * with strstr and sscanf */
static char* slurpFile(FILE *f_handle)
{
	const long int f_len = getFileLength(f_handle);
	char *ret = NULL;

	AOC_ASSERT(f_len != -1);

	if (((ret = malloc(sizeof(char) * (f_len + 1))) == NULL)
	|| (fread(ret, sizeof(char), f_len, f_handle) != f_len))
	{
		AOC_FREE(ret);

		return NULL;
	}

	ret[f_len] = '\0';

	return ret;
}

static long int parseMul(const char *str)
{
	long int arg_1;
	long int arg_2;
	char check;

	/* Without check sscanf will still report 2 matches even if the
	 * closing parenthesis does not match because that count is 
	 * incrimented when the match is made even if later parts of the 
	 * format string do not match */
	if ((sscanf(str, "mul(%ld,%ld%c", &arg_1, &arg_2, &check) == 3)
	&& (check == ')'))
	{
		return (arg_1 * arg_2);	
	}

	return 0;
}

static unsigned long int parseInputPart1(const char *str)
{
	unsigned long int running_total = 0;
	const char *pos = str;

	while ((pos = strstr(pos, mul_str)) != NULL)
	{
		running_total += parseMul(pos);
		pos += mul_len;
	}

	return running_total;
}

enum needleID
{
	ID_DO = 0,
	ID_DONT,
	ID_MUL,
	ID_LEN,
	ID_ERROR = ID_LEN
};

static const char* getNextNeedle(const char *str, enum needleID *id)
{
	const struct
	{
		const char *needle;
		size_t len;
		enum needleID id;
	} needle_arr[] =
	{
		{mul_str,  mul_len,  ID_MUL},
		{do_str,   do_len,   ID_DO},
		{dont_str, dont_len, ID_DONT}
	};
	const size_t needle_len = sizeof(needle_arr) / sizeof(needle_arr[0]);
	long int best_pos = -1;
	size_t i;

	/* All cases must be handled, would be even better as a static assert
	 * but C doesn't get those until C11 I believe */
	AOC_ASSERT(needle_len == ID_LEN);

	for (i = 0; i < needle_len; i++)
	{
		char *tmp = strstr(str, needle_arr[i].needle);

		if (tmp == NULL)
		{
			continue;
		}

		if ((best_pos == -1)
		|| (tmp - str < best_pos))
		{
			best_pos = tmp - str;
			*id = needle_arr[i].id;
		}
	}

	return (best_pos == -1) ? NULL : str + best_pos;
}

static unsigned long int parseInputPart2(const char *str)
{	
	unsigned long int running_total = 0;
	const char *pos = str;
	enum needleID id = ID_ERROR;
	AOC_BOOL is_active = AOC_TRUE;

	while ((pos = getNextNeedle(pos, &id)) != NULL)
	{
		switch (id)
		{
			case ID_MUL:
				if (is_active == AOC_TRUE)
				{
					running_total += parseMul(pos);
				}

				pos += mul_len;

				break;
			case ID_DO:
				is_active = AOC_TRUE;
				pos += do_len;

				break;
			case ID_DONT:
				is_active = AOC_FALSE;
				pos += dont_len;

				break;
			default:
				AOC_ASSERT(0 || "UNREACHABLE");

				break;
		}
	}

	return running_total;
}

int main(int argc, char **argv)
{
	FILE *f_handle = NULL;
	char *slurped = NULL;

	if (argc < 2)
	{
		fputs("Please provide a file to act upon\n", stderr);

		return AOC_FAILURE;
	}

	if ((f_handle = fopen(argv[1], "rb")) == NULL)
	{
		fprintf(stderr, "Unable to open file '%s' for reading\n",
			argv[1]);
	}

	if ((slurped = slurpFile(f_handle)) != NULL)
	{
		fprintf(stdout, "Part 1: %lu\n", parseInputPart1(slurped));
		fprintf(stdout, "Part 2: %lu\n", parseInputPart2(slurped));
	}

	fclose(f_handle);
	AOC_FREE(slurped);

	return AOC_SUCCESS;
}

