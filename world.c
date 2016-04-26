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

/* private definitions */

#define ATTR_SET(flags, attr) (flags) |= (1 << (attr))
#define ATTR_IS_SET(flags, attr) ((flags) & (1 << (attr)))

enum world_flags {
	WORLD_MATRICES_ALLOCATED,
};

struct list_element {
	int i;
	int j;
	struct list_head list;
};

static struct list_element *_list_element_new(int i, int j);

static void _world_init_empty(struct world *this);

static void _world_init_density(struct world *this, int density);

static unsigned char _world_get_cell(const struct world *this, int i, int j);

static void _world_set_cell(struct world *this, int i, int j, unsigned char lifeness);

static unsigned char _world_get_cell_previous(const struct world *this, int i, int j);

static void _world_set_cell_previous(struct world *this, int i, int j, unsigned char lifeness);

static inline int _rrand(int from, int to)
{
	return rand() % (to - from) + from;
}

static unsigned char _next_state_of(const struct world *this, int i, int j);

static void _world_next_gen(struct world *this);

static void _world_print(const struct world *this);

#define _NW(i, j) i - 1, j - 1
#define _N_(i, j) i - 1, j + 0
#define _NE(i, j) i - 1, j + 1
#define _W_(i, j) i + 0, j - 1
#define _O_(i, j) i + 0, j + 0
#define _E_(i, j) i + 0, j + 1
#define _SW(i, j) i + 1, j - 1
#define _S_(i, j) i + 1, j + 0
#define _SE(i, j) i + 1, j + 1

static void _world_load(struct world *this, FILE *stream);

static void _world_save(const struct world *this, FILE *stream);

/* private & public implementations */

static struct list_element *_list_element_new(int i, int j)
{
	struct list_element *result = malloc(sizeof(struct list_element));

	result->i = i;
	result->j = j;
	return result;
}


struct world *world_alloc(int rows, int cols)
{
	struct world *result = (struct world *) (malloc(sizeof(struct world)));

	world_init(result, rows, cols);

	return result;
}

void world_init(struct world *this, int rows, int cols)
{
	this->rows = rows;
	this->cols = cols;
	this->current_matrix = (unsigned char *) (calloc((size_t) rows * cols, sizeof(unsigned char)));
	this->previous_matrix = (unsigned char *) (calloc((size_t) rows * cols, sizeof(unsigned char)));
	this->flags = 0;
	this->flags |= (1 << WORLD_MATRICES_ALLOCATED);
	INIT_LIST_HEAD(&this->alive_list);
	this->alive_cells_count = 0;
	this->generation =  0;

	this->init_empty = _world_init_empty;
	this->init_with_density = _world_init_density;
	this->next_gen = _world_next_gen;
	this->print = _world_print;
	this->get_cell = _world_get_cell;
	this->set_cell = _world_set_cell;
	this->get_cell_previous = _world_get_cell_previous;
	this->set_cell_previous = _world_set_cell_previous;
	this->load = _world_load;
	this->save = _world_save;
}


static void _world_init_empty(struct world *this)
{
	assert(ATTR_IS_SET(this->flags, WORLD_MATRICES_ALLOCATED));

	memset(this->previous_matrix, DEAD, this->rows * this->cols);
	memset(this->current_matrix, DEAD, this->rows * this->cols);

	struct list_element *it, *_t;

	list_for_each_entry_safe(it, _t, &this->alive_list, list) {
		list_del(&it->list);
		free(it);
	}

	INIT_LIST_HEAD(&this->alive_list);
	this->alive_cells_count = 0;
	this->generation = 0;
}


static void _world_init_density(struct world *this, int density)
{
	assert(density <= 100);
	this->init_empty(this);

	int to_be_alive = this->rows * this->cols * density / 100;
	int collisions = 0;

	srand((unsigned int) time(0));

	while (this->alive_cells_count < to_be_alive) {
		int i = _rrand(0, this->rows);
		int j = _rrand(0, this->cols);

		if (this->get_cell(this, i, j) == DEAD) {
			this->set_cell(this, i, j, ALIVE);
			struct list_element *le = _list_element_new(i, j);

			list_add(&le->list, &this->alive_list);
			this->alive_cells_count++;
			collisions = 0;
		} else {
			if (collisions++ >= 3)
				to_be_alive--;
		}
	}

	this->generation = 1;
}


static unsigned char _world_get_cell(const struct world *this, int i, int j)
{
	if (i < 0)
		i = 0;
	if (i >= this->rows)
		i = this->rows - 1;
	if (j < 0)
		j = 0;
	if (j >= this->cols)
		j = this->cols - 1;
	return this->current_matrix[this->cols * i + j];
}


static void _world_set_cell(struct world *this, int i, int j, unsigned char lifeness)
{
	if (i < 0)
		i = 0;
	if (i >= this->rows)
		i = this->rows - 1;
	if (j < 0)
		j = 0;
	if (j >= this->cols)
		j = this->cols - 1;
	this->current_matrix[this->cols * i + j] = lifeness;
}


static unsigned char _world_get_cell_previous(const struct world *this, int i, int j)
{
	if (i < 0)
		i = 0;
	if (i >= this->rows)
		i = this->rows - 1;
	if (j < 0)
		j = 0;
	if (j >= this->cols)
		j = this->cols - 1;
	return this->previous_matrix[this->cols * i + j];
}


static void _world_set_cell_previous(struct world *this, int i, int j, unsigned char lifeness)
{
	if (i < 0)
		i = 0;
	if (i >= this->rows)
		i = this->rows - 1;
	if (j < 0)
		j = 0;
	if (j >= this->cols)
		j = this->cols - 1;
	this->previous_matrix[this->cols * i + j] = lifeness;
}


static unsigned char _next_state_of(const struct world *this, int i, int j)
{
	int neighbourhood;

	neighbourhood = this->get_cell_previous(this, _NW(i, j)) +
					this->get_cell_previous(this, _N_(i, j)) +
					this->get_cell_previous(this, _NE(i, j)) +
					this->get_cell_previous(this, _W_(i, j)) +
					this->get_cell_previous(this, _O_(i, j)) +
					this->get_cell_previous(this, _E_(i, j)) +
					this->get_cell_previous(this, _SW(i, j)) +
					this->get_cell_previous(this, _S_(i, j)) +
					this->get_cell_previous(this, _SE(i, j));

	if (this->get_cell_previous(this, _O_(i, j)) == DEAD && neighbourhood == 3)
		return ALIVE;
	else if (this->get_cell_previous(this, _O_(i, j)) == ALIVE &&
			 (neighbourhood == 2 || neighbourhood == 3))
		return ALIVE;
	else
		return DEAD;
}


static void _world_next_gen(struct world *this)
{
	assert(ATTR_IS_SET(this->flags, WORLD_MATRICES_ALLOCATED));

	/* swap matrices */
	unsigned char *_tp = this->previous_matrix;

	this->previous_matrix = this->current_matrix;
	this->current_matrix = _tp;
	memset(this->current_matrix, DEAD, this->rows * this->cols);

	struct list_element *it, *_t;
	struct list_head to_be_checked;

	/* make a list of current alive cells and its 8 neighbours for being later checked */
	INIT_LIST_HEAD(&to_be_checked);
	int i, j;

	list_for_each_entry_safe(it, _t, &(this->alive_list), list) {
		i = it->i;
		j = it->j;
		if (this->get_cell_previous(this, _NW(i, j)) == DEAD)
			list_add(&_list_element_new(_NW(i, j))->list, &to_be_checked);
		if (this->get_cell_previous(this, _N_(i, j)) == DEAD)
			list_add(&_list_element_new(_N_(i, j))->list, &to_be_checked);
		if (this->get_cell_previous(this, _NE(i, j)) == DEAD)
			list_add(&_list_element_new(_NE(i, j))->list, &to_be_checked);

		if (this->get_cell_previous(this, _W_(i, j)) == DEAD)
			list_add(&_list_element_new(_W_(i, j))->list, &to_be_checked);
		list_move(&it->list, &to_be_checked);
		this->alive_cells_count--;
		if (this->get_cell_previous(this, _E_(i, j)) == DEAD)
			list_add(&_list_element_new(_E_(i, j))->list, &to_be_checked);

		if (this->get_cell_previous(this, _SW(i, j)) == DEAD)
			list_add(&_list_element_new(_SW(i, j))->list, &to_be_checked);
		if (this->get_cell_previous(this, _S_(i, j)) == DEAD)
			list_add(&_list_element_new(_S_(i, j))->list, &to_be_checked);
		if (this->get_cell_previous(this, _SE(i, j)) == DEAD)
			list_add(&_list_element_new(_SE(i, j))->list, &to_be_checked);
	}

	/* alive_list should be empty now */
	assert(list_empty(&this->alive_list));
	assert(this->alive_cells_count == 0);

	/* check cells that may have changed their state */
	unsigned char next_state;

	list_for_each_entry_safe(it, _t, &(to_be_checked), list) {
		i = it->i;
		j = it->j;
		next_state = _next_state_of(this, i, j);
		if (next_state == ALIVE && this->get_cell(this, _O_(i, j)) == DEAD) {
			this->set_cell(this, i, j, ALIVE);
			list_move(&it->list, &this->alive_list);
			this->alive_cells_count++;
		} else {
			list_del(&it->list);
			free(it);
		}
	}

	/* list to_be_checked must be empty now */
	assert(list_empty(&to_be_checked));
	this->generation++;
}


void world_free(struct world *w)
{
	if (w != NULL) {
		if (ATTR_IS_SET(w->flags, WORLD_MATRICES_ALLOCATED)) {
			free(w->previous_matrix);
			free(w->current_matrix);
		}

		struct list_element *it, *_t;

		list_for_each_entry_safe(it, _t, &w->alive_list, list) {
			list_del(&it->list);
			free(it);
		}
	}

	free(w);
}


static void _world_print(const struct world *this)
{
	int z = this->cols;

	printf("/");
	while (z--)
		printf("-");
	printf("\\\n");

	for (int i = 0; i < this->rows; i++) {
		printf("|");
		for (int j = 0; j < this->cols; j++) {
			if (this->get_cell(this, i, j) == ALIVE)
				printf("o");
			else
				printf(" ");
		}
		printf("|\n");
	}

	z = this->cols;
	printf("\\");
	while (z--)
		printf("-");
	printf("/\n");
	printf("Alive cells count: %5d  (%5.2f%%)\n", this->alive_cells_count,
		   (float) this->alive_cells_count / (this->cols * this->rows) * 100);
}

static void _world_load(struct world *this, FILE *stream)
{
	struct world read;

	assert(ATTR_IS_SET(this->flags, WORLD_MATRICES_ALLOCATED));
	fread(&read, sizeof(struct world), 1, stream);
	assert(this->rows == read.rows && this->cols == read.cols);
	this->generation = read.generation;
	fread(this->previous_matrix, sizeof(unsigned char), (size_t)(this->cols * this->rows), stream);
	fread(this->current_matrix, sizeof(unsigned char), (size_t)(this->cols * this->rows), stream);

	INIT_LIST_HEAD(&this->alive_list);
	this->alive_cells_count = 0;
	for (int i = 0; i < this->rows; i++) {
		for (int j = 0; j < this->cols; j++) {
			if (this->get_cell(this, _O_(i, j)) == ALIVE) {
				struct list_element *le = _list_element_new(i, j);

				list_add(&le->list, &this->alive_list);
				this->alive_cells_count++;
			}
		}
	}
}

static void _world_save(const struct world *this, FILE *stream)
{
	fwrite(this, sizeof(struct world), 1, stream);
	if (ATTR_IS_SET(this->flags, WORLD_MATRICES_ALLOCATED)) {
		fwrite(this->previous_matrix, sizeof(unsigned char), (size_t) (this->cols * this->rows), stream);
		fwrite(this->current_matrix, sizeof(unsigned char), (size_t) (this->cols * this->rows), stream);
	}
}
