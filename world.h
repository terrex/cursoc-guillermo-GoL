#ifndef __WORLD_H__
#define __WORLD_H__

struct world;

struct world * world_random(void) __attribute__ ((deprecated));

struct world * world_random_with_size(int rows, int cols, int density);

struct world * world_next_gen(struct world *before);

struct world * world_free(struct world *w);

struct world * world_dup(const struct world *w);

void world_print(const struct world *w);

#endif /* __WORLD_H__ */
