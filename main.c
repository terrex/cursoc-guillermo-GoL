#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "world.h"

#define RESET_SCREEN "\x1B[1;1H\x1B[2J"
#define DEFAULT_ROWS 16
#define DEFAULT_COLS 32
#define DEFAULT_DENSITY 22

#define BEEP() do { printf("\a"); fflush(stdout); } while (0)

int main(void)
{
	/* ===== Ask for world size ===== */

	int rows = 0, cols = 0, density = 0;
	/*
	 * I use geline() instead of scanf() to accept default values using
	 * the Enter key. Because of that, I must use dynamically allocated
	 * buffer with the possibility to grow (according to getline spec).
	 */
	size_t buf_sz = 50;
	char *buf = calloc(buf_sz, sizeof(char));

	printf("Numero de filas? [%d]: ", DEFAULT_ROWS);
	fflush(stdout);
	getline(&buf, &buf_sz, stdin);
	rows = atoi(buf);
	if (rows <= 0)
		rows = DEFAULT_ROWS;

	printf("Numero de columnas? [%d]: ", DEFAULT_COLS);
	fflush(stdout);
	getline(&buf, &buf_sz, stdin);
	cols = atoi(buf);
	if (cols <= 0)
		cols = DEFAULT_COLS;

	printf("%% de densidad de poblacion viva inicial? [%d]: ", DEFAULT_DENSITY);
	fflush(stdout);
	getline(&buf, &buf_sz, stdin);
	density = atoi(buf);
	if (density <= 0)
		density = DEFAULT_DENSITY;

	free(buf);
	buf_sz = 0;
	buf = NULL;

	/* ===== Run program ===== */

	int it = 1;
	struct world *w = world_random_with_size(rows, cols, density);

	BEEP();
	printf(RESET_SCREEN "Mundo %d:\n", it++);
	world_print(w);
	sleep(1);

	do {
		w = world_next_gen(w);
		BEEP();
		printf(RESET_SCREEN "Mundo %d:\n", it++);
		world_print(w);
		sleep(1);
	} while (it < 15);

	w = world_free(w);

	return 0;
}
