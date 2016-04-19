#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/rand.h>
#include <assert.h>
#include "world.h"

#define ROWS 8
#define COLS 16
#define DEFAULT_DENSITY 22

struct world {
	short rows;
	short cols;
	unsigned char *matrix;
};

enum lifeness {
	DEAD = 0,
	ALIVE = 1,
};

struct world *world_random(void)
{
	/*
	 * I do not simply drop `world_random' function because I want to
	 * keep the backward compatibility. Instead of that, I'll pass
	 * with default values for ROWS and COLS.
	 */
	return world_random_with_size(ROWS, COLS, DEFAULT_DENSITY);
}

struct world *world_random_with_size(int rows, int cols, int density)
{
	struct world *result = world_alloc(rows, cols);

	RAND_pseudo_bytes(result->matrix, result->rows * result->cols);

	/* there will be a density percent of ALIVE cells on world */
	unsigned char threshold = (unsigned char) ((float) density / 100.0 * 255.0);

	for (int i = 0; i < result->rows; i++)
		for (int j = 0; j < result->cols; j++)
			result->matrix[i * result->cols + j] = (result->matrix[i * result->cols + j] < threshold);

	return result;
}

void world_next_gen(const struct world *before, struct world *after)
{
	assert(before != NULL);
	assert(after != NULL);
	assert(before->rows == after->rows && before->cols == after->cols);

	int neighbourhood;

	register int r = before->rows;
	register int c = before->cols;

	for (int i = 0; i < after->rows; i++) {
		for (int j = 0; j < after->cols; j++) {
			neighbourhood =
before->matrix[((i - 1 + r) % r) * c + (j - 1 + c) % c] + /* NW */
before->matrix[((i - 1 + r) % r) * c + (j        ) % c] + /* N */
before->matrix[((i - 1 + r) % r) * c + (j + 1    ) % c] + /* NE */
before->matrix[((i        ) % r) * c + (j - 1 + c) % c] + /* W */
before->matrix[((i        ) % r) * c + (j + 1    ) % c] + /* E */
before->matrix[((i + 1    ) % r) * c + (j - 1 + c) % c] + /* SW */
before->matrix[((i + 1    ) % r) * c + (j        ) % c] + /* S */
before->matrix[((i + 1    ) % r) * c + (j + 1    ) % c] ; /* SE */

			if (before->matrix[i * before->cols + j] == DEAD && neighbourhood == 3)
				after->matrix[i * after->cols + j] = ALIVE;
			else if (before->matrix[i * after->cols + j] == ALIVE &&
				(neighbourhood == 2 || neighbourhood == 3))
				after->matrix[i * after->cols + j] = ALIVE;
			else
				after->matrix[i * after->cols + j] = DEAD;
		}
	}
}

void world_free(struct world *w)
{
	if (w != NULL)
		free(w->matrix);

	free(w);
}

void world_print(const struct world *w)
{
	assert(w != NULL);

	int z = w->cols;

	printf("/");
	while (z--)
		printf("-");
	printf("\\\n");

	for (int i = 0; i < w->rows; i++) {
		printf("|");
		for (int j = 0; j < w->cols; j++) {
			if (w->matrix[i * w->cols + j] == ALIVE)
				printf("o");
			else
				printf(" ");
		}
		printf("|\n");
	}

	z = w->cols;
	printf("\\");
	while (z--)
		printf("-");
	printf("/\n");
}

void world_copy(struct world *dest, const struct world *src)
{
	assert(dest != NULL);
	assert(src != NULL);
	assert(dest->rows == src->rows && dest->cols == src->cols);

	memcpy(dest->matrix, src->matrix, (dest->rows * dest->cols * sizeof(unsigned char)));
}

struct world *world_alloc(int rows, int cols)
{
	struct world *result = (struct world *) (malloc(sizeof(struct world)));

	result->rows = rows;
	result->cols = cols;
	result->matrix = (unsigned char *) (malloc(rows * cols * sizeof(unsigned char)));

	return result;
}

struct world *world_dup(const struct world *w)
{
	struct world *result = world_alloc(w->rows, w->cols);

	world_copy(result, w);
	return result;
}
