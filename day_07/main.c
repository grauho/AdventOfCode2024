#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../aocCommon.h"

#define BUFFER_LEN (1024)

static long int addOperator(const long int left, const long int right);
static long int multiplyOperator(const long int left, const long int right);
static long int combineOperator(const long int left, const long int right);

enum operator
{
	AOC_MULTIPLY = 0,
	AOC_ADD,
	AOC_COMBINE,
	AOC_NUM_OPERATORS
};

struct testCase
{
	long int check;
	long int *val_arr;
	size_t val_len;
};

const struct 
{
	const enum operator op; /* can be used as a check value */
	const long int (*Callback)(const long int, const long int);
} operator_table[] =
{
	{AOC_MULTIPLY, multiplyOperator},
	{AOC_ADD,      addOperator},
	{AOC_COMBINE,  combineOperator}
};
const size_t operator_table_len 
	= sizeof(operator_table) / sizeof(operator_table[0]);

static long int addOperator(const long int left, const long int right)
{
	return left + right;
}

static long int multiplyOperator(const long int left, const long int right)
{
	return left * right;
}

static long int combineOperator(const long int left, const long int right)
{
	size_t exp = 1;

	do
	{
		exp *= 10;
	} while (right >= exp);

	return (left * exp) + right;
}

static void freeCases(struct testCase *case_arr, const size_t case_len)
{
	size_t i;

	if (case_arr != NULL)
	{
		for (i = 0; i < case_len; i++)
		{
			AOC_FREE(case_arr[i].val_arr);
		}

		AOC_FREE(case_arr);
	}
}

static long int extractValue(const char * const str)
{
	long int ret;

	errno = 0;
	ret = strtol(str, NULL, 0);
	AOC_ASSERT(errno == 0);

	return ret;
}

static AOC_STAT parseInput(FILE *input, struct testCase **out, size_t *out_len)
{
	char buffer[BUFFER_LEN] = {0};
	size_t out_max = 10;

	if ((input == NULL) || (out == NULL))
	{
		return AOC_FAILURE;
	}

	AOC_NEW_DYN_ARR(struct testCase, *out, out_max);

	while (fgets(buffer, BUFFER_LEN, input) != NULL)
	{
		struct testCase tmp = {0};
		char *token = NULL;
		size_t tmp_max = 5;
		long int check_val;
		int arg_start;

		if (sscanf(buffer, "%ld: %n\n", &check_val, &arg_start) != 1)
		{
			fprintf(stderr, "Failed to parse line '%s'\n", buffer);

			return AOC_FAILURE;
		}

		tmp.check = check_val;

		if ((token = strtok(buffer + arg_start, " ")) == NULL)
		{
			fprintf(stderr, "No arguments given for line '%s'\n",
				buffer);

			return AOC_FAILURE;
		}

		AOC_NEW_DYN_ARR(long int, tmp.val_arr, tmp_max);

		do
		{
			const long int tok_val = extractValue(token);

			AOC_CAT_DYN_ARR(long int, tmp.val_arr, tmp.val_len,
				tmp_max, tok_val);
		} while ((token = strtok(NULL, " ")) != NULL);

		AOC_CAT_DYN_ARR(struct testCase, *out, *out_len, out_max, tmp);
	}
	
	return AOC_SUCCESS;
}

static AOC_BOOL recursiveTest(const struct testCase test_case, 
	const long int running, const size_t pos, const size_t num_ops)
{
	size_t i;
	AOC_BOOL ret = 0;

	if (pos == test_case.val_len)
	{
		return (running == test_case.check) ? AOC_TRUE : AOC_FALSE;
	}

	for (i = 0; i < num_ops; i++)
	{
		const long int tmp = operator_table[i].Callback(running, 
			test_case.val_arr[pos]);
		ret += recursiveTest(test_case, tmp, pos + 1, num_ops);
	}
	
	return ret;
}

static long int sumValidReports(struct testCase *case_arr, 
	const size_t case_len, const size_t operator_len)
{
	long int total = 0;
	size_t i;

	for (i = 0; i < case_len; i++)
	{
		int tmp = 0;

		tmp += recursiveTest(case_arr[i], case_arr[i].val_arr[0], 1, 
			operator_len);

		if (tmp != 0)
		{
			total += case_arr[i].check;
		}
	}

	return total;
}

int main(int argc, char **argv)
{
	size_t i;

	AOC_ASSERT(operator_table_len == AOC_NUM_OPERATORS);

	if (argc < 2)
	{
		fputs("Please provive a file or files to act upon\n", stderr);

		return AOC_FAILURE;
	}

	for (i = 1; i < argc; i++)
	{
		FILE *input = fopen(argv[i], "rb");
		struct testCase *case_arr = NULL;
		size_t case_len = 0;

		if (input == NULL)
		{
			fprintf(stderr, "Cannot open file '%s'\n", argv[i]);

			continue;
		}

		if (parseInput(input, &case_arr, &case_len) == AOC_FAILURE)
		{
			fprintf(stderr, "Failed to parse file '%s'\n", 
				argv[i]);
			freeCases(case_arr, case_len);
			fclose(input);

			continue;
		}

		fprintf(stdout, "Part 1: %ld\n", sumValidReports(case_arr, 
			case_len, AOC_NUM_OPERATORS - 1));
		fprintf(stdout, "Part 2: %ld\n", sumValidReports(case_arr, 
			case_len, AOC_NUM_OPERATORS));
		freeCases(case_arr, case_len);
		fclose(input);
	}

	return AOC_SUCCESS;
}

