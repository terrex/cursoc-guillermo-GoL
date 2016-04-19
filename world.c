#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/rand.h>
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
	struct world *result = (struct world *) (calloc(1, sizeof(struct world)));

	result->rows = rows;
	result->cols = cols;
	result->matrix = (unsigned char *) (calloc(rows * cols, sizeof(unsigned char)));

	RAND_pseudo_bytes(result->matrix, result->rows * result->cols);

	/* habrá un PERCENT_ALIVE % de células vivas en el mapa */
	unsigned char threshold = (unsigned char) ((float) density / 100.0 * 255.0);

	for (int i = 0; i < result->rows; i++)
		for (int j = 0; j < result->cols; j++)
			result->matrix[i * result->cols + j] = (result->matrix[i * result->cols + j] < threshold);

	return result;
}

struct world *world_next_gen(struct world *before)
{
	int neighbourhood;
	struct world *after = world_dup(before);

	for (int i = 0; i < after->rows; i++) {
		for (int j = 0; j < after->cols; j++) {
			neighbourhood =
before->matrix[((i - 1) % after->rows) * after->cols + (j - 1) % after->cols] + /* NW */
before->matrix[((i - 1) % after->rows) * after->cols + (j + 0) % after->cols] + /* N */
before->matrix[((i - 1) % after->rows) * after->cols + (j + 1) % after->cols] + /* NE */
before->matrix[((i + 0) % after->rows) * after->cols + (j - 1) % after->cols] + /* W */
before->matrix[((i + 0) % after->rows) * after->cols + (j + 1) % after->cols] + /* E */
before->matrix[((i + 1) % after->rows) * after->cols + (j - 1) % after->cols] + /* SW */
before->matrix[((i + 1) % after->rows) * after->cols + (j + 0) % after->cols] + /* S */
before->matrix[((i + 1) % after->rows) * after->cols + (j + 1) % after->cols] ; /* SE */

			if (before->matrix[i * before->cols + j] == DEAD && neighbourhood == 3)
				after->matrix[i * after->cols + j] = ALIVE;
			else if (before->matrix[i * after->cols + j] == ALIVE &&
				(neighbourhood == 2 || neighbourhood == 3))
				after->matrix[i * after->cols + j] = ALIVE;
			else
				after->matrix[i * after->cols + j] = DEAD;
		}
	}

	before = world_free(before);
	return after;
}

struct world *world_free(struct world *w)
{
	if (w != NULL) {
		free(w->matrix);
		free(w);
	}

	return NULL;
}

void world_print(const struct world *w)
{
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

struct world *world_dup(const struct world *w)
{
	struct world *result = (struct world *) (malloc(sizeof(*w)));

	memcpy(result, w, sizeof(*w));
	result->matrix = (unsigned char *) (malloc(w->rows * w->cols * sizeof(unsigned char)));
	memcpy(result->matrix, w->matrix, (w->rows * w->cols * sizeof(unsigned char)));

	return result;
}
