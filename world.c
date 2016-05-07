#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>

#ifndef typeof
#define typeof __typeof__
#endif /* typeof */
#include "list.h"
#include "world.h"

/* private definitions */

struct world_private {
	int rows;
	int cols;
	unsigned char *previous_matrix;
	unsigned char *current_matrix;
	struct list_head alive_list;
	int alive_cells_count;
	int generation;
	uint32_t flags;
};

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

static int _world_get_rows(const struct world *this);
static int _world_get_cols(const struct world *this);
static int _world_get_alive_cells_count(const struct world *this);
static int _world_get_generation(const struct world *this);
static unsigned char *_world_get_current_matrix(const struct world *this);
static unsigned char *_world_get_previous_matrix(const struct world *this);
static void _world_refresh_alive_cells_index(struct world *this);

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
	this->world_pr = (struct world_private *) (malloc(sizeof(struct world_private)));
	this->world_pr->rows = rows;
	this->world_pr->cols = cols;
	this->world_pr->current_matrix = (unsigned char *) (calloc((size_t) rows * cols, sizeof(unsigned char)));
	this->world_pr->previous_matrix = (unsigned char *) (calloc((size_t) rows * cols, sizeof(unsigned char)));
	this->world_pr->flags = 0;
	this->world_pr->flags |= (1 << WORLD_MATRICES_ALLOCATED);
	INIT_LIST_HEAD(&this->world_pr->alive_list);
	this->world_pr->alive_cells_count = 0;
	this->world_pr->generation =  0;

	this->get_rows = _world_get_rows;
	this->get_cols = _world_get_cols;
	this->get_alive_cells_count = _world_get_alive_cells_count;
	this->get_generation = _world_get_generation;
	this->get_current_matrix = _world_get_current_matrix;
	this->get_previous_matrix = _world_get_previous_matrix;

	this->init_empty = _world_init_empty;
	this->init_with_density = _world_init_density;
	this->next_gen = _world_next_gen;
	this->print = _world_print;
	this->get_cell = NULL;
	this->set_cell = NULL;
	this->get_cell_previous = NULL;
	this->set_cell_previous = NULL;
	this->load = _world_load;
	this->save = _world_save;
	this->refresh_alive_cells_index = _world_refresh_alive_cells_index;
}


static void _world_init_empty(struct world *this)
{
	assert(ATTR_IS_SET(this->world_pr->flags, WORLD_MATRICES_ALLOCATED));

	memset(this->world_pr->previous_matrix, DEAD, this->world_pr->rows * this->world_pr->cols);
	memset(this->world_pr->current_matrix, DEAD, this->world_pr->rows * this->world_pr->cols);

	struct list_element *it, *_t;

	list_for_each_entry_safe(it, _t, &this->world_pr->alive_list, list) {
		list_del(&it->list);
		free(it);
	}

	INIT_LIST_HEAD(&this->world_pr->alive_list);
	this->world_pr->alive_cells_count = 0;
	this->world_pr->generation = 0;
}


static void _world_init_density(struct world *this, int density)
{
	static bool seeded;

	assert(density <= 100);
	this->init_empty(this);

	int to_be_alive = this->world_pr->rows * this->world_pr->cols * density / 100;
	int collisions = 0;

	if (!seeded) {
		srand((unsigned int) time(0));
		seeded = true;
	}

	while (this->world_pr->alive_cells_count < to_be_alive) {
		int i = _rrand(0, this->world_pr->rows);
		int j = _rrand(0, this->world_pr->cols);

		if (this->get_cell(this, i, j) == DEAD) {
			this->set_cell(this, i, j, ALIVE);
			struct list_element *le = _list_element_new(i, j);

			list_add(&le->list, &this->world_pr->alive_list);
			this->world_pr->alive_cells_count++;
			collisions = 0;
		} else {
			if (collisions++ >= 3)
				to_be_alive--;
		}
	}

	this->world_pr->generation = 1;
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
	assert(ATTR_IS_SET(this->world_pr->flags, WORLD_MATRICES_ALLOCATED));

	/* swap matrices */
	unsigned char *_tp = this->world_pr->previous_matrix;

	this->world_pr->previous_matrix = this->world_pr->current_matrix;
	this->world_pr->current_matrix = _tp;
	memset(this->world_pr->current_matrix, DEAD, this->world_pr->rows * this->world_pr->cols);

	struct list_element *it, *_t;
	struct list_head to_be_checked;

	/* make a list of current alive cells and its 8 neighbours for being later checked */
	INIT_LIST_HEAD(&to_be_checked);
	int i, j;

	list_for_each_entry_safe(it, _t, &(this->world_pr->alive_list), list) {
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
		this->world_pr->alive_cells_count--;
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
	assert(list_empty(&this->world_pr->alive_list));
	assert(this->world_pr->alive_cells_count == 0);

	/* check cells that may have changed their state */
	unsigned char next_state;

	list_for_each_entry_safe(it, _t, &(to_be_checked), list) {
		i = it->i;
		j = it->j;
		next_state = _next_state_of(this, i, j);
		if (next_state == ALIVE && this->get_cell(this, _O_(i, j)) == DEAD) {
			this->set_cell(this, i, j, ALIVE);
			list_move(&it->list, &this->world_pr->alive_list);
			this->world_pr->alive_cells_count++;
		} else {
			list_del(&it->list);
			free(it);
		}
	}

	/* list to_be_checked must be empty now */
	assert(list_empty(&to_be_checked));
	this->world_pr->generation++;
}


void world_free(struct world *w)
{
	if (w != NULL) {
		if (ATTR_IS_SET(w->world_pr->flags, WORLD_MATRICES_ALLOCATED)) {
			free(w->world_pr->previous_matrix);
			free(w->world_pr->current_matrix);
		}

		struct list_element *it, *_t;

		list_for_each_entry_safe(it, _t, &w->world_pr->alive_list, list) {
			list_del(&it->list);
			free(it);
		}

		free(w->world_pr);
	}

	free(w);
}


static void _world_print(const struct world *this)
{
	int z = this->world_pr->cols;

	printf("/");
	while (z--)
		printf("-");
	printf("\\\n");

	for (int i = 0; i < this->world_pr->rows; i++) {
		printf("|");
		for (int j = 0; j < this->world_pr->cols; j++) {
			if (this->get_cell(this, i, j) == ALIVE)
				printf("o");
			else
				printf(" ");
		}
		printf("|\n");
	}

	z = this->world_pr->cols;
	printf("\\");
	while (z--)
		printf("-");
	printf("/\n");
	printf("Alive cells count: %5d  (%5.2f%%)\n", this->world_pr->alive_cells_count,
		   (float) this->world_pr->alive_cells_count / (this->world_pr->cols * this->world_pr->rows) * 100);
}

static void _world_load(struct world *this, FILE *stream)
{
	struct world_private read;

	assert(ATTR_IS_SET(this->world_pr->flags, WORLD_MATRICES_ALLOCATED));
	if (fread(&read, sizeof(struct world_private), 1, stream) != 1) {
		perror("error loading struct world from file");
		exit(EXIT_FAILURE);
	}
	assert(this->world_pr->rows == read.rows && this->world_pr->cols == read.cols);
	this->world_pr->generation = read.generation;
	if (fread(this->world_pr->previous_matrix, sizeof(unsigned char), (size_t)(this->world_pr->cols * this->world_pr->rows), stream) != (size_t)(this->world_pr->cols * this->world_pr->rows)) {
		perror("error loading previous_matrix from file");
		exit(EXIT_FAILURE);
	}

	if (fread(this->world_pr->current_matrix, sizeof(unsigned char), (size_t)(this->world_pr->cols * this->world_pr->rows), stream) != (size_t)(this->world_pr->cols * this->world_pr->rows)) {
		perror("error loading current_matrix from file");
		exit(EXIT_FAILURE);
	}

	_world_refresh_alive_cells_index(this);
}

static void _world_save(const struct world *this, FILE *stream)
{
	if (fwrite(this->world_pr, sizeof(struct world_private), 1, stream) != 1) {
		perror("error writing struct world to file");
		exit(EXIT_FAILURE);
	}
	if (ATTR_IS_SET(this->world_pr->flags, WORLD_MATRICES_ALLOCATED)) {
		if (fwrite(this->world_pr->previous_matrix, sizeof(unsigned char), (size_t) (this->world_pr->cols * this->world_pr->rows), stream) != (size_t) (this->world_pr->cols * this->world_pr->rows)) {
			perror("error writing previous_matrix to file");
			exit(EXIT_FAILURE);
		}
		if (fwrite(this->world_pr->current_matrix, sizeof(unsigned char), (size_t) (this->world_pr->cols * this->world_pr->rows), stream) != (size_t) (this->world_pr->cols * this->world_pr->rows)) {
			perror("error writing current_matrix to file");
			exit(EXIT_FAILURE);
		}
	}
}

static int _world_get_rows(const struct world *this)
{
	return this->world_pr->rows;
}

static int _world_get_cols(const struct world *this)
{
	return this->world_pr->cols;
}
static int _world_get_alive_cells_count(const struct world *this)
{
	return this->world_pr->alive_cells_count;
}
static int _world_get_generation(const struct world *this)
{
	return this->world_pr->generation;
}
static unsigned char *_world_get_current_matrix(const struct world *this)
{
	return this->world_pr->current_matrix;
}

static unsigned char *_world_get_previous_matrix(const struct world *this)
{
	return this->world_pr->previous_matrix;
}

static void _world_refresh_alive_cells_index(struct world *this)
{
	INIT_LIST_HEAD(&this->world_pr->alive_list);
	this->world_pr->alive_cells_count = 0;
	for (int i = 0; i < this->world_pr->rows; i++) {
		for (int j = 0; j < this->world_pr->cols; j++) {
			if (this->get_cell(this, _O_(i, j)) == ALIVE) {
				struct list_element *le = _list_element_new(i, j);

				list_add(&le->list, &this->world_pr->alive_list);
				this->world_pr->alive_cells_count++;
			}
		}
	}
}
