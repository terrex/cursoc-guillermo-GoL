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

#define _NW(w, i, j) ((w)->matrix[((((i) - 1 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) - 1 + (w->cols)) % (w->cols))])
#define _N_(w, i, j) ((w)->matrix[((((i) - 1 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) + 0 + (w->cols)) % (w->cols))])
#define _NE(w, i, j) ((w)->matrix[((((i) - 1 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) + 1 + (w->cols)) % (w->cols))])
#define _W_(w, i, j) ((w)->matrix[((((i) + 0 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) - 1 + (w->cols)) % (w->cols))])
#define _O_(w, i, j) ((w)->matrix[((((i) + 0 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) + 0 + (w->cols)) % (w->cols))])
#define _E_(w, i, j) ((w)->matrix[((((i) + 0 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) + 1 + (w->cols)) % (w->cols))])
#define _SW(w, i, j) ((w)->matrix[((((i) + 1 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) - 1 + (w->cols)) % (w->cols))])
#define _S_(w, i, j) ((w)->matrix[((((i) + 1 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) + 0 + (w->cols)) % (w->cols))])
#define _SE(w, i, j) ((w)->matrix[((((i) + 1 + (w->rows)) % (w->rows)) * (w->cols)) + (((j) + 1 + (w->cols)) % (w->cols))])

struct list_element {
    int i;
    int j;
    struct list_head list;
};

struct list_element *list_element_new(int i, int j);

enum lifeness {
    DEAD = 0,
    ALIVE = 1,
};

#endif /* __WORLD_H__ */
