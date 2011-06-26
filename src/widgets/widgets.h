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
#ifndef _widgets_c
#define EXTERN extern
#else
#define EXTERN
#endif

#include "lvledit/lvledit.h"

enum widget_type {
	WIDGET_BUTTON,
	WIDGET_TOOLBAR,
	WIDGET_MAP,
	WIDGET_CATEGORY_SELECTOR,
	WIDGET_MINIMAP,
};

/* A widget in the level editor */
struct widget {
	enum widget_type type;	//Type of widget
	SDL_Rect rect;		//Space occupied
	uint8_t enabled;
	void (*display) (struct widget *);
	void (*mouseenter) (SDL_Event *, struct widget *);
	void (*mouseleave) (SDL_Event *, struct widget *);
	void (*mouserelease) (SDL_Event *, struct widget *);
	void (*mousepress) (SDL_Event *, struct widget *);
	void (*mouserightrelease) (SDL_Event *, struct widget *);
	void (*mouserightpress) (SDL_Event *, struct widget *);
	void (*mousewheelup) (SDL_Event *, struct widget *);
	void (*mousewheeldown) (SDL_Event *, struct widget *);
	void (*mousemove) (SDL_Event *, struct widget *);
	int (*keybevent) (SDL_Event *, struct widget *);
	void *ext;		//Type specific information
	struct list_head node;
};

void widget_lvledit_init(void);
void leveleditor_update_button_states(void);
void lvledit_select_type(enum lvledit_object_type);

void lvledit_categoryselect_switch(int direction);

void display_widgets();
struct widget *get_active_widget(int, int);
void widget_set_rect(struct widget *, int, int, int, int);
EXTERN struct list_head widget_list;
EXTERN struct list_head *lvledit_widget_list;
EXTERN struct widget *previously_active_widget;

#undef EXTERN

#define WIDGET(x) ((struct widget *)x)

#include "widgets/widget_group.h"
#include "widgets/widget_button.h"
#include "lvledit/lvledit_widget_map.h"
#include "lvledit/lvledit_widget_toolbar.h"
#include "lvledit/lvledit_widget_categoryselect.h"
#include "lvledit/lvledit_widget_minimap.h"
