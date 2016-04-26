#include <stdlib.h>
#include "world_toroidal.h"

/* private declarations */

static unsigned char _world_toroidal_get_cell(const struct world *this, int i, int j);

static void _world_toroidal_set_cell(struct world *this, int i, int j, unsigned char lifeness);

static unsigned char _world_toroidal_get_cell_previous(const struct world *this, int i, int j);

static void _world_toroidal_set_cell_previous(struct world *this, int i, int j, unsigned char lifeness);

/* public & private implementations */

struct world_toroidal *world_toroidal_alloc(int rows, int cols)
{
    struct world_toroidal *result = (struct world_toroidal *) (malloc(sizeof(struct world_toroidal)));

    world_toroidal_init(result, rows, cols);

    return result;
}

void world_toroidal_init(struct world_toroidal *this, int rows, int cols)
{
    world_init(&this->super, rows, cols);

    this->init_empty = this->super.init_empty;
    this->init_with_density = this->super.init_with_density;
    this->next_gen = this->super.next_gen;
    this->print = this->super.print;
    this->load = this->super.load;
    this->save = this->super.save;

    this->get_cell = this->super.get_cell = _world_toroidal_get_cell;
    this->set_cell = this->super.set_cell = _world_toroidal_set_cell;
    this->get_cell_previous = this->super.get_cell_previous = _world_toroidal_get_cell_previous;
    this->set_cell_previous = this->super.set_cell_previous = _world_toroidal_set_cell_previous;
}

void world_toroidal_free(struct world_toroidal *w)
{
    world_free(&w->super);
}


static unsigned char _world_toroidal_get_cell(const struct world *this, int i, int j)
{
    i = i % this->rows;
    j = j % this->cols;
    return this->current_matrix[this->cols * i + j];
}


static void _world_toroidal_set_cell(struct world *this, int i, int j, unsigned char lifeness)
{
    i = i % this->rows;
    j = j % this->cols;
    this->current_matrix[this->cols * i + j] = lifeness;
}


static unsigned char _world_toroidal_get_cell_previous(const struct world *this, int i, int j)
{
    i = i % this->rows;
    j = j % this->cols;
    return this->previous_matrix[this->cols * i + j];
}


static void _world_toroidal_set_cell_previous(struct world *this, int i, int j, unsigned char lifeness)
{
    i = i % this->rows;
    j = j % this->cols;
    this->previous_matrix[this->cols * i + j] = lifeness;
}
