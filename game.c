#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include "game.h"

#define DEFAULT_ROWS 16
#define DEFAULT_COLS 32
#define DEFAULT_DENSITY 22
#define DEFAULT_GENERATIONS 100
#define DEFAULT_SPEED 100000

void game_config_defaults(struct game_config *gc)
{
	gc->rows = DEFAULT_ROWS;
	gc->cols = DEFAULT_COLS;
	gc->density = DEFAULT_DENSITY;
	gc->generations = DEFAULT_GENERATIONS;
	gc->speed = DEFAULT_SPEED;
	strcpy(gc->file_config, "");
}

void game_parse_command_line_options(int argc, char *argv[], struct game_config *gc)
{
	int option_index = 0;
	int c;
	FILE *fconfig;
	int newargc = 1;
	char *newargv_p[30];
	char newargv_v[30][256];
	char buf[256];
	int oldoptind;

	strncpy(newargv_v[0], argv[0], 256);
	newargv_p[0] = newargv_v[0];
	newargv_p[1] = NULL;

	static struct option long_options[] = {
		{"rows", required_argument, 0, 'r'},
		{"cols", required_argument, 0, 'c'},
		{"density", required_argument, 0, 'd'},
		{"generations", required_argument, 0, 'g'},
		{"speed", required_argument, 0, 's'},
		{"file", required_argument, 0, 'f'},
		{0, 0, 0, 0},
	};
	while ((c = getopt_long(argc, argv, "r:c:d:g:s:f:", long_options, &option_index)) != -1) {
		switch (c) {
		case 'r':
			gc->rows = (int) strtol(optarg, NULL, 0);
			break;
		case 'c':
			gc->cols = (int) strtol(optarg, NULL, 0);
			break;
		case 'd':
			gc->density = (int) strtol(optarg, NULL, 0);
			break;
		case 'g':
			gc->generations = (int) strtol(optarg, NULL, 0);
			break;
		case 's':
			gc->speed = (int) strtol(optarg, NULL, 0);
			break;
		case 'f':
			strncpy(gc->file_config, optarg, 256);
			fconfig = fopen(optarg, "r");
			while (!feof(fconfig) && fscanf(fconfig, "%256s", buf)) {
				if (strlen(buf) > 0) {
					strncpy(newargv_v[newargc], buf, 256);
					newargv_p[newargc] = newargv_v[newargc];
					newargv_p[newargc + 1] = NULL;
					newargc++;
				}
			}
			fclose(fconfig);
			oldoptind = optind;
			optind = 0;
			game_parse_command_line_options(newargc, newargv_p, gc);
			optind = oldoptind;
			break;
		}
	}
}
