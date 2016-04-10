#ifndef __WORLD_H__
#define __WORLD_H__

struct world;

enum lifeness {
	DEAD = 0,
	ALIVE = 1,
};

struct world * world_random(void);

struct world * world_next_gen(struct world *before);

struct world * world_free(struct world *w);

void world_print(const struct world *w);

#endif /* __WORLD_H__ */
