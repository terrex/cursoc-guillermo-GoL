#ifndef __WORLD_NORMAL_H__
#define __WORLD_NORMAL_H__

#include <stdio.h>
#include "world.h"

struct world_normal {
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

struct world_normal *world_normal_alloc(int rows, int cols);

void world_normal_init(struct world_normal *this, int rows, int cols);

void world_normal_free(struct world_normal *w);

#endif /* __WORLD_NORMAL_H__ */
