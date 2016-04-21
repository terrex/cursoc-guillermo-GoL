#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/rand.h>
#include <assert.h>
#include "world.h"
#include "list.h"

#define ROWS 8
#define COLS 16
#define DEFAULT_DENSITY 22

struct list_element {
	unsigned int index;
	struct list_head list;
};

struct world {
	unsigned short rows;
	unsigned short cols;
	unsigned char *matrix;
	struct list_element *alive_list;
};

#define _NW(w, i, j) ((w)->matrix[((((i) - 1 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) - 1 + (w->cols)) % (w->cols))])
#define _N_(w, i, j) ((w)->matrix[((((i) - 1 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) + 0 + (w->cols)) % (w->cols))])
#define _NE(w, i, j) ((w)->matrix[((((i) - 1 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) + 1 + (w->cols)) % (w->cols))])
#define _W_(w, i, j) ((w)->matrix[((((i) + 0 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) - 1 + (w->cols)) % (w->cols))])
#define _O_(w, i, j) ((w)->matrix[((((i) + 0 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) + 0 + (w->cols)) % (w->cols))])
#define _E_(w, i, j) ((w)->matrix[((((i) + 0 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) + 1 + (w->cols)) % (w->cols))])
#define _SW(w, i, j) ((w)->matrix[((((i) + 1 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) - 1 + (w->cols)) % (w->cols))])
#define _S_(w, i, j) ((w)->matrix[((((i) + 1 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) + 0 + (w->cols)) % (w->cols))])
#define _SE(w, i, j) ((w)->matrix[((((i) + 1 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) + 1 + (w->cols)) % (w->cols))])

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

struct world *world_random_with_size(unsigned short rows, unsigned short cols, unsigned short density)
{
	assert(density <= 100);
	struct world *result = world_alloc(rows, cols);

	RAND_pseudo_bytes(result->matrix, rows * cols);

	/* there will be a density percent of ALIVE cells on world */
	unsigned char threshold = (unsigned char) ((float) density / 100.0 * 255.0);

	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++)
			_O_(result, i, j) = (unsigned char) (_O_(result, i, j) < threshold);

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

	for (int i = 0; i < r; i++) {
		for (int j = 0; j < c; j++) {
			neighbourhood =
					_NW(before, i, j) + _N_(before, i, j) + _NE(before, i, j) +
					_W_(before, i, j) + _E_(before, i, j) +
					_SW(before, i, j) + _S_(before, i, j) + _SE(before, i, j);

			if (_O_(before, i, j) == DEAD && neighbourhood == 3)
				_O_(after, i, j) = ALIVE;
			else if (_O_(before, i, j) == ALIVE &&
				(neighbourhood == 2 || neighbourhood == 3))
				_O_(after, i, j) = ALIVE;
			else
				_O_(after, i, j) = DEAD;
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
			if (_O_(w, i, j) == ALIVE)
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

struct world *world_alloc(unsigned short rows, unsigned short cols)
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
