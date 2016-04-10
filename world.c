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
	char matrix[ROWS][COLS];
};

struct world *world_random(void)
{
	struct world *result = calloc(1, sizeof(struct world));

	result->rows = ROWS;
	result->cols = COLS;

	RAND_pseudo_bytes((unsigned char *)(result->matrix),
		result->rows * result->cols);

	/* me quedo con la bi-paridad del número para decidir DEAD o ALIVE */
	for (int i = 0; i < result->rows; i++)
		for (int j = 0; j < result->cols; j++)
			result->matrix[i][j] &= 11;

	return result;
}

struct world *world_next_gen(struct world *before)
{
	struct world *after = malloc(sizeof(*before));

	memcpy(after, before, sizeof(*before));
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
