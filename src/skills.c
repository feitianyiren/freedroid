/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
 *   Copyright (c) 2004-2010 Arthur Huillet 
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

/**
 * This file contains all the functions managing the character skills,
 * which means all the special functions and also the spells of the
 * players character.
 */

#define _skills_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#define CLASS_X 175

#define EXPERIENCE_Y 150
#define NEXT_LEVEL_Y 220

#define GOLD_Y 132

#define BUTTON_MOD_X (-6)
#define BUTTON_MOD_Y (-4)
#define BUTTON_WIDTH 35
#define BUTTON_HEIGHT 19

#define DAMAGE_X 260
#define DAMAGE_Y 225

#define RECHARGE_X 260
#define RECHARGE_Y 200

#define DAMRED_X 260
#define DAMRED_Y 171

#define INV_BUTTON_X 600
#define INV_BUTTON_Y 400
#define CHA_BUTTON_X 600
#define CHA_BUTTON_Y 430
#define INV_BUTTON_WIDTH 38
#define INV_BUTTON_HEIGHT 22

#define SPELL_LEVEL_BUTTONS_X 60
#define SPELL_LEVEL_BUTTONS_Y 423
#define SPELL_LEVEL_BUTTON_WIDTH 30

int Override_Power_Limit = 0;

/**
 * This function improves a generic skill (hack melee ranged magic) by one
 * 
 */
void ImproveSkill(int *skill)
{
	if (*skill >= NUMBER_OF_SKILL_LEVELS - 1)
		return;
	(*skill)++;
};				// void ImproveSkill ( int * skill )

/**
 * This function improves a program by one
 * returns 1 if it can't be improved any further, returns 0 otherwise
 *
 */
int improve_program(int prog_id)
{
	if(prog_id < 0)
		return 0;

	if(Me.base_skill_level[prog_id] >= NUMBER_OF_SKILL_LEVELS - 1)
		return 1;

	Me.base_skill_level[prog_id]++;
	return 0;
}

/* ------------------
 * This function calculates the heat cost of running a given program (source or blob), based on current program level and casting ability
 * -----------------*/
int calculate_program_heat_cost(int program_id)
{
//	                                           0.9^0, 0.9^1, 0.9^2 ... ...0.9^9
	float cost_ratio[NUMBER_OF_SKILL_LEVELS] = { 1.0, 0.9, 0.81, 0.73, 0.66, 0.59, 0.53, 0.48, 0.43, 0.39 };

	if (program_id == get_program_index_with_name("Emergency shutdown") ) { //then use cost_ratio^-1
		return (1/cost_ratio[Me.spellcasting_skill]) * (SpellSkillMap[program_id].heat_cost +
							    	SpellSkillMap[program_id].heat_cost_per_level * (Me.SkillLevel[program_id] - 1));
	} else {
		return cost_ratio[Me.spellcasting_skill] * (SpellSkillMap[program_id].heat_cost +
							    SpellSkillMap[program_id].heat_cost_per_level * (Me.SkillLevel[program_id] - 1));
	}
};

/* ------------------
 * This function calculates the damage dealt by a hit of a given program
 * -----------------*/
static int calculate_program_hit_damage(int program_id)
{
	return (SpellSkillMap[program_id].damage_base + SpellSkillMap[program_id].damage_per_level * (Me.SkillLevel[program_id] - 1) +
		MyRandom(SpellSkillMap[program_id].damage_mod));
}

/* ------------------
 * This function calculates the lowest damage a program can deal
 *-----------------*/
static int calculate_program_hit_damage_low(int program_id)
{
	return (SpellSkillMap[program_id].damage_base + SpellSkillMap[program_id].damage_per_level * (Me.SkillLevel[program_id] - 1));

}

static int calculate_program_hit_damage_high(int program_id)
{
	return (SpellSkillMap[program_id].damage_base + SpellSkillMap[program_id].damage_per_level * (Me.SkillLevel[program_id] - 1) +
		SpellSkillMap[program_id].damage_mod);
}

/* ------------------
 * This function calculates the duration of the special effect of a
 * given program
 * ------------------*/
static float calculate_program_effect_duration(int program_id)
{
	return (SpellSkillMap[program_id].effect_duration +
		SpellSkillMap[program_id].effect_duration_per_level * (Me.SkillLevel[program_id] - 1));
}

/**
 * This function calculates the busy time (recovery time) for tux after running a program
 */
static float calculate_program_busy_time(void)
{
	float busy_time[NUMBER_OF_SKILL_LEVELS] = { 1.0, 0.9, 0.81, 0.73, 0.66, 0.59, 0.53, 0.48, 0.43, 0.39 };
	return busy_time[Me.spellcasting_skill];
}

/* ------------------
 * This function looks for a given program name in the program spec array
 * -------------------*/
int get_program_index_with_name(const char *pname)
{
	int i = 0;
	while (i < number_of_skills) {
		if (!strcmp(SpellSkillMap[i].name, pname))
			return i;
		i++;
	}

	fprintf(stderr, "%s\n", pname);

	ErrorMessage(__FUNCTION__, "\
FreedroidRPG could not find the program name above in the program spec array!", PLEASE_INFORM, IS_FATAL);
	return -1;
}

/**
 * This function creates a teleporter portal to the home location.
 */
static void TeleportHome(void)
{
	location HomeSpot;

	// Find homespot position.

	ResolveMapLabelOnShip("TeleportHomeTarget", &(HomeSpot));

	// If homespot was not found, a fatal error has been generated, but we
	// however check it once again, to prevent any bug.

	if (HomeSpot.x == -1)
		return;

	//--------------------
	// Case 1 : Tux is in homespot's level, and there is a teleport anchor
	//          -> teleport back to previous position
	//
	if (Me.pos.z == HomeSpot.level && ((Me.teleport_anchor.x != 0) || (Me.teleport_anchor.y != 0))) {

		// Teleport
		teleport_arrival_sound();
		reset_visible_levels();
		Teleport(Me.teleport_anchor.z, Me.teleport_anchor.x, Me.teleport_anchor.y, TRUE);
		clear_active_bullets();

		// Reset teleport anchor
		Me.teleport_anchor.x = 0;
		Me.teleport_anchor.y = 0;
		Me.teleport_anchor.z = 0;

		return;
	}

	//--------------------
	// Any other cases : Store current position and teleport Tux to homespot
	//
	Me.teleport_anchor.x = Me.pos.x;
	Me.teleport_anchor.y = Me.pos.y;
	Me.teleport_anchor.z = Me.pos.z;

	teleport_arrival_sound();
	reset_visible_levels();
	Teleport(HomeSpot.level, HomeSpot.x, HomeSpot.y, TRUE);
	clear_active_bullets();

}				// void TeleportHome ( void )

/**
 * This function handles the program the player has just activated.
 * It checks temperature (does not increase it), and makes sure a 
 * busy_time is set.
 */
void HandleCurrentlyActivatedSkill()
{
	if (!MouseRightClicked() || Me.busy_time > 0)
		return;

	int Grabbed_InvPos = GetInventoryItemAt(GetInventorySquare_x(GetMousePos_x()), GetInventorySquare_y(GetMousePos_y()));
	if (Grabbed_InvPos != -1)
		if (ItemMap[Me.Inventory[Grabbed_InvPos].type].item_can_be_applied_in_combat)
			return;	//if the cursor is over an item that can be applied, then the player wants to apply it not trigger a spell

	if (Me.SkillLevel[Me.readied_skill] <= 0)
		return;

	/* We calculate the spellcost and check the power limit override - the temperature is raised further down, when the actual effect
	   gets triggered */
	int SpellCost = calculate_program_heat_cost(Me.readied_skill);

	if (Me.temperature > Me.max_temperature - SpellCost && !Override_Power_Limit) {
		//Not_Enough_Mana_Sound(  );
		Override_Power_Limit = 1;
		return;
	}
	Override_Power_Limit = 0;

	switch (SpellSkillMap[Me.readied_skill].form) {
	case PROGRAM_FORM_IMMEDIATE:
	case PROGRAM_FORM_BULLET:
	case PROGRAM_FORM_RADIAL:
		if (!MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y()))
			return;;
		break;
	}

	DoSkill(Me.readied_skill, SpellCost);

	/* Certain special actions implemented through DoSkill may set their own
	 * busy time, such as weapon reload. In that case, do not touch busy_time.
	 * Otherwise, mark that we are running a program.
	 */

	if (Me.busy_time == 0) {
		Me.busy_time = calculate_program_busy_time();
		Me.busy_type = RUNNING_PROGRAM;
	}

	return;
};				// void HandleCurrentlyActivatedSkill( void )

int DoSkill(int skill_index, int SpellCost)
{
	enemy *droid_below_mouse_cursor = NULL;

	float hitdmg = calculate_program_hit_damage(skill_index);
	float effdur = calculate_program_effect_duration(skill_index);

	/*we handle the form of the program now */
	switch (SpellSkillMap[skill_index].form) {
	case PROGRAM_FORM_IMMEDIATE:
		droid_below_mouse_cursor = GetLivingDroidBelowMouseCursor();
		if (droid_below_mouse_cursor == NULL)
			goto done_handling_instant_hits;
		if (!DirectLineColldet(Me.pos.x,
				       Me.pos.y,
				       translate_pixel_to_map_location((float)input_axis.x,
								       (float)input_axis.y,
								       TRUE),
				       translate_pixel_to_map_location((float)input_axis.x,
								       (float)input_axis.y, FALSE), Me.pos.z, &FlyablePassFilter))
			goto done_handling_instant_hits;

		if ((Druidmap[droid_below_mouse_cursor->type].is_human && !SpellSkillMap[skill_index].hurt_humans)
		    || (!Druidmap[droid_below_mouse_cursor->type].is_human && !SpellSkillMap[skill_index].hurt_bots))
			return 1;

		if (hitdmg > 0)
			hit_enemy(droid_below_mouse_cursor, hitdmg, 1, -1, 1);

		if (!strcmp(SpellSkillMap[skill_index].effect, "paralyze"))
			droid_below_mouse_cursor->paralysation_duration_left += effdur;
		if (!strcmp(SpellSkillMap[skill_index].effect, "slowdown"))
			droid_below_mouse_cursor->frozen += effdur;
		if (!strcmp(SpellSkillMap[skill_index].effect, "poison")) {
			droid_below_mouse_cursor->poison_duration_left += effdur;
			droid_below_mouse_cursor->poison_damage_per_sec += hitdmg;
		}

		Me.temperature += SpellCost;
		break;

	case PROGRAM_FORM_SELF:
		Me.energy -= hitdmg;
		Me.temperature += SpellCost;
		Me.slowdown_duration += strcmp(SpellSkillMap[skill_index].effect, "slowdown") ? 0 : effdur;
		Me.paralyze_duration += strcmp(SpellSkillMap[skill_index].effect, "paralyze") ? 0 : effdur;
		Me.invisible_duration += strcmp(SpellSkillMap[skill_index].effect, "invisibility") ? 0 : effdur;
		break;

	case PROGRAM_FORM_BULLET:
		Me.temperature += SpellCost;

		moderately_finepoint target_location;
		target_location.x = translate_pixel_to_map_location(input_axis.x, input_axis.y, TRUE);
		target_location.y = translate_pixel_to_map_location(input_axis.x, input_axis.y, FALSE);

		bullet bul_parms;
		/*XXX hardcoded laser pistol type */
		if (SpellSkillMap[skill_index].graphics_code != -1)
			FillInDefaultBulletStruct(&bul_parms, SpellSkillMap[skill_index].graphics_code, GetItemIndexByName("Laser pistol"));
		else
			FillInDefaultBulletStruct(&bul_parms, MAGENTA_BULLET, GetItemIndexByName("Laser pistol"));

		bul_parms.freezing_level = strcmp(SpellSkillMap[skill_index].effect, "slowdown") ? 0 : effdur;
		bul_parms.poison_duration = strcmp(SpellSkillMap[skill_index].effect, "poison") ? 0 : effdur;
		bul_parms.poison_damage_per_sec = strcmp(SpellSkillMap[skill_index].effect, "poison") ? 0 : hitdmg;
		bul_parms.paralysation_duration = strcmp(SpellSkillMap[skill_index].effect, "paralyze") ? 0 : effdur;
		bul_parms.damage = hitdmg;
		bul_parms.to_hit = SpellHitPercentageTable[Me.spellcasting_skill];
		if (SpellSkillMap[skill_index].hurt_humans && SpellSkillMap[skill_index].hurt_bots)
			bul_parms.hit_type = ATTACK_HIT_ALL;
		else if (SpellSkillMap[skill_index].hurt_humans)
			bul_parms.hit_type = ATTACK_HIT_HUMANS;
		else
			bul_parms.hit_type = ATTACK_HIT_BOTS;

		FireTuxRangedWeaponRaw(GetItemIndexByName("Laser pistol"), -1, &bul_parms, target_location);

		return 1;	//no extra effects

	case PROGRAM_FORM_RADIAL:
		Me.temperature += SpellCost;

		int i, j;
		for (i = 0; i < MAX_ACTIVE_SPELLS; i++) {
			if (AllActiveSpells[i].img_type == (-1))
				break;
		}
		if (i >= MAX_ACTIVE_SPELLS)
			i = 0;

		AllActiveSpells[i].img_type =
		    (SpellSkillMap[skill_index].graphics_code == -1 ? 2 : SpellSkillMap[skill_index].graphics_code);
		AllActiveSpells[i].spell_center.x = Me.pos.x;
		AllActiveSpells[i].spell_center.y = Me.pos.y;
		AllActiveSpells[i].spell_radius = 0.3;
		AllActiveSpells[i].spell_age = 0;
		AllActiveSpells[i].mine = 1;
		if (SpellSkillMap[skill_index].hurt_humans && SpellSkillMap[skill_index].hurt_bots)
			AllActiveSpells[i].hit_type = ATTACK_HIT_ALL;
		else if (SpellSkillMap[skill_index].hurt_humans)
			AllActiveSpells[i].hit_type = ATTACK_HIT_HUMANS;
		else
			AllActiveSpells[i].hit_type = ATTACK_HIT_BOTS;

		for (j = 0; j < RADIAL_SPELL_DIRECTIONS; j++) {
			AllActiveSpells[i].active_directions[j] = TRUE;
		}

		AllActiveSpells[i].freeze_duration = strcmp(SpellSkillMap[skill_index].effect, "slowdown") ? 0 : effdur;
		AllActiveSpells[i].poison_duration = strcmp(SpellSkillMap[skill_index].effect, "poison") ? 0 : effdur;
		AllActiveSpells[i].poison_dmg = strcmp(SpellSkillMap[skill_index].effect, "poison") ? 0 : hitdmg;
		AllActiveSpells[i].paralyze_duration = strcmp(SpellSkillMap[skill_index].effect, "paralyze") ? 0 : effdur;
		AllActiveSpells[i].damage = hitdmg;

		return 1;

	}

 done_handling_instant_hits:

	/*handle the special extra effects of the skill */
	if (!strcmp(SpellSkillMap[skill_index].effect, "none")) {
		goto out;
	}

	if (!strcmp(SpellSkillMap[skill_index].effect, "takeover")) {
		if (!MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y()))
			goto out;
		droid_below_mouse_cursor = GetLivingDroidBelowMouseCursor();
		if (droid_below_mouse_cursor == NULL)
			goto out;
		if (!DirectLineColldet(Me.pos.x,
				       Me.pos.y,
				       translate_pixel_to_map_location((float)input_axis.x,
								       (float)input_axis.y, TRUE),
				       translate_pixel_to_map_location((float)input_axis.x,
								       (float)input_axis.y, FALSE), Me.pos.z, &FlyablePassFilter))
			goto out;

		if (!droid_below_mouse_cursor->is_friendly) {
			//--------------------
			// Only droids can be hacked.  Humans can't be 
			// hacked.
			//
			if (droid_takeover(droid_below_mouse_cursor)) {
				// upon successful takeover
				// restore original heat
				// Me . temperature -= SpellCost;
				// go directly to chat to choose droid program
				if (GameConfig.talk_to_bots_after_takeover)
					ChatWithFriendlyDroid(droid_below_mouse_cursor);
			}
		} else {
			Me.temperature -= SpellCost;	//pretend nothing happened
		}
		goto out;
	}

	if (!strcmp(SpellSkillMap[skill_index].effect, "weapon")) {
		if (!MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y()))
			goto out;
		if (MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y()))
			tux_wants_to_attack_now(TRUE);
		goto out;
	}

	if (!strcmp(SpellSkillMap[skill_index].effect, "repair")) {
		if (!MouseCursorIsInInvRect(GetMousePos_x(), GetMousePos_y())
		    || (!GameConfig.Inventory_Visible)) {
			//--------------------
			// Do nothing here.  The right mouse click while in inventory screen
			// will be handled in the inventory screen management function.
			//
			PlayOnceNeededSoundSample("effects/tux_ingame_comments/CantRepairThat.ogg", FALSE, FALSE);
		}
		goto out;
	}

	if (!strcmp(SpellSkillMap[skill_index].effect, "identify")) {
		//--------------------
		// Maybe the identify mode has already been triggered and
		// is activated right now.  Then of course this (second) mouse
		// click must be ignored completely.
		//
		if (global_ingame_mode == GLOBAL_INGAME_MODE_IDENTIFY)
			Me.temperature -= SpellCost;

		silently_unhold_all_items();
		global_ingame_mode = GLOBAL_INGAME_MODE_IDENTIFY;

		goto out;
	}

	if (!strcmp(SpellSkillMap[skill_index].effect, "invisibility")) {
		goto out;
	}

	if (!strcmp(SpellSkillMap[skill_index].effect, "burnup")) {
		goto out;
	}

	if (!strcmp(SpellSkillMap[skill_index].effect, "teleport_home")) {
		TeleportHome();
		goto out;
	}

 out:

	return 1;
};				// void HandleCurrentlyActivatedSkill( void )

/**
 * This function checks if a given screen position lies within the 
 * one of the skill icons and returns the number of that skill icon.
 */
int CursorIsOnWhichSkillButton(int x, int y)
{
	//--------------------
	// First we check if the cursor is in at least horizontally
	// in the row of the skill items
	//
	if (x > SkillScreenRect.x + 16 + 64)
		return (-1);
	if (x < SkillScreenRect.x + 16)
		return (-1);

	//--------------------
	// Now we can check on which skill rectangle exactly the cursor
	// is hovering, since we know that it is hitting, horizontally
	// at least, the row of skill icons.
	//
	if (y < SkillScreenRect.y + 16 + 0 * 64)
		return (-1);
	if (y < SkillScreenRect.y + 16 + 1 * 64)
		return (0);

	if (y < SkillScreenRect.y + 16 + 1 * 64 + 16)
		return (-1);
	if (y < SkillScreenRect.y + 16 + 2 * 64 + 16)
		return (1);

	if (y < SkillScreenRect.y + 16 + 2 * 64 + 2 * 16)
		return (-1);
	if (y < SkillScreenRect.y + 16 + 3 * 64 + 2 * 16)
		return (2);

	if (y < SkillScreenRect.y + 16 + 3 * 64 + 3 * 16)
		return (-1);
	if (y < SkillScreenRect.y + 16 + 4 * 64 + 3 * 16)
		return (3);

	if (y < SkillScreenRect.y + 16 + 4 * 64 + 4 * 16)
		return (-1);
	if (y < SkillScreenRect.y + 16 + 5 * 64 + 4 * 16)
		return (4);

	return (-1);
};				// int CursorIsOnWhichSkillButton( int x , int y )

/**
 * This function checks if a given screen position lies within 
 * one of the spell level buttons and returns the number of that 
 * spell level button.
 */
static int CursorIsOnWhichSpellPageButton(int x, int y)
{
	int i;

	//--------------------
	// First we check if the cursor is in at least horizontally
	// and vertically in the line with the spell level buttons.
	//
	if (x > SkillScreenRect.x + SPELL_LEVEL_BUTTONS_X + 300)
		return (-1);
	if (x < SkillScreenRect.x + SPELL_LEVEL_BUTTONS_X)
		return (-1);
	if (y > SkillScreenRect.y + SPELL_LEVEL_BUTTONS_Y + 16)
		return (-1);
	if (y < SkillScreenRect.y + SPELL_LEVEL_BUTTONS_Y)
		return (-1);

	//--------------------
	// Now we can check on which skill rectangle exactly the cursor
	// is hovering, since we know that it is hitting, horizontally
	// at least, the row of skill icons.
	//
	for (i = 0; i < NUMBER_OF_SKILL_PAGES; i++) {
		if (x < SkillScreenRect.x + SPELL_LEVEL_BUTTONS_X + (i + 1) * SPELL_LEVEL_BUTTON_WIDTH)
			return i;
	}

	return (-1);
};				// int CursorIsOnWhichSpellLevelButton( int x , int y )

/** 
 *
 *
 */
static void ShowSkillsExplanationScreen(void)
{
	int ICON_OFFSET_X = 20;
	int ICON_OFFSET_Y = 20;
	int TEXT_OFFSET_X = 15;
	SDL_Rect TargetSkillRect;

	//--------------------
	// This should draw the background...
	//
	blit_special_background(SKILL_SCREEN_EXPLANATION_BACKGROUND_CODE);

	//--------------------
	// Draws the skill icon at the correct position
	//
	TargetSkillRect.x = ICON_OFFSET_X;
	TargetSkillRect.y = ICON_OFFSET_Y;
	LoadOneSkillSurfaceIfNotYetLoaded(Me.readied_skill);
	if (use_open_gl) {
		draw_gl_textured_quad_at_screen_position(&SpellSkillMap[Me.readied_skill].icon_surface,
							 TargetSkillRect.x, TargetSkillRect.y);
	} else {
		our_SDL_blit_surface_wrapper(SpellSkillMap[Me.readied_skill].icon_surface.surface, NULL, Screen, &TargetSkillRect);
	}

	//--------------------
	// Draws the explanation text
	// (We will use the FPS display font, cause the small one isn't 
	// very well readable on the silver background.)
	//
	TargetSkillRect.x = TEXT_OFFSET_X;
	TargetSkillRect.w = 320 - 2 * TEXT_OFFSET_X;
	TargetSkillRect.h = 480 - 15;
	SetCurrentFont(FPS_Display_BFont);
	DisplayText(D_(SpellSkillMap[Me.readied_skill].description), 16, 16 + 64 + 16, &TargetSkillRect, TEXT_STRETCH);

};				// void ShowSkillsExplanationScreen( void )

/** 
 * We will draw only those skills to the skills inventory, that are
 * already present in the Tux.  That way the game remains open for new
 * skills to the player and he doesn't now in advance which skills there
 * are, which is more interesting than complete control and overview.
 *
 * Any skills not in use will be marked as -1.
 *
 * The first few entries will be filled with internal skill index numbers
 * for reference.
 *
 */
static void establish_skill_subset_map(int *SkillSubsetMap)
{
	int i;
	int NextPosition = 0;
	for (i = 0; i < number_of_skills; i++) {
		SkillSubsetMap[i] = (-1);
	}
	for (i = 0; i < number_of_skills; i++) {
		if (Me.SkillLevel[i] > 0) {
			SkillSubsetMap[NextPosition] = i;
			NextPosition++;
		}
	}
};				// void establish_skill_subset_map ( int *SkillSubsetMap );

/** Activate nth skill from all skills.
 * @param skill_num is a index into all skills array. 
 *        It must points an aquired skill.
 *
 */
void activate_nth_skill(int skill_num)
{

	//--------------------
	// If the n-th skill does exist, we activate the n-th skill,
	// otherwise we leave the last readied skill.
	//
	if (Me.SkillLevel[skill_num] > 0) {
		Me.readied_skill = skill_num;
	} else {
		ErrorMessage(__FUNCTION__,
			"Tried to activate skill number %d which was not aquired yet.\n",
				PLEASE_INFORM, IS_WARNING_ONLY, skill_num);
	}

};				// void activate_nth_skill ( int skill_num )

/** Set quick skill to the skill on which is mouse cursor
 * @param quick_skill Index of quick skill slot 
 */
void set_nth_quick_skill(int quick_skill)
{
		int ski =
		    CursorIsOnWhichSkillButton(GetMousePos_x(),
					       GetMousePos_y()) +
		    NUMBER_OF_SKILLS_PER_SKILL_PAGE * GameConfig.spell_level_visible;
		
		//--------------------
		// Variable number is an index into already aquired skills.
		// We change it into index into array of all skills.
		int SkillSubsetMap[number_of_skills];
		establish_skill_subset_map(&(SkillSubsetMap[0]));
		ski = SkillSubsetMap[ski];
			
		if (Me.SkillLevel[ski] <=  0) {
			// Invalid skill was selected
			ErrorMessage(__FUNCTION__,
				"Tried to set skill number %d in quick skills. Skill was not aquired yet.\n",
					PLEASE_INFORM, IS_WARNING_ONLY, ski);
			return;
		}
		int i;
	
		for (i = 0; i < 10; i++) {
			if (Me.program_shortcuts[i] == ski && i != quick_skill)
				Me.program_shortcuts[i] = -1;
		}

		Me.program_shortcuts[quick_skill] = ski;
}; // void set_nth_quick_skill(int quick_skill)

/**
 * This function displays the SKILLS SCREEN.  This is NOT the same as the
 * CHARACTER SCREEN.  In the skills screen you can see what skills/spells
 * you currenlty have availabe and you can select a new readied skill by
 * clicking on it with the mouse.
 */
void ShowSkillsScreen(void)
{
#define INTER_SKILLRECT_DIST 17
#define FIRST_SKILLRECT_Y 16

	static SDL_Rect ButtonRect;
	char CharText[1000];
	point CurPos;
	int i;
	SDL_Rect SpellLevelRect;
	int SkillSubsetMap[number_of_skills];
	int SkillOfThisSlot;
	point SkillRectLocations[NUMBER_OF_SKILLS_PER_SKILL_PAGE];

	DebugPrintf(2, "\n%s(): Function call confirmed.", __FUNCTION__);

	SkillScreenRect.x = CHARACTERRECT_X;
	SkillScreenRect.y = 0;
	SkillScreenRect.w = CHARACTERRECT_W;
	SkillScreenRect.h = CHARACTERRECT_H;

	for (i = 0; i < NUMBER_OF_SKILLS_PER_SKILL_PAGE; i++) {
		SkillRectLocations[i].x = SkillScreenRect.x + 20;
		SkillRectLocations[i].y = SkillScreenRect.y + FIRST_SKILLRECT_Y + i * (64 + INTER_SKILLRECT_DIST) + 3;
	}

	//--------------------
	// If the log is not set to visible right now, we do not need to 
	// do anything more, but to restore the usual user rectangle size
	// back to normal and to return...
	//
	if (GameConfig.SkillScreen_Visible == FALSE)
		return;

	//--------------------
	// We will use the FPS display font, cause the small one isn't 
	// very well readable on the silver background
	//
	SetCurrentFont(FPS_Display_BFont);

	//--------------------
	// Maybe the skill circle images for clicking between different spell circles
	// have not been loaded yet.  Then it is time to do so.  If this was already
	// done before, then the function will know it and don't do anything anyway.
	//
	Load_Skill_Level_Button_Surfaces();

	// --------------------
	// We will need the current mouse position on several spots...
	//
	CurPos.x = GetMousePos_x();
	CurPos.y = GetMousePos_y();

	//--------------------
	// We will draw only those skills to the skills inventory, that are
	// already present in the Tux.  That way the game remains open for new
	// skills to the player and he doesn't now in advance which skills there
	// are, which is more interesting than complete control and overview.
	//
	establish_skill_subset_map(SkillSubsetMap);

	//--------------------
	// At this point we know, that the skill screen is desired and must be
	// displayed in-game:
	//
	// SDL_SetClipRect( Screen, NULL );
	// our_SDL_blit_surface_wrapper ( SkillScreenImage , NULL , Screen , &SkillScreenRect );
	//
	blit_special_background(SKILL_SCREEN_BACKGROUND_CODE);

	if (GameConfig.skill_explanation_screen_visible)
		ShowSkillsExplanationScreen();

	//--------------------
	// According to the page in the spell book currently opened,
	// we draw a 'button' or activation mark over the appropriate spot
	//
	SpellLevelRect.x = SkillScreenRect.x + SPELL_LEVEL_BUTTONS_X + SPELL_LEVEL_BUTTON_WIDTH * GameConfig.spell_level_visible;
	SpellLevelRect.y = SkillScreenRect.y + SPELL_LEVEL_BUTTONS_Y;
	our_SDL_blit_surface_wrapper(SpellLevelButtonImageList[GameConfig.spell_level_visible].surface, NULL, Screen, &SpellLevelRect);

	//--------------------
	// Now we fill in the skills available to this bot.  ( For now, these skills 
	// are not class-specific, like in diablo or something, but this is our first
	// approach to the topic after all.... :)
	//
	for (i = 0; i < NUMBER_OF_SKILLS_PER_SKILL_PAGE; i++) {
		ButtonRect.x = SkillRectLocations[i].x;
		ButtonRect.y = SkillRectLocations[i].y;
		ButtonRect.w = 64;
		ButtonRect.h = 64;

		if (i + NUMBER_OF_SKILLS_PER_SKILL_PAGE * GameConfig.spell_level_visible >= number_of_skills)
			break;
		SkillOfThisSlot = SkillSubsetMap[i + NUMBER_OF_SKILLS_PER_SKILL_PAGE * GameConfig.spell_level_visible];
		if (SkillOfThisSlot < 0)
			continue;

		LoadOneSkillSurfaceIfNotYetLoaded(SkillOfThisSlot);

		if (use_open_gl) {
			draw_gl_textured_quad_at_screen_position(&SpellSkillMap[SkillOfThisSlot].icon_surface, ButtonRect.x, ButtonRect.y);
		} else {
			our_SDL_blit_surface_wrapper(SpellSkillMap[SkillOfThisSlot].icon_surface.surface, NULL, Screen, &ButtonRect);
		}

		SetCurrentFont(FPS_Display_BFont);

		// Program shortcut
		int sci;
		for (sci = 0; sci < 10; sci++) {
			if (Me.program_shortcuts[sci] == SkillOfThisSlot)
				break;
		}

		if (sci != 10) {
			// print the quick key number
			char str[10];
			sprintf(str, "F%d\n", 5 + sci);
			DisplayText(str, ButtonRect.x + ButtonRect.w - 2 - TextWidth(str), ButtonRect.y, &SkillScreenRect, TEXT_STRETCH);
		}
		// Name of the skill

		DisplayText(D_(SpellSkillMap[SkillOfThisSlot].name),
			    16 + 64 + 16 + SkillScreenRect.x,
			    FIRST_SKILLRECT_Y - 6 + i * (64 + INTER_SKILLRECT_DIST) + SkillScreenRect.y, &SkillScreenRect, TEXT_STRETCH);

		SetCurrentFont(Messagestat_BFont);
		int tmp, tmp2;
		int nextypos =
		    FIRST_SKILLRECT_Y - 8 + i * (64 + INTER_SKILLRECT_DIST) + SkillScreenRect.y + 2 * FontHeight(GetCurrentFont());

		// Program revision
		sprintf(CharText, _("Program revision: %c%d%c "), font_switchto_msgvar[0], Me.SkillLevel[SkillOfThisSlot],
			font_switchto_msgstat[0]);
		DisplayText(CharText, 16 + 64 + 16 + SkillScreenRect.x, nextypos, &SkillScreenRect, TEXT_STRETCH);
		nextypos += FontHeight(GetCurrentFont());

		// Heat cost/cooling
		tmp = calculate_program_heat_cost(SkillOfThisSlot);
		if (tmp != 0) {
			if (tmp > 0)
				sprintf(CharText, _("Heat produced: %c%d%c "), font_switchto_msgvar[0], tmp, font_switchto_msgstat[0]);
			else
				sprintf(CharText, _("Cooling: %c%d%c "), font_switchto_msgvar[0], -tmp, font_switchto_msgstat[0]);
			DisplayText(CharText, 16 + 64 + 16 + SkillScreenRect.x, nextypos, &SkillScreenRect, TEXT_STRETCH);
			nextypos += FontHeight(GetCurrentFont());
		}
		// Damage/healing
		tmp = calculate_program_hit_damage_low(SkillOfThisSlot);
		tmp2 = calculate_program_hit_damage_high(SkillOfThisSlot);
		if (tmp != 0) {
			if (tmp > 0) {
				if (tmp == tmp2)
					sprintf(CharText, _("Damage: %c%d%c "), font_switchto_msgvar[0], tmp, font_switchto_msgstat[0]);
				else
					sprintf(CharText, _("Damage: %c%d-%d%c "), font_switchto_msgvar[0], tmp, tmp2,
						font_switchto_msgstat[0]);
			} else {
				if (tmp == tmp2)
					sprintf(CharText, _("Healing: %c%d%c "), font_switchto_msgvar[0], -tmp, font_switchto_msgstat[0]);
				else
					sprintf(CharText, _("Healing: %c%d-%d%c "), font_switchto_msgvar[0], -tmp, -tmp2,
						font_switchto_msgstat[0]);
			}
			DisplayText(CharText, 16 + 64 + 16 + SkillScreenRect.x, nextypos, &SkillScreenRect, TEXT_STRETCH);
			nextypos += FontHeight(GetCurrentFont());
		}
		// Special effect and duration
		if (strcmp(SpellSkillMap[SkillOfThisSlot].effect, "none")) {
			if (!strcmp(SpellSkillMap[SkillOfThisSlot].effect, "paralyze"))
				sprintf(CharText, _("Paralyze"));
			else if (!strcmp(SpellSkillMap[SkillOfThisSlot].effect, "slowdown"))
				sprintf(CharText, _("Slow down"));
			else if (!strcmp(SpellSkillMap[SkillOfThisSlot].effect, "invisibility"))
				sprintf(CharText, _("Invisible"));
			else if (!strcmp(SpellSkillMap[SkillOfThisSlot].effect, "poison"))
				sprintf(CharText, _("Poison"));
			else if (!strcmp(SpellSkillMap[SkillOfThisSlot].effect, "takeover")) {
				tmp = Me.SkillLevel[SkillOfThisSlot] + 2;
				sprintf(CharText, _("Takeover charges: %c%d%c "), font_switchto_msgvar[0], tmp, font_switchto_msgstat[0]);
			} else if (!strcmp(SpellSkillMap[SkillOfThisSlot].effect, "teleport_home"))
				sprintf(CharText, _("Escape"));
			else if (!strcmp(SpellSkillMap[SkillOfThisSlot].effect, "passive"))
				sprintf(CharText, _("Passive"));
			else if (!strcmp(SpellSkillMap[SkillOfThisSlot].effect, "identify"))
				sprintf(CharText, " ");
			else if (!strcmp(SpellSkillMap[SkillOfThisSlot].effect, "weapon"))
				sprintf(CharText, " ");
			else if (!strcmp(SpellSkillMap[SkillOfThisSlot].effect, "repair"))
				sprintf(CharText, " ");
			else if (!strcmp(SpellSkillMap[SkillOfThisSlot].effect, "satellite"))
				sprintf(CharText, " ");
			else if (!strcmp(SpellSkillMap[SkillOfThisSlot].effect, "light"))
				sprintf(CharText, " ");
			else if (!strcmp(SpellSkillMap[SkillOfThisSlot].effect, "burnup"))
				sprintf(CharText, " ");

			float dur = calculate_program_effect_duration(SkillOfThisSlot);
			if (dur > 0)
				sprintf(CharText + strlen(CharText), _(" for %c%.1f%c seconds"), font_switchto_msgvar[0], dur,
					font_switchto_msgstat[0]);

			DisplayText(CharText, 16 + 64 + 16 + SkillScreenRect.x, nextypos, &SkillScreenRect, TEXT_STRETCH);
			nextypos += FontHeight(GetCurrentFont());
		}

	}

	//--------------------
	// Now we see if perhaps the player has just clicked on one of the skills
	// available to this class.  In this case of course we must set a different
	// skill/spell as the currently activated skill/spell.
	//
	if ((CursorIsOnWhichSkillButton(CurPos.x, CurPos.y) != (-1)) && MouseLeftClicked()) {
		if (CursorIsOnWhichSkillButton(CurPos.x, CurPos.y) +
		    NUMBER_OF_SKILLS_PER_SKILL_PAGE * GameConfig.spell_level_visible < number_of_skills)
			if (SkillSubsetMap[CursorIsOnWhichSkillButton(CurPos.x, CurPos.y) +
					   NUMBER_OF_SKILLS_PER_SKILL_PAGE * GameConfig.spell_level_visible] >= 0)
				Me.readied_skill = SkillSubsetMap[CursorIsOnWhichSkillButton(CurPos.x, CurPos.y) +
								  NUMBER_OF_SKILLS_PER_SKILL_PAGE * GameConfig.spell_level_visible];
	}

	if (MouseCursorIsOnButton(OPEN_CLOSE_SKILL_EXPLANATION_BUTTON, CurPos.x, CurPos.y) && MouseLeftClicked()) {
		toggle_game_config_screen_visibility(GAME_CONFIG_SCREEN_VISIBLE_SKILL_EXPLANATION);
		while (MouseLeftPressed())
			SDL_Delay(1);
	}
	//--------------------
	// Now we see if perhaps the player has just clicked on another skill level
	// button.  In this case of course we must set a different skill/spell level
	// as the currently visible spell level.
	//
	if ((CursorIsOnWhichSpellPageButton(CurPos.x, CurPos.y) != (-1)) && MouseLeftClicked()) {
		GameConfig.spell_level_visible = CursorIsOnWhichSpellPageButton(CurPos.x, CurPos.y);
	}
	//--------------------
	// We want to know, if the button was pressed the previous frame when we
	// are in the next frame and back in this function.  Therefore we store
	// the current button situation, so that we can conclude on button just
	// pressed later.
	//

};				// ShowSkillsScreen ( void )

#undef _skills_c
