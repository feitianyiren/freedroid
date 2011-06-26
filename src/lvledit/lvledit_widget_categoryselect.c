/* 
 *
 *   Copyright (c) 2009 Arthur Huillet
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

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "widgets/widgets.h"

static struct widget_lvledit_categoryselect *currently_selected_category = NULL;

static void widget_lvledit_categoryselect_mouseenter(SDL_Event *event, struct widget *w)
{
	struct widget_lvledit_categoryselect *cs = w->ext;
	(void)cs;
}

static void widget_lvledit_categoryselect_mouseleave(SDL_Event *event, struct widget *w)
{
	struct widget_lvledit_categoryselect *cs = w->ext;
	(void)cs;
}

static void widget_lvledit_categoryselect_mouserelease(SDL_Event *event, struct widget *w)
{
	struct widget_lvledit_categoryselect *cs = w->ext;
	(void)cs;
}

static void widget_lvledit_categoryselect_mousepress(SDL_Event *event, struct widget *w)
{
	struct widget_lvledit_categoryselect *cs = w->ext;
	currently_selected_category = cs;
}

static void widget_lvledit_categoryselect_mouserightrelease(SDL_Event *event, struct widget *w)
{
	struct widget_lvledit_categoryselect *cs = w->ext;
	(void)cs;
}

static void widget_lvledit_categoryselect_mouserightpress(SDL_Event *event, struct widget *w)
{
	struct widget_lvledit_categoryselect *cs = w->ext;
	(void)cs;
}

static void widget_lvledit_categoryselect_mousewheelup(SDL_Event *event, struct widget *w)
{
	struct widget_lvledit_categoryselect *cs = w->ext;
	(void)cs;
}

static void widget_lvledit_categoryselect_mousewheeldown(SDL_Event *event, struct widget *w)
{
	struct widget_lvledit_categoryselect *cs = w->ext;
	(void)cs;
}

static void categoryselect_display(struct widget *w)
{
	struct widget_lvledit_categoryselect *cs = w->ext;
	SDL_Rect tr, hr;
	int tab_width = 80;

	draw_rectangle(&w->rect, 80, 100, 100, 150);

	BFont_Info *PreviousFont;
	PreviousFont = GetCurrentFont();
	SetCurrentFont(Messagevar_BFont);

	tr.y = w->rect.y;
	tr.w = 2;
	tr.h = w->rect.h;
	hr.y = w->rect.y;
	hr.w = tab_width - 2;
	hr.h = w->rect.h;

	hr.x = w->rect.x;

	if (cs == currently_selected_category)
		draw_rectangle(&hr, 100, 220, 220, 350);

	display_text(cs->title, hr.x + 2, hr.y, &hr);
	tr.x = hr.x + tab_width - 2;
	draw_rectangle(&tr, 0, 0, 0, 136);
	SetCurrentFont(PreviousFont);
}

struct widget_lvledit_categoryselect *get_current_object_type()
{
	return currently_selected_category;
}

void widget_lvledit_categoryselect_activate(struct widget_lvledit_categoryselect *e)
{
	currently_selected_category = e;
}

struct widget *widget_lvledit_categoryselector_create(int x, char *text, enum lvledit_object_type type, int *olist)
{
	struct widget *a = MyMalloc(sizeof(struct widget));
	a->type = WIDGET_CATEGORY_SELECTOR;
	widget_set_rect(a, x * 80, 73, 80, 17);
	a->display = categoryselect_display;
	a->mouseenter = widget_lvledit_categoryselect_mouseenter;
	a->mouseleave = widget_lvledit_categoryselect_mouseleave;
	a->mouserelease = widget_lvledit_categoryselect_mouserelease;
	a->mousepress = widget_lvledit_categoryselect_mousepress;
	a->mouserightrelease = widget_lvledit_categoryselect_mouserightrelease;
	a->mouserightpress = widget_lvledit_categoryselect_mouserightpress;
	a->mousewheelup = widget_lvledit_categoryselect_mousewheelup;
	a->mousewheeldown = widget_lvledit_categoryselect_mousewheeldown;
	a->enabled = 1;

	struct widget_lvledit_categoryselect *cs = MyMalloc(sizeof(struct widget_lvledit_categoryselect));
	cs->type = type;
	cs->indices = olist;
	cs->title = text;

	a->ext = cs;
	return a;
}
