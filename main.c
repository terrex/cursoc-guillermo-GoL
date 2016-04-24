#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include "world.h"

#define RESET_SCREEN "\x1B[1;1H\x1B[2J"
#define DEFAULT_ROWS 16
#define DEFAULT_COLS 32
#define DEFAULT_DENSITY 22
#define DEFAULT_GENERATIONS 100
#define DEFAULT_SPEED 100000

#define BEEP() do { printf("\a"); fflush(stdout); } while (0)

int main(int argc, char *argv[])
{
	int option_index = 0;
	int c;
	int rows = DEFAULT_ROWS, cols = DEFAULT_COLS,
			density = DEFAULT_DENSITY, generations = DEFAULT_GENERATIONS;
	int speed = DEFAULT_SPEED;
	static struct option long_options[] = {
			{"rows", required_argument, 0, 'r'},
			{"cols", required_argument, 0, 'c'},
			{"density", required_argument, 0, 'd'},
			{"generations", required_argument, 0, 'g'},
			{"speed", required_argument, 0, 's'},
			{0, 0, 0, 0}
	};
	while ((c = getopt_long(argc, argv, "r:c:d:g:s:", long_options, &option_index)) != -1) {
		switch (c) {
		case 'r':
			rows = (int) strtol(optarg, NULL, 0);
			break;
		case 'c':
			cols = (int) strtol(optarg, NULL, 0);
			break;
		case 'd':
			density = (int) strtol(optarg, NULL, 0);
			break;
		case 'g':
			generations = (int) strtol(optarg, NULL, 0);
			break;
		case 's':
			speed = (int) strtol(optarg, NULL, 0);
			break;
		}
	}

	/* ===== Run program ===== */

	int it = 1;
	struct world *w = world_random_with_size(rows, cols, density);
	struct world *w1 = world_alloc(rows, cols);
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
		usleep(speed);
	} while (it < generations);

	world_free(w);
	world_free(w1);

	return 0;
}
