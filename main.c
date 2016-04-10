#include <stdio.h>
#include <unistd.h>
#include "world.h"

#define RESET_SCREEN "\x1B[1;1H\x1B[2J"

int main(void)
{
	int it = 1;
	struct world *w = world_random();

	printf(RESET_SCREEN "Mundo %d:\n", it++);
	world_print(w);
	sleep(1);

	do {
		w = world_next_gen(w);
		printf(RESET_SCREEN "Mundo %d:\n", it++);
		world_print(w);
		sleep(1);
	} while (it < 15);

	w = world_free(w);

	return 0;
}
