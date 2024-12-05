#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../aocCommon.h"

#define BUFFER_LEN (256)

struct pageReport
{
	int *vals;
	size_t len;
};

struct pageRule
{
	int left;
	int right;
};

static long int extractValue(const char * const str)
{
	long int ret;

	errno = 0;
	ret = strtol(str, NULL, 0);
	AOC_ASSERT(errno == 0);

	return ret;
}

static int compareRules(const void *left, const void *right)
{
	const struct pageRule * const foo = (struct pageRule *) left;
	const struct pageRule * const bar = (struct pageRule *) right;

	return (foo->left == bar->left) 
		? foo->right - bar->right
		: foo->left - bar->left;
}

AOC_STAT parseFile(FILE *input, struct pageRule **rules, size_t *num_rules,
	struct pageReport **reports, size_t *num_reports)
{
	char buffer[BUFFER_LEN] = {0};
	size_t len = 0;
	size_t max = 10;

	if ((input == NULL) || (rules == NULL) || (reports == NULL))
	{
		return AOC_FAILURE;
	}

	AOC_NEW_DYN_ARR(struct pageRule, (*rules), max);

	/* Parse the page rules, go until either the end of the file is
	 * encountered or until the blank line separating the rules and 
	 * report sections */
	while ((fgets(buffer, BUFFER_LEN, input) != NULL)
	&& (buffer[0] != '\n'))
	{
		struct pageRule tmp = {0};

		if (sscanf(buffer, "%d|%d\n", &tmp.left, &tmp.right) != 2)
		{
			fprintf(stderr, "Malformed page rules\n");

			return AOC_FAILURE;
		}

		AOC_CAT_DYN_ARR(struct pageRule, (*rules), len, max, tmp);
	}

	qsort((*rules), len, sizeof(struct pageRule), compareRules);
	*num_rules = len;

	max = 10;
	len = 0;
	AOC_NEW_DYN_ARR(struct pageReport, (*reports), max);

	/* Parse the page reports until the end of the file */
	while (fgets(buffer, BUFFER_LEN, input) != NULL)
	{
		int *val_arr = NULL;
		char *token = NULL;
		size_t val_len = 0;
		size_t val_max = 10;

		token = strtok(buffer, ",");

		if (len == max)
		{
			AOC_GROW_DYN_ARR(struct pageReport, (*reports), max);
		}

		AOC_NEW_DYN_ARR(int, val_arr, val_max);

		do
		{
			AOC_CAT_DYN_ARR(int, val_arr, val_len, val_max, 
				extractValue(token));
		} while ((token = strtok(NULL, ",")) != NULL);

		(*reports)[len].vals = val_arr;
		(*reports)[len].len = val_len;
		len++;
	}

	*num_reports = len;

	return AOC_SUCCESS;
}

static void freeReports(struct pageReport *reports, const size_t len)
{
	if (reports != NULL)
	{
		size_t i;

		for (i = 0; i < len; i++)
		{
			AOC_FREE(reports[i].vals);
		}

		AOC_FREE(reports);
	}
}

static long int calculatePart1(const struct pageRule *rules, 
	const size_t num_rules, const struct pageReport *reports, 
	const size_t num_reports)
{
	long int ret = 0;
	size_t i, j, k;

	for (i = 0; i < num_reports; i++)
	{
		AOC_BOOL is_valid = AOC_TRUE;

		/* Can short circuit out with is_valid as soon as an invalid
		 * rule is found */
		for (j = 0; (j < reports[i].len - 1) && (is_valid == AOC_TRUE); 
			j++)
		{
			for (k = j; k < reports[i].len; k++)
			{
				struct pageRule key = {0};
				struct pageRule *found = NULL;

				/* Checks for a rule directly conflicting 
				 * with the observed order */
				key.left  = reports[i].vals[k];
				key.right = reports[i].vals[j];

				if ((found = bsearch(&key, rules, num_rules, 
					sizeof(struct pageRule), compareRules))
						!= NULL)
				{
					is_valid = AOC_FALSE;

					break;
				}
			}
		}

		if (is_valid == AOC_TRUE)
		{
			AOC_ASSERT(reports[i].len % 2 != 0);
			ret += (reports[i].vals[(reports[i].len >> 1)]);
		}
	}

	return ret;
}

/* There is likely a better way to accomplish this besides what here is 
 * essentially a version of swap sort. In production one would want to also
 * enforce a maximum number of passes to avoid deadlock caused by an invalid
 * set of rules that have a circular logic to them:
 * 	eg: 10 | 15
 * 	    15 | 10 
 * This could also be accomplished at rules loading time by disallowing any
 * rule that mirrors an existing rule */
static AOC_BOOL fixReport(const struct pageRule *rules, const size_t num_rules, 
	const struct pageReport *report)
{
	size_t i, j;
	AOC_BOOL already_valid = AOC_TRUE;
	AOC_BOOL is_fixed = AOC_TRUE;

	/* Runs this loop until the entire report has been traversed without
	 * requiring any swaps, it should require only one pass with this
	 * logic but it doesn't hurt to make sure */
	do
	{
		is_fixed = AOC_TRUE;

		for (i = 0; i < report->len - 1; i++)
		{
			for (j = i; j < report->len; j++)
			{
				struct pageRule key = {0};
				struct pageRule *found = NULL;

				key.left  = report->vals[j];
				key.right = report->vals[i];

				if ((found = bsearch(&key, rules, num_rules, 
					sizeof(struct pageRule), compareRules)) 
						!= NULL)
				{
					already_valid = AOC_FALSE;
					is_fixed = AOC_FALSE;
					AOC_SWAP(int, report->vals[i], 
						report->vals[j]);
				}
			}
		}
	} while (is_fixed == AOC_FALSE);

	return already_valid;
}

static long int calculatePart2(const struct pageRule *rules, 
	const size_t num_rules, struct pageReport *reports, 
	const size_t num_reports)
{
	long int ret = 0;
	size_t i;

	for (i = 0; i < num_reports; i++)
	{
		const AOC_BOOL already_valid = fixReport(rules, num_rules,
			&reports[i]);

		if (already_valid == AOC_FALSE)
		{
			/* Asserts that the report processing has remedied the
			 * error */
			AOC_ASSERT(fixReport(rules, num_rules, 
				&reports[i]) == AOC_TRUE);
			AOC_ASSERT(reports[i].len % 2 != 0);
			ret += (reports[i].vals[(reports[i].len >> 1)]);
		}
	}

	return ret;
}

int main(int argc, char **argv)
{
	size_t i;

	if (argc < 2)
	{
		fputs("Please supply an input file or files to act upon\n", 
			stderr);

		return AOC_FAILURE;
	}

	for (i = 1; i < argc; i++)
	{
		struct pageReport *page_reports = NULL;
		struct pageRule *page_rules = NULL;
		size_t num_reports = 0;
		size_t num_rules = 0;
		FILE *input = NULL;

		if ((input = fopen(argv[i], "rb")) == NULL)
		{
			fprintf(stderr, 
				"Unable to open file '%s' for reading\n", 
				argv[i]);

			return AOC_FAILURE;
		}

		if (parseFile(input, &page_rules, &num_rules, &page_reports, 
			&num_reports) != AOC_SUCCESS)
		{
			fprintf(stderr, "Failed to parse input file '%s'\n", 
				argv[1]);
			AOC_FREE(page_rules);
			freeReports(page_reports, num_reports);
			fclose(input);

			return AOC_FAILURE;
		}

		fprintf(stdout, "Part 1: %ld\n", 
			calculatePart1(page_rules, num_rules, page_reports, 
				num_reports));
		fprintf(stdout, "Part 2: %ld\n", 
			calculatePart2(page_rules, num_rules, page_reports, 
				num_reports));

		AOC_FREE(page_rules);
		freeReports(page_reports, num_reports);
		fclose(input);
	}

	return AOC_SUCCESS;
}

