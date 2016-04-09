#include <stdlib.h>
#include "world.h"

#define ROWS 8
#define COLS 8

struct world {
	short rows;
	short cols;
	char matrix[ROWS][COLS];
};

struct world * world_random(void)
{
	struct world *result = calloc(1, sizeof(result));
	result->rows = ROWS;
	result->cols = COLS;
	return result;
}

struct world * world_free(struct world *w)
{
	if (w != NULL)
		free(w);
	return NULL;
}

void world_print(const struct world *w)
{
	
}
