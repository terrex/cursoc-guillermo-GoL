#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/rand.h>
#include <assert.h>
#include <stdbool.h>
#include "world.h"
#define typeof __typeof__
#include "list.h"

#define ROWS 8
#define COLS 16
#define DEFAULT_DENSITY 22

struct list_element {
	int i;
	int j;
	struct list_head list;
	bool marked_for_deletion;
};

static inline struct list_element *list_element_new(int i, int j)
{
	struct list_element *result = malloc(sizeof(struct list_element));

	result->i = i;
	result->j = j;
	return result;
}

struct world {
	int rows;
	int cols;
	unsigned char *matrix;
	struct list_head alive_list;
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

struct world *world_random_with_size(int rows, int cols, int density)
{
	assert(density <= 100);
	struct world *result = world_alloc(rows, cols);

	RAND_pseudo_bytes(result->matrix, rows * cols);

	/* there will be a density percent of ALIVE cells on world */
	unsigned char threshold = (unsigned char) ((float) density / 100.0 * 255.0);

	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++) {
			enum lifeness ln = (enum lifeness) (_O_(result, i, j) < threshold);

			_O_(result, i, j) = ln;
			if (ln == ALIVE) {
				struct list_element *le = list_element_new(i, j);

				list_add(&le->list, &result->alive_list);
			}
		}

	return result;
}

static enum lifeness _next_state_of(const struct world *before, int i, int j)
{
	int neighbourhood;

	neighbourhood =
			_NW(before, i, j) + _N_(before, i, j) + _NE(before, i, j) +
			_W_(before, i, j) + _E_(before, i, j) +
			_SW(before, i, j) + _S_(before, i, j) + _SE(before, i, j);

	if (_O_(before, i, j) == DEAD && neighbourhood == 3)
		return ALIVE;
	else if (_O_(before, i, j) == ALIVE &&
			 (neighbourhood == 2 || neighbourhood == 3))
		return ALIVE;
	else
		return DEAD;
}

void world_next_gen(const struct world *before, struct world *after)
{
	assert(before != NULL);
	assert(after != NULL);
	assert(before->rows == after->rows && before->cols == after->cols);

	struct list_element *it, *_t;

	struct list_head to_be_checked;

	INIT_LIST_HEAD(&to_be_checked);

	/* check now alive */
	list_for_each_entry_safe(it, _t, &(before->alive_list), list) {
		enum lifeness next_state;

		next_state = _next_state_of(before, it->i, it->j);
		if (next_state == DEAD) {
			_O_(after, it->i, it->j) = next_state;

			list_add(&list_element_new(it->i - 1, it->j - 1)->list, &to_be_checked);
			list_add(&list_element_new(it->i - 1, it->j + 0)->list, &to_be_checked);
			list_add(&list_element_new(it->i - 1, it->j + 1)->list, &to_be_checked);

			list_add(&list_element_new(it->i + 0, it->j - 1)->list, &to_be_checked);
			list_add(&list_element_new(it->i + 0, it->j + 1)->list, &to_be_checked);

			list_add(&list_element_new(it->i + 1, it->j - 1)->list, &to_be_checked);
			list_add(&list_element_new(it->i + 1, it->j + 0)->list, &to_be_checked);
			list_add(&list_element_new(it->i + 1, it->j + 1)->list, &to_be_checked);

			list_del(&it->list);
			free(it);
		}
	}

	/* check cells that may have changed their state */
	list_for_each_entry_safe(it, _t, &(to_be_checked), list) {
		enum lifeness next_state, previous_state;

		previous_state = (enum lifeness) _O_(before, it->i, it->j);
		next_state = _next_state_of(before, it->i, it->j);
		if (previous_state == DEAD && next_state == ALIVE) {
			_O_(after, it->i, it->j) = next_state;

			list_add(&list_element_new(it->i, it->j)->list, &after->alive_list);
		}

		/* forget it anyway */
		list_del(&it->list);
		free(it);
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

struct world *world_alloc(int rows, int cols)
{
	struct world *result = (struct world *) (malloc(sizeof(struct world)));

	result->rows = rows;
	result->cols = cols;
	result->matrix = (unsigned char *) (malloc(rows * cols * sizeof(unsigned char)));
	INIT_LIST_HEAD(&result->alive_list);

	return result;
}

struct world *world_dup(const struct world *w)
{
	struct world *result = world_alloc(w->rows, w->cols);

	world_copy(result, w);
	return result;
}
