#include <stdlib.h>
#include "world_normal.h"

/* private declarations */

static unsigned char _world_normal_get_cell(const struct world *this, int i, int j);

static void _world_normal_set_cell(struct world *this, int i, int j, unsigned char lifeness);

static unsigned char _world_normal_get_cell_previous(const struct world *this, int i, int j);

static void _world_normal_set_cell_previous(struct world *this, int i, int j, unsigned char lifeness);

/* public & private implementations */

struct world_normal *world_normal_alloc(int rows, int cols)
{
    struct world_normal *result = (struct world_normal *) (malloc(sizeof(struct world_normal)));

    world_normal_init(result, rows, cols);

    return result;
}

void world_normal_init(struct world_normal *this, int rows, int cols)
{
    world_init(&this->super, rows, cols);

    this->init_empty = this->super.init_empty;
    this->init_with_density = this->super.init_with_density;
    this->next_gen = this->super.next_gen;
    this->print = this->super.print;
    this->load = this->super.load;
    this->save = this->super.save;

    this->get_cell = this->super.get_cell = _world_normal_get_cell;
    this->set_cell = this->super.set_cell = _world_normal_set_cell;
    this->get_cell_previous = this->super.get_cell_previous = _world_normal_get_cell_previous;
    this->set_cell_previous = this->super.set_cell_previous = _world_normal_set_cell_previous;
}

void world_normal_free(struct world_normal *w)
{
    world_free(&w->super);
}


static unsigned char _world_normal_get_cell(const struct world *this, int i, int j)
{
    if (i >= 0 && i < this->get_rows(this) && j >= 0 && j < this->get_cols(this))
	    return this->get_current_matrix(this)[this->get_cols(this) * i + j];
    else
	    return DEAD;
}


static void _world_normal_set_cell(struct world *this, int i, int j, unsigned char lifeness)
{
    if (i >= 0 && i < this->get_rows(this) && j >= 0 && j < this->get_cols(this))
	    this->get_current_matrix(this)[this->get_cols(this) * i + j] = lifeness;
}


static unsigned char _world_normal_get_cell_previous(const struct world *this, int i, int j)
{
    if (i >= 0 && i < this->get_rows(this) && j >= 0 && j < this->get_cols(this))
	    return this->get_previous_matrix(this)[this->get_cols(this) * i + j];
    else
	    return DEAD;
}


static void _world_normal_set_cell_previous(struct world *this, int i, int j, unsigned char lifeness)
{
    if (i >= 0 && i < this->get_rows(this) && j >= 0 && j < this->get_cols(this))
	this->get_previous_matrix(this)[this->get_cols(this) * i + j] = lifeness;
}
