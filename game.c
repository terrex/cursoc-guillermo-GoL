#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "world_toroidal.h"
#include "world_normal.h"
#include "game.h"

#define DEFAULT_ROWS 16
#define DEFAULT_COLS 32
#define DEFAULT_DENSITY 22
#define DEFAULT_GENERATIONS 100
#define DEFAULT_SPEED 0.1

void game_config_defaults(struct game_config *gc)
{
	gc->rows = DEFAULT_ROWS;
	gc->cols = DEFAULT_COLS;
	gc->density = DEFAULT_DENSITY;
	gc->generations = DEFAULT_GENERATIONS;
	gc->speed = DEFAULT_SPEED;
	gc->file_config[0] = '\0';
	gc->output[0] = '\0';
	gc->output_fp = NULL;
	gc->write_world[0] = '\0';
	gc->load_world[0] = '\0';
	gc->game_type = TYPE_NORMAL;
}

void game_parse_command_line_options(int argc, char *argv[], struct game_config *gc, int loops)
{
	int option_index = 0;
	int c;
	FILE *fconfig;
	int newargc;
	char *newargv_p[30];
	char newargv_v[30][256];
	char buf[256];
	int oldoptind;
	FILE *load_fp;

	if (++loops >= 20) {
		fprintf(stderr, "game_parse_command_line_options(): stop processing arguments, maximum loops reached.\n");
		return;
	}

	static struct option long_options[] = {
		{"rows", required_argument, 0, 'r'},
		{"cols", required_argument, 0, 'c'},
		{"density", required_argument, 0, 'd'},
		{"generations", required_argument, 0, 'g'},
		{"speed", required_argument, 0, 's'},
		{"file", required_argument, 0, 'f'},
		{"output", required_argument, 0, 'o'},
		{"write-world", required_argument, 0, 'w'},
		{"load-world", required_argument, 0, 'l'},
		{"normal", no_argument, 0, 0},
		{"toroidal", no_argument, 0, 0},
		{0, 0, 0, 0},
	};
	while ((c = getopt_long(argc, argv, "r:c:d:g:s:f:o:w:l:", long_options, &option_index)) != -1) {
		switch (c) {
		case 0:
			if (strcmp("normal", long_options[option_index].name) == 0)
				gc->game_type = TYPE_NORMAL;
			else if (strcmp("toroidal", long_options[option_index].name) == 0)
				gc->game_type = TYPE_TOROIDAL;
			break;
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
			gc->speed = strtof(optarg, NULL);
			break;
		case 'o':
			strncpy(gc->output, optarg, 256);
			break;
		case 'w':
			strncpy(gc->write_world, optarg, 256);
			break;
		case 'l':
			load_fp = fopen(optarg, "r");
			if (load_fp == NULL) {
				perror("failed to open file specified by -l option");
				break;
			}
			if (fread(gc, sizeof(struct game_config), 1, load_fp) != 1) {
				perror("error loading game_config");
				/* abort due garbage could have been read. */
				exit(EXIT_FAILURE);
			}
			fclose(load_fp);
			strncpy(gc->load_world, optarg, 256);
			break;
		case 'f':
			fconfig = fopen(optarg, "r");
			if (fconfig == NULL) {
				perror("failed to open file specified by -f option");
				break;
			}
			strncpy(gc->file_config, optarg, 256);

			/* reset here next-call args to allow several -f in a row */
			newargc = 1;
			strncpy(newargv_v[0], argv[0], 256);
			newargv_p[0] = newargv_v[0];
			newargv_p[1] = NULL;

			while (!feof(fconfig) && (fscanf(fconfig, "%256s", buf)) == 1) {
				if (strlen(buf) > 0) {
					strncpy(newargv_v[newargc], buf, 256);
					newargv_p[newargc] = newargv_v[newargc];
					newargv_p[newargc + 1] = NULL;
					newargc++;
				}
			}
			if (ferror(fconfig)) {
				perror("error reading fconfig (-f)");
				fclose(fconfig);
				break;
			}
			fclose(fconfig);
			oldoptind = optind;
			optind = 0;
			game_parse_command_line_options(newargc, newargv_p, gc, loops);
			optind = oldoptind;
			break;
		}
	}
}

void game_log_start(struct game_config *gc)
{
	if (gc->output[0] == '\0')
		return;

	gc->output_fp = fopen(gc->output, "w+");

	if (gc->output_fp == NULL)
		perror("failed to open file for writing stats");
}

void game_log_output(const struct game_config *gc, const struct world *w)
{
	if (gc->output_fp == NULL)
		return;

	fprintf(gc->output_fp, "%d\t%d\n", w->get_generation(w), w->get_alive_cells_count(w));
}

void game_log_stop(struct game_config *gc)
{
	if (gc->output_fp == NULL)
		return;

	fclose(gc->output_fp);
	gc->output_fp = NULL;
}

void game_write(struct game_config *gc, const struct world *w)
{
	FILE *write_fp;

	if (gc->write_world[0] == '\0')
		return;

	write_fp = fopen(gc->write_world, "w+");
	if (write_fp == NULL) {
		perror("failed to open file for dump game");
		return;
	}

	if (fwrite(gc, sizeof(struct game_config), 1, write_fp) != 1) {
		perror("failed to write game_config");
		exit(EXIT_FAILURE);
	}
	w->save(w, write_fp);
	fclose(write_fp);
}

void game_alloc_n_load(struct game_config *gc, struct world **w)
{
	FILE *load_fp;

	if (gc->load_world[0] == '\0')
		return;

	load_fp = fopen(gc->load_world, "r");
	if (load_fp == NULL) {
		perror("failed to open file for loading game from dump");
		if (gc->game_type == TYPE_NORMAL)
			*w = (struct world *)world_normal_alloc(gc->rows, gc->cols);
		else if (gc->game_type == TYPE_TOROIDAL)
			*w = (struct world *)world_toroidal_alloc(gc->rows, gc->cols);
		return;
	}

	/*
	 * load config in opt parsing time, not now,
	 * to allow overwrite of params in next run after -l
	 */
	fseek(load_fp, sizeof(struct game_config), SEEK_CUR);
	if (gc->game_type == TYPE_NORMAL)
		*w = (struct world *)world_normal_alloc(gc->rows, gc->cols);
	else if (gc->game_type == TYPE_TOROIDAL)
		*w = (struct world *)world_toroidal_alloc(gc->rows, gc->cols);
	(*w)->load(*w, load_fp);
	fclose(load_fp);
}
