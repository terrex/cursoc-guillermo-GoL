#include <stdio.h>
#include <unistd.h>
#include "world.h"
#include "game.h"

#define RESET_SCREEN "\x1B[1;1H\x1B[2J"

int main(int argc, char *argv[])
{
	struct game_config gc;

	game_config_defaults(&gc);
	game_parse_command_line_options(argc, argv, &gc);

	/* ===== Run program ===== */

	int it = 1;
	struct world *w = world_random_with_size(gc.rows, gc.cols, gc.density);
	struct world *w1 = world_alloc(gc.rows, gc.cols);
	struct world *wt;

	printf(RESET_SCREEN "World #%d:\n", it++);
	world_print(w);
	sleep(1);

	do {
		world_next_gen(w, w1);
		/* swap world pointers */
		wt = w1;
		w1 = w;
		w = wt;
		printf(RESET_SCREEN "World #%d:\n", it++);
		world_print(w);
		usleep(gc.speed);
	} while (it < gc.generations);

	world_free(w);
	world_free(w1);

	return 0;
}
