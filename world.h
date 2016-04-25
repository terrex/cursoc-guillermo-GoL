#ifndef __WORLD_H__
#define __WORLD_H__

#include "list.h"

struct world {
    int rows;
    int cols;
    unsigned char *matrix;
    struct list_head alive_list;
    int alive_cells_count;
    int generation;
};

struct world *world_random(void) __attribute__ ((deprecated));

struct world *world_random_with_size(int rows, int cols, int density);

struct world *world_alloc(int rows, int cols);

void world_next_gen(struct world *before, struct world *after);

void world_free(struct world *w);

struct world *world_dup(const struct world *w);

void world_print(const struct world *w);

void world_copy(struct world *dest, const struct world *src);

#endif /* __WORLD_H__ */
