#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/rand.h>
#undef NDEBUG
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
	int alive_cells_count;
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
				result->alive_cells_count++;
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
	fprintf(stderr, "[%d,%d] = %d\n", i, j, neighbourhood);

	if (_O_(before, i, j) == DEAD && neighbourhood == 3)
		return ALIVE;
	else if (_O_(before, i, j) == ALIVE &&
			 (neighbourhood == 2 || neighbourhood == 3))
		return ALIVE;
	else
		return DEAD;
}

void world_next_gen(struct world *before, struct world *after)
{
	assert(before != NULL);
	assert(after != NULL);
	assert(before->rows == after->rows && before->cols == after->cols);

	struct list_element *it, *_t;

	struct list_head to_be_checked;

	INIT_LIST_HEAD(&to_be_checked);

	/* may a list of current alive cells and its 8 neighbours for being later checked */
	int _dbg_alive_list_count = 0;

	list_for_each_entry_safe(it, _t, &(before->alive_list), list) {
		if (_NW(before, it->i, it->j) == DEAD)
			list_add(&list_element_new(it->i - 1, it->j - 1)->list, &to_be_checked);
		if (_N_(before, it->i, it->j) == DEAD)
			list_add(&list_element_new(it->i - 1, it->j + 0)->list, &to_be_checked);
		if (_NE(before, it->i, it->j) == DEAD)
			list_add(&list_element_new(it->i - 1, it->j + 1)->list, &to_be_checked);

		if (_W_(before, it->i, it->j) == DEAD)
			list_add(&list_element_new(it->i + 0, it->j - 1)->list, &to_be_checked);
		list_move(&it->list, &to_be_checked);
		before->alive_cells_count--;
		if (_E_(before, it->i, it->j) == DEAD)
			list_add(&list_element_new(it->i + 0, it->j + 1)->list, &to_be_checked);

		if (_SW(before, it->i, it->j) == DEAD)
			list_add(&list_element_new(it->i + 1, it->j - 1)->list, &to_be_checked);
		if (_S_(before, it->i, it->j) == DEAD)
			list_add(&list_element_new(it->i + 1, it->j + 0)->list, &to_be_checked);
		if (_SE(before, it->i, it->j) == DEAD)
			list_add(&list_element_new(it->i + 1, it->j + 1)->list, &to_be_checked);
		_dbg_alive_list_count++;
	}
	printf("DBG alive list count: %d\n", _dbg_alive_list_count);

	/* alive_list should be empty now */
	assert(list_empty(&before->alive_list));
	assert(before->alive_cells_count == 0);

	/* starting new alive_list*/
	INIT_LIST_HEAD(&after->alive_list);
	after->alive_cells_count = 0;

	/* check cells that may have changed their state */
	int _dbg_to_be_checked_list_count = 0;

	list_for_each_entry_safe(it, _t, &(to_be_checked), list) {
		enum lifeness next_state;

		next_state = _next_state_of(before, it->i, it->j);
		_O_(after, it->i, it->j) = next_state;
		if (next_state == ALIVE) {
			list_move(&it->list, &after->alive_list);
			after->alive_cells_count++;
		} else {
			list_del(&it->list);
			free(it);
		}
		_dbg_to_be_checked_list_count++;
	}
	printf("DBG to_be_checked list count: %d\n", _dbg_alive_list_count);

	/* list to_be_checked must be empty now */
	assert(list_empty(&to_be_checked));
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
	printf("Alive cells count: %d\n", w->alive_cells_count);
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
	result->alive_cells_count = 0;

	return result;
}

struct world *world_dup(const struct world *w)
{
	struct world *result = world_alloc(w->rows, w->cols);

	world_copy(result, w);
	return result;
}
