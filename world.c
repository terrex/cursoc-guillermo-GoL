#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/rand.h>
#include "world.h"

#define ROWS 8
#define COLS 16

struct world {
	short rows;
	short cols;
	unsigned char matrix[ROWS][COLS];
};

struct world *world_random(void)
{
	struct world *result = calloc(1, sizeof(struct world));

	result->rows = ROWS;
	result->cols = COLS;

	RAND_pseudo_bytes((unsigned char *)(result->matrix),
		result->rows * result->cols);

	/* la celda estará ALIVE con una probabilidad del (55 / 256) */
	for (int i = 0; i < result->rows; i++)
		for (int j = 0; j < result->cols; j++)
			result->matrix[i][j] = (result->matrix[i][j] < 55);

	return result;
}

struct world *world_next_gen(struct world *before)
{
	int neighbourhood;
	struct world *after = malloc(sizeof(*before));

	memcpy(after, before, sizeof(*before));

	for (int i = 0; i < after->rows; i++) {
		for (int j = 0; j < after->cols; j++) {
			neighbourhood =
before->matrix[(i - 1) % after->rows][(j - 1) % after->cols] + /* NW */
before->matrix[(i - 1) % after->rows][(j + 0) % after->cols] + /* N */
before->matrix[(i - 1) % after->rows][(j + 1) % after->cols] + /* NE */
before->matrix[(i + 0) % after->rows][(j - 1) % after->cols] + /* W */
before->matrix[(i + 0) % after->rows][(j + 1) % after->cols] + /* E */
before->matrix[(i + 1) % after->rows][(j - 1) % after->cols] + /* SW */
before->matrix[(i + 1) % after->rows][(j + 0) % after->cols] + /* S */
before->matrix[(i + 1) % after->rows][(j + 1) % after->cols] ; /* SE */

			if (before->matrix[i][j] == DEAD && neighbourhood == 3)
				after->matrix[i][j] = ALIVE;
			else if (before->matrix[i][j] == ALIVE &&
				(neighbourhood == 2 || neighbourhood == 3))
				after->matrix[i][j] = ALIVE;
			else
				after->matrix[i][j] = DEAD;
		}
	}

	before = world_free(before);
	return after;
}

struct world *world_free(struct world *w)
{
	if (w != NULL)
		free(w);
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
			if (w->matrix[i][j] == ALIVE)
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
