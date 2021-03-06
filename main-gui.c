#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "game.h"
#include "gui.h"

int main(int argc, char **argv)
{
	struct game_config gc;
	struct gui *g;

	gtk_init(&argc, &argv);

	game_config_defaults(&gc);
	gc.rows = 40;
	gc.cols = 80;
	gc.game_type = TYPE_TOROIDAL;
	gc.speed = 0.10;

	g = gui_alloc(&gc);
	if (!g) {
		perror("Can't create gui");
		exit(EXIT_FAILURE);
	}

	gtk_main();

	gui_free(g);

	return 0;
}
