#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "world_normal.h"
#include "world_toroidal.h"
#include "game.h"

#define RESET_SCREEN "\x1B[1;1H\x1B[2J"

int main(int argc, char *argv[])
{
	struct game_config gc;

	game_config_defaults(&gc);
	game_parse_command_line_options(argc, argv, &gc, 0);

	/* ===== Run program ===== */

	struct world *w;

	if (gc.load_world[0] == '\0') {
		if (gc.game_type == TYPE_NORMAL)
			w = (struct world *) world_normal_alloc(gc.rows, gc.cols);
		else if (gc.game_type == TYPE_TOROIDAL)
			w = (struct world *) world_toroidal_alloc(gc.rows, gc.cols);
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

	if (gc.game_type == TYPE_NORMAL)
		world_normal_free((struct world_normal *) w);
	else if (gc.game_type == TYPE_TOROIDAL)
		world_toroidal_free((struct world_toroidal *) w);

	return EXIT_SUCCESS;
}
