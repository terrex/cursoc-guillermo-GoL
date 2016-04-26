#ifndef __WORLD_H__
#define __WORLD_H__

#include <stdint.h>

#include "list.h"

enum lifeness {
    DEAD = 0,
    ALIVE = 1,
};

struct world {
    int rows;
    int cols;
    unsigned char *previous_matrix;
    unsigned char *current_matrix;
    struct list_head alive_list;
    int alive_cells_count;
    int generation;
    uint32_t flags;

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

struct world *world_alloc(int rows, int cols);

void world_free(struct world *w);

#endif /* __WORLD_H__ */
