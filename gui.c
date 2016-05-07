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

struct gui {
	GtkBuilder *builder;

	GtkWidget *awMain;
	GtkWidget *btnSave;
	GtkWidget *btnPlayPause;
	GtkWidget *btnStep;
	GtkWidget *tglToroidal;
	GtkWidget *sclSpeed;
	GtkWidget *daMapContainer;
	GtkDrawingArea *daMap;
	GtkStatusbar *statusbar;
	GtkWidget *menuFileNew;

	bool run;
	int draw_scale;
	struct world *world;
	struct game_config *gc;
	bool resetting_size;
};

/* Callbacks */
static gboolean daMap_draw(GtkWidget *widget, cairo_t *cr, struct gui *g);
static gboolean timer_cb(gpointer gui);
static void btnStep_clicked(GtkWidget *widget, struct gui *g);
static void btnPlayPause_clicked(GtkWidget *widget, struct gui *g);
static gboolean mouse_btn_cb(GtkWidget *widget, GdkEventButton *e,
			 struct gui *g);
static void reset_world(struct gui *g, int newrows, int newcols);
static void refresh_status(struct gui *g);
static void tglToroidal_toggled(GtkWidget *widget, struct gui *g);
static void sclSpeed_value_changed(GtkWidget *widget, struct gui *g);
static void menuFileNew_activate(GtkWidget *widget, struct gui *g);


struct gui *gui_alloc(struct game_config *gc)
{
	struct gui *g;

	/* struct gui */
	g = (struct gui *)malloc(sizeof(struct gui));
	if (!g)
		return NULL;

	/* builder */
	g->builder = gtk_builder_new();
	if (!gtk_builder_add_from_file(g->builder, "builder.ui", NULL))
		return NULL;

	/* GObjects */
	g->awMain = GTK_WIDGET(gtk_builder_get_object(g->builder, "awMain"));
	g->btnSave = GTK_WIDGET(gtk_builder_get_object(g->builder, "btnSave"));
	g->btnPlayPause = GTK_WIDGET(gtk_builder_get_object(g->builder, "btnPlayPause"));
	g->btnStep = GTK_WIDGET(gtk_builder_get_object(g->builder, "btnStep"));
	g->tglToroidal = GTK_WIDGET(gtk_builder_get_object(g->builder, "tglToroidal"));
	g->daMapContainer = GTK_WIDGET(gtk_builder_get_object(g->builder, "daMapContainer"));
	g->sclSpeed = GTK_WIDGET(gtk_builder_get_object(g->builder, "sclSpeed"));
	g->daMap = GTK_DRAWING_AREA(gtk_builder_get_object(g->builder, "daMap"));
	g->statusbar = GTK_STATUSBAR(gtk_builder_get_object(g->builder, "statusbar"));
	g->menuFileNew = GTK_WIDGET(gtk_builder_get_object(g->builder, "menuFileNew"));
	if (!(g->awMain && g->btnPlayPause && g->btnStep && g->tglToroidal && g->daMap)) {
		/* Establecemos errno para que perror imprima el mensaje correcto */
		errno = EINVAL;
		return NULL;
	}

	g->gc = gc;
	g->run = false;
	g->draw_scale = 4;
	g->world = NULL;
	g->resetting_size = true;
	reset_world(g, g->gc->rows, g->gc->cols);

	g_signal_connect(g->awMain, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_widget_set_size_request(GTK_WIDGET(g->daMap), g->gc->cols * g->draw_scale, g->gc->rows * g->draw_scale);
	g_signal_connect(g->daMap, "draw", G_CALLBACK(daMap_draw), g);
	g_signal_connect(g->btnStep, "clicked", G_CALLBACK(btnStep_clicked), g);
	g_signal_connect(g->btnPlayPause, "clicked", G_CALLBACK(btnPlayPause_clicked), g);
	g_signal_connect(g->tglToroidal, "toggled", G_CALLBACK(tglToroidal_toggled), g);
	g_signal_connect(g->sclSpeed, "value-changed", G_CALLBACK(sclSpeed_value_changed), g);
	g_signal_connect(g->menuFileNew, "activate", G_CALLBACK(menuFileNew_activate), g);


	gtk_builder_connect_signals(g->builder, NULL);

	gtk_widget_show_all(g->awMain);

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

	g_snprintf(text, 512, "%dx%d. Gen: %4d. Alive: %5d (%5.2f%%).", g->gc->cols, g->gc->rows,
			   g->world->get_generation(g->world), g->world->get_alive_cells_count(g->world),
			   (float) g->world->get_alive_cells_count(g->world) / (g->gc->cols * g->gc->rows) * 100);
	gtk_statusbar_push(g->statusbar, gtk_statusbar_get_context_id(g->statusbar, "status"), text);
}

static gboolean daMap_draw(GtkWidget *widget, cairo_t *cr, struct gui *g)
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
	if (g->resetting_size) {
		g->resetting_size = false;
		/* gtk_widget_set_size_request(g->daMapContainer, g->gc->cols * g->draw_scale, g->gc->rows * g->draw_scale); */
		/* gtk_widget_set_size_request(widget, g->gc->cols * g->draw_scale, g->gc->rows * g->draw_scale); */
		gtk_window_resize(g->awMain, 30, 30);
	} else {
		if (newrows != g->gc->rows || newcols != g->gc->cols)
			reset_world(g, newrows, newcols);
		gtk_widget_set_size_request(widget, g->gc->cols * g->draw_scale, g->gc->rows * g->draw_scale);
	}

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

	btnStep_clicked(g->btnStep, g);

	if (g->run)
		g_timeout_add((guint) (g->gc->speed * 1000), timer_cb, g);

	return false;
}

static void btnStep_clicked(GtkWidget *widget, struct gui *g)
{
	g->world->next_gen(g->world);
	gtk_widget_queue_draw(GTK_WIDGET(g->daMap));
	refresh_status(g);
}

static void btnPlayPause_clicked(GtkWidget *widget, struct gui *g)
{
	g->run = !g->run;

	if (g->run) {
		g_timeout_add((guint) (g->gc->speed * 1000), timer_cb, g);
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(g->btnPlayPause), "media-playback-pause");
	} else {
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(g->btnPlayPause), "media-playback-start");
	}
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

static void tglToroidal_toggled(GtkWidget *widget, struct gui *g)
{
	if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget)))
		g->gc->game_type = TYPE_TOROIDAL;
	else
		g->gc->game_type = TYPE_NORMAL;

	reset_world(g, g->gc->rows, g->gc->cols);
	gtk_widget_set_size_request(GTK_WIDGET(g->daMap), g->gc->cols * g->draw_scale, g->gc->rows * g->draw_scale);
}

static void sclSpeed_value_changed(GtkWidget *widget, struct gui *g)
{
	g->gc->speed = (float) gtk_range_get_value(GTK_RANGE(widget));
}

static void menuFileNew_activate(GtkWidget *widget, struct gui *g)
{
	/* reset UI */
	g->run = false;
	g->gc->game_type = TYPE_TOROIDAL;
	g->gc->speed = 0.10;
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(g->btnPlayPause), "media-playback-start");
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(g->tglToroidal), true);
	gtk_range_set_value(GTK_RANGE(g->sclSpeed), 0.10);

	/* reset world and Map */
	g->resetting_size = true;
	reset_world(g, 40, 80);
	gtk_widget_set_size_request(GTK_WIDGET(g->daMap), g->gc->cols * g->draw_scale, g->gc->rows * g->draw_scale);
}
