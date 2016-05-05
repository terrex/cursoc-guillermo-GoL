#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <gtk/gtk.h>
#include "world.h"
#include "game.h"
#include "gui.h"
#include "world_normal.h"
#include "world_toroidal.h"

#define VEL_MS 10

struct gui {
	GtkBuilder *builder;

	GtkWidget *awMain;
	GtkWidget *btnOpen;
	GtkWidget *btnSave;
	GtkWidget *btnPlay;
	GtkWidget *btnPause;
	GtkWidget *btnStep;
	GtkWidget *tglToroidal;
	GtkWidget *daMapContainer;
	GtkDrawingArea *daMap;
	GtkStatusbar *statusbar;

	bool run;
	int draw_scale;
	struct world *world;
	struct game_config *gc;
};

/* Callbacks */
static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, struct gui *g);
static gboolean timer_cb(gpointer gui);
static void step_cb(GtkWidget *widget, struct gui *g);
static void pause_cb(GtkWidget *widget, struct gui *g);
static gboolean mouse_btn_cb(GtkWidget *widget, GdkEventButton *e,
			 struct gui *g);
static void reset_world(struct gui *g, int newrows, int newcols);
static void refresh_status(struct gui *g);


struct gui *gui_alloc(struct game_config *gc)
{
	struct gui *g;

	/* struct gui */
	g = (struct gui *)malloc(sizeof(struct gui));
	if (!g)
		return NULL;

	g->gc = gc;
	g->run = false;
	g->draw_scale = 4;
	g->world = NULL;
	reset_world(g, g->gc->rows, g->gc->cols);

	/* builder */
	g->builder = gtk_builder_new();
	if (!gtk_builder_add_from_file(g->builder, "builder.ui", NULL))
		return NULL;

	/* GObjects */
	g->awMain = GTK_WIDGET(gtk_builder_get_object(g->builder, "awMain"));
	g->btnOpen = GTK_WIDGET(gtk_builder_get_object(g->builder, "btnOpen"));
	g->btnSave = GTK_WIDGET(gtk_builder_get_object(g->builder, "btnSave"));
	g->btnPlay = GTK_WIDGET(gtk_builder_get_object(g->builder, "btnPlay"));
	g->btnPause = GTK_WIDGET(gtk_builder_get_object(g->builder, "btnPause"));
	g->btnStep = GTK_WIDGET(gtk_builder_get_object(g->builder, "btnStep"));
	g->tglToroidal = GTK_WIDGET(gtk_builder_get_object(g->builder, "tglToroidal"));
	g->daMapContainer = GTK_WIDGET(gtk_builder_get_object(g->builder, "daMapContainer"));
	g->daMap = GTK_DRAWING_AREA(gtk_builder_get_object(g->builder, "daMap"));
	g->statusbar = GTK_STATUSBAR(gtk_builder_get_object(g->builder, "statusbar"));
	if (!(g->awMain && g->btnOpen && g->btnSave && g->btnPlay &&
	g->btnPause && g->btnStep && g->tglToroidal && g->daMap)) {
		/* Establecemos errno para que perror imprima el mensaje correcto */
		errno = EINVAL;
		return NULL;
	}

	g_signal_connect(g->awMain, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_widget_set_size_request(GTK_WIDGET(g->daMap), g->gc->cols * g->draw_scale, g->gc->rows * g->draw_scale);
	g_signal_connect(g->daMap, "draw", G_CALLBACK(draw_cb), g);

	gtk_builder_connect_signals(g->builder, NULL);

	gtk_widget_show_all(g->awMain);

	guint context_id = gtk_statusbar_get_context_id(g->statusbar, "status");

	gtk_statusbar_push(g->statusbar, context_id, "Mi texto");

	return g;
}

void gui_free(struct gui *g)
{
	free(g);
}

static void reset_world(struct gui *g, int newrows, int newcols)
{
	world_free(g->world);
	g->gc->rows = newrows;
	g->gc->cols = newcols;
	if (g->gc->game_type == TYPE_NORMAL) {
		g->world = (struct world *) world_normal_alloc(newrows, newcols);
	} else {
		g->world = (struct world *) world_toroidal_alloc(newrows, newcols);
	}
	g->world->init_with_density(g->world, g->gc->density);

	refresh_status(g);
}

static void refresh_status(struct gui *g)
{
	gchar text[512];

	g_snprintf(text, 512, "World size %dx%d. Generation: %d.", g->gc->cols, g->gc->rows,
			   g->world->get_generation(g->world));
	gtk_statusbar_push(g->statusbar, gtk_statusbar_get_context_id(g->statusbar, "status"), text);
}

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, struct gui *g)
{
	static bool painting;

	if (painting)
		return false;

	painting = true;

	/* available space */
	gint w, h;
	GtkAllocation alloc;

	gtk_widget_get_allocation(g->daMapContainer, &alloc);
	w = alloc.width;
	h = alloc.height;
	/* resize map */
	gint newrows, newcols;

	newrows = h / g->draw_scale;
	newcols = w / g->draw_scale;
	if (newrows != g->gc->rows || newcols != g->gc->cols)
		reset_world(g, newrows, newcols);
	gtk_widget_set_size_request(widget, newcols * g->draw_scale, newrows * g->draw_scale);

	/* Clear screen */
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);

	cairo_set_source_rgb(cr, 1, 1, 1);
	for (int i = 0; i < g->gc->rows; i++)
		for (int j = 0; j < g->gc->cols; j++)
			if (g->world->get_cell(g->world, i, j) == ALIVE)
				cairo_rectangle(cr, j * g->draw_scale, i * g->draw_scale, g->draw_scale, g->draw_scale);

	cairo_fill(cr);

	painting = false;

	return false;
}

static gboolean timer_cb(gpointer gui)
{
	struct gui *g = (struct gui *)gui;

	step_cb(NULL, g);
	/* Si devuelve false, la senyal se desconecta y el callback no se vuelve
	 * a llamar
	 */
	return g->run;
}

static void step_cb(GtkWidget *widget, struct gui *g)
{
}

static void pause_cb(GtkWidget *widget, struct gui *g)
{
	/* TODO
	 * Cambiar el label del boton de "run" a "pause" para que indique en
	 * cada momento su funcionalidad
	 */
	if (!g->run)
		g_timeout_add(VEL_MS, timer_cb, g); /* Conectamos la senyal de nuevo */
	g->run = !g->run;
}

static gboolean mouse_btn_cb(GtkWidget *widget, GdkEventButton *e,
			 struct gui *g)
{
	/* TODO
	 * e->x y e->y contienen las coordenadas de donde se ha hecho click con
	 * el ratón. Pista: las coordenadas del mundo no son las mismas que las
	 * de la ventana.
	 */
}
