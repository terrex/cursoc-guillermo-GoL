#include <stdio.h>
#include "world.h"

int main(void)
{
	struct world *w = world_random();

	printf("Mundo %d:\n", 1);
	world_print(w);


	w = world_free(w);

	return 0;
}
