#ifndef __GAME_H__
#define __GAME_H__

struct game_config {
	int rows;
	int cols;
	int density;
	int generations;
	int speed;
	char file_config[256];
};

void game_config_defaults(struct game_config *gc);

void game_parse_command_line_options(int argc, char *argv[], struct game_config *gc);

#endif /* __GAME_H__ */
