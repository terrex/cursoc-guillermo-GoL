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

	struct world *w;

	if (gc.load_world[0] == '\0') {
		w = world_alloc(gc.rows, gc.cols);
		w->init_with_density(w, gc.density);
	} else
		game_alloc_n_load(&gc, &w);

	game_log_start(&gc);
	printf(RESET_SCREEN "World #%d:\n", w->generation);
	w->print(w);
	game_log_output(&gc, w);
	sleep(1);

	int gens = 1;

	do {
		w->next_gen(w);
		printf(RESET_SCREEN "World #%d:\n", w->generation);
		w->print(w);
		usleep(gc.speed);
		game_log_output(&gc, w);
	} while (++gens < gc.generations);

	game_log_stop(&gc);
	game_write(&gc, w);

	world_free(w);

	return 0;
}
