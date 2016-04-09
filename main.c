#include <stdio.h>
#include "world.h"

int main(void)
{
	struct world *w = world_random();



	w = world_free(w);

	return 0;
}
