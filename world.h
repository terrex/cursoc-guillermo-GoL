#ifndef __WORLD_H__
#define __WORLD_H__

struct world;

enum lifeness {
	ALIVE,
	DEAD
};

struct world * world_random(void);

struct world * world_free(struct world *w);

void world_print(const struct world *w);

#endif /* __WORLD_H__ */
