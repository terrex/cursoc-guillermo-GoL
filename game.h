#ifndef __GAME_H__
#define __GAME_H__

#include <stdio.h>
#include "world.h"

struct game_config {
	int rows;
	int cols;
	int density;
	int generations;
	int speed;
	char file_config[256];
	char output[256];
	FILE *output_fp;
};

void game_config_defaults(struct game_config *gc);

void game_parse_command_line_options(int argc, char *argv[], struct game_config *gc);

void game_log_start(struct game_config *gc);

void game_log_output(const struct game_config *gc, const struct world *w);

void game_log_stop(struct game_config *gc);

#endif /* __GAME_H__ */
