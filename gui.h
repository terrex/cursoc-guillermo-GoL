#ifndef __GUI_H__
#define __GUI_H__

#include "world.h"
#include "game.h"

struct gui;

struct gui *gui_alloc(struct game_config *gc);
void gui_free(struct gui *g);

#endif /* __GUI_H__ */
