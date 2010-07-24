/* 
 *
 *   Copyright (c) 2009-2010 Arthur Huillet
 *
 *
 *  This file is part of Freedroid
 *
 *  Freedroid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Freedroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Freedroid; see the file COPYING. If not, write to the 
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 *
 */

#undef EXTERN
#ifndef _leveleditor_widgets_c
#define EXTERN extern
#else
#define EXTERN
#endif

enum leveleditor_widget_type {
	WIDGET_BUTTON,
	WIDGET_TOOLBAR,
	WIDGET_MAP,
	WIDGET_CATEGORY_SELECTOR,
	WIDGET_MINIMAP,
};

/* A widget in the level editor */
struct leveleditor_widget {
	enum leveleditor_widget_type type;	//Type of widget
	SDL_Rect rect;		//Space occupied
	struct list_head node;
	int enabled;
	void (*mouseenter) (SDL_Event *, struct leveleditor_widget *);
	void (*mouseleave) (SDL_Event *, struct leveleditor_widget *);
	void (*mouserelease) (SDL_Event *, struct leveleditor_widget *);
	void (*mousepress) (SDL_Event *, struct leveleditor_widget *);
	void (*mouserightrelease) (SDL_Event *, struct leveleditor_widget *);
	void (*mouserightpress) (SDL_Event *, struct leveleditor_widget *);
	void (*mousewheelup) (SDL_Event *, struct leveleditor_widget *);
	void (*mousewheeldown) (SDL_Event *, struct leveleditor_widget *);
	void (*mousemove) (SDL_Event *, struct leveleditor_widget *);
	int (*keybevent) (SDL_Event *, struct leveleditor_widget *);
	void *ext;		//Type specific information
};

struct leveleditor_button {
	int btn_index;		//index in AllMousePressButtons array
	int pressed;
	int active;
	char *text;
	char *tooltip;
};

struct leveleditor_toolbar {
};

struct leveleditor_mapwidget {
};

struct leveleditor_categoryselect {
	unsigned int selected_tile_nb;
	unsigned int toolbar_first_block;
	char *title;
	enum leveleditor_object_type type;
	int *indices;
};

#define WIDGET_MINIMAP_WIDTH 210
#define WIDGET_MINIMAP_HEIGHT 140

struct leveleditor_minimap {
};

void leveleditor_init_widgets(void);
void leveleditor_display_widgets(void);
void leveleditor_update_button_states(void);
void leveleditor_select_type(enum leveleditor_object_type);

struct leveleditor_widget *get_active_widget(int, int);
EXTERN struct list_head leveleditor_widget_list;
EXTERN struct leveleditor_widget *previously_active_widget;

#undef EXTERN

#include "lvledit/lvledit_widget_button.h"
#include "lvledit/lvledit_widget_map.h"
#include "lvledit/lvledit_widget_toolbar.h"
#include "lvledit/lvledit_widget_categoryselect.h"
#include "lvledit/lvledit_widget_minimap.h"
