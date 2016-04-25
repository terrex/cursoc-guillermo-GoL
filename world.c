#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#ifndef typeof
#define typeof __typeof__
#endif /* typeof */
#include "list.h"
#include "world.h"

#define ROWS 8
#define COLS 16
#define DEFAULT_DENSITY 22

struct list_element *list_element_new(int i, int j)
{
	struct list_element *result = malloc(sizeof(struct list_element));

	result->i = i;
	result->j = j;
	return result;
}

static void _world_reset(struct world *w);

struct world *world_random(void)
{
	/*
	 * I do not simply drop `world_random' function because I want to
	 * keep the backward compatibility. Instead of that, I'll pass
	 * with default values for ROWS and COLS.
	 */
	return world_random_with_size(ROWS, COLS, DEFAULT_DENSITY);
}

static inline int rrand(int from, int to)
{
	return rand() % (to - from + 1) + from;
}

struct world *world_random_with_size(int rows, int cols, int density)
{
	assert(density <= 100);
	struct world *result = world_alloc(rows, cols);

	_world_reset(result);
	int to_be_alive = rows * cols * density / 100;
	int collisions = 0;

	srand((unsigned int) time(0));

	while (result->alive_cells_count < to_be_alive) {
		int i = rrand(0, rows);
		int j = rrand(0, cols);

		if (_O_(result, i, j) == DEAD) {
			_O_(result, i, j) = ALIVE;
			struct list_element *le = list_element_new(i, j);

			list_add(&le->list, &result->alive_list);
			result->alive_cells_count++;
			collisions = 0;
		} else {
			if (collisions++ >= 3)
				to_be_alive--;
		}
	}

	result->generation = 1;
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

static void _world_reset(struct world *w)
{
	memset(w->matrix, DEAD, w->rows * w->cols);

	struct list_element *it, *_t;

	list_for_each_entry_safe(it, _t, &w->alive_list, list) {
		list_del(&it->list);
		free(it);
	}

	INIT_LIST_HEAD(&w->alive_list);
	w->alive_cells_count = 0;
	w->generation = 0;
}

void world_next_gen(struct world *before, struct world *after)
{
	assert(before != NULL);
	assert(after != NULL);
	assert(before->rows == after->rows && before->cols == after->cols);

	_world_reset(after);

	struct list_element *it, *_t;
	struct list_head to_be_checked;

	/* make a list of current alive cells and its 8 neighbours for being later checked */
	INIT_LIST_HEAD(&to_be_checked);
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
	}

	/* alive_list should be empty now */
	assert(list_empty(&before->alive_list));
	assert(before->alive_cells_count == 0);

	/* check cells that may have changed their state */
	list_for_each_entry_safe(it, _t, &(to_be_checked), list) {
		enum lifeness next_state;

		next_state = _next_state_of(before, it->i, it->j);
		if (next_state == ALIVE && _O_(after, it->i, it->j) == DEAD) {
			_O_(after, it->i, it->j) = ALIVE;
			list_move(&it->list, &after->alive_list);
			after->alive_cells_count++;
		} else {
			list_del(&it->list);
			free(it);
		}
	}

	/* list to_be_checked must be empty now */
	assert(list_empty(&to_be_checked));
	after->generation = before->generation + 1;
}

void world_free(struct world *w)
{
	if (w != NULL)
		free(w->matrix);

	struct list_element *it, *_t;

	list_for_each_entry_safe(it, _t, &w->alive_list, list) {
		list_del(&it->list);
		free(it);
	}

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
	printf("Alive cells count: %5d  (%5.2f%%)\n", w->alive_cells_count,
		   (float) w->alive_cells_count / (w->cols * w->rows) * 100);
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
	result->generation =  0;

	return result;
}

struct world *world_dup(const struct world *w)
{
	struct world *result = world_alloc(w->rows, w->cols);

	world_copy(result, w);
	return result;
}
