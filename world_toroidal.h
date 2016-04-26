#ifndef __WORLD_TOROIDAL_H__
#define __WORLD_TOROIDAL_H__

#include <stdio.h>
#include "world.h"

struct world_toroidal {
    struct world super;

    void (*init_empty)(struct world *this);
    void (*init_with_density)(struct world *this, int density);
    void (*next_gen)(struct world *this);
    void (*print)(const struct world *this);
    unsigned char (*get_cell)(const struct world *this, int i, int j);
    void (*set_cell)(struct world *this, int i, int j, unsigned char lifeness);
    unsigned char (*get_cell_previous)(const struct world *this, int i, int j);
    void (*set_cell_previous)(struct world *this, int i, int j, unsigned char lifeness);

    void (*load)(struct world *this, FILE *stream);
    void (*save)(const struct world *this, FILE *stream);
};

struct world_toroidal *world_toroidal_alloc(int rows, int cols);

void world_toroidal_init(struct world_toroidal *this, int rows, int cols);

void world_toroidal_free(struct world_toroidal *w);

#endif /* __WORLD_TOROIDAL_H__ */
