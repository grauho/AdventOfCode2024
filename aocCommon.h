#ifndef AOC_COMMON_H 
#define AOC_COMMON_H

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function" 
#endif

#include <stdio.h> /* for getFileLength */
#include <stdlib.h>

#ifndef AOC_MEMMOVE
#include <string.h>
#define AOC_MEMMOVE memmove
#endif

#ifndef AOC_ASSERT
#include <assert.h>
#define AOC_ASSERT assert
#endif /* AOC_ASSERT */

#define AOC_NEW_DYN_ARR(type, arr_ptr, max)       \
do                                                \
{                                                 \
	(arr_ptr) = malloc(sizeof(type) * (max)); \
	AOC_ASSERT((arr_ptr) != NULL);            \
} while (0)

/* Modifies both arr and len */
#define AOC_GROW_DYN_ARR(type, arr, len)                                \
do                                                                      \
{                                                                       \
	type *AOC_EXPAND_ARR;                                           \
	size_t AOC_EXPAND_LEN = ((len) == 0) ? (2) : (((len) * 3) / 2); \
	                                                                \
	AOC_EXPAND_ARR = realloc((arr), sizeof(type) * AOC_EXPAND_LEN); \
	AOC_ASSERT(AOC_EXPAND_ARR != NULL);                             \
	(arr) = AOC_EXPAND_ARR;                                         \
	(len) = AOC_EXPAND_LEN;                                         \
} while (0)

#define AOC_CAT_DYN_ARR(type, arr, len, max, val)     \
do                                                    \
{                                                     \
	if ((len) == (max))                           \
	{                                             \
		AOC_GROW_DYN_ARR(type, (arr), (max)); \
	}                                             \
	                                              \
	(arr)[(len)++] = (val);                       \
} while (0)

/* Insertion sort concatination. Entirely possible there is a way to do this 
 * in a regular function should one prefer */
#define AOC_INSERT_CAT(type, arr, len, max, val, CompFunc)              \
do                                                                      \
{                                                                       \
	if ((len) == (max))                                             \
	{                                                               \
		AOC_GROW_DYN_ARR(type, (arr), (max));                   \
	}                                                               \
	                                                                \
	if ((len) == 0)                                                 \
	{                                                               \
		(arr)[(len)++] = (val);                                 \
	}                                                               \
	else                                                            \
	{                                                               \
		const size_t AOC_INSERT_INDEX = leftBinSearch((arr),    \
			(len), &(val), sizeof(type), (CompFunc));       \
		                                                        \
		AOC_MEMMOVE(&((arr)[AOC_INSERT_INDEX + 1]),             \
			&((arr)[AOC_INSERT_INDEX]),                     \
			sizeof(type) * ((len) - AOC_INSERT_INDEX));     \
		(arr)[AOC_INSERT_INDEX] = (val);                        \
		(len)++;                                                \
	}                                                               \
} while (0)

#define AOC_FREE(ptr)         \
do                            \
{                             \
	if ((ptr) != NULL)    \
	{                     \
		free((ptr));  \
		(ptr) = NULL; \
	}                     \
} while (0)                   \

#define AOC_SWAP(type, left, right) \
do                                  \
{                                   \
	type AOC_SWAP_TMP = (left); \
	(left) = (right);           \
	(right) = AOC_SWAP_TMP;     \
} while (0)

#define AOC_BOOL    unsigned char
#define AOC_STAT    signed char
#define AOC_TRUE    (1)
#define AOC_FALSE   (0)
#define AOC_SUCCESS (0)
#define AOC_FAILURE (1)
#define AOC_ERROR   (-1)

#define AOC_ABS(x) (((x) < 0) ? ((x) * -1) : (x))
#define AOC_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define AOC_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define AOC_CLAMP(l, x, u) (AOC_MAX((l), (AOC_MIN((x), (u)))))

/* These binary search variant functions are here just because while the 
 * standard library does include bsearch its behavior is undefined for what
 * element it returns if the target element appears more than once. These 
 * return the leftmost and rightmost occurance of that element instead */

/* This is just a helper function because performing pointer arithmatic on
 * a void pointer is disallowed but casting it to a character pointer is valid
 * and also the only cast that doesn't violate strict aliasing */
#define AOC_ACCESS_VARR(arr, index, size) ((char *) (arr) + ((index) * (size)))

static size_t leftBinSearch(void *arr, const size_t len, const void *target,
	const size_t size, int (*CompFunc)(const void *, const void *))
{
	size_t left = 0;
	size_t right = len;

	AOC_ASSERT((arr != NULL) && (len != 0) && (target != NULL) 
		&& (CompFunc != NULL));
	
	while (left < right)
	{
		size_t mid = ((left + right) >> 1);

		if (CompFunc(AOC_ACCESS_VARR(arr, mid, size), target) < 0)
		{
			left = mid + 1;
		}
		else
		{
			right = mid;
		}
	}

	return left;
}

static size_t rightBinSearch(void *arr, const size_t len, const void *target,
	const size_t size, int (*CompFunc)(const void *, const void *))
{
	size_t left = 0;
	size_t right = len;

	AOC_ASSERT((arr != NULL) && (len != 0) && (target != NULL) 
		&& (CompFunc != NULL));
	
	while (left < right)
	{
		size_t mid = ((left + right) >> 1);

		if (CompFunc(AOC_ACCESS_VARR(arr, mid, size), target) > 0)
		{
			right = mid;
		}
		else
		{
			left = mid + 1;
		}
	}

	return right - 1;
}

static long int getFileLength(FILE *f_handle)
{
	long int length = -1;

	if (f_handle != NULL)
	{
		const long int initial = ftell(f_handle);

		if (fseek(f_handle, 0, SEEK_END) == -1)
		{
			return -1;
		}

		length = ftell(f_handle);

		if (fseek(f_handle, initial, SEEK_SET) == -1)
		{
			return -1;
		}
	}

	return length;
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif /* AOC_COMMON_H */

/*
Copyright (c) 2024, grauho <grauho@proton.me> All rights reserved.
*/
