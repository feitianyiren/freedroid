---------------------------------------------------------------------
-- This file is part of Freedroid
--
-- Freedroid is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- Freedroid is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with Freedroid; see the file COPYING. If not, write to the
-- Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
-- MA 02111-1307 USA
----------------------------------------------------------------------

return {
	EveryTime = function()
		play_sound("effects/Menu_Item_Deselected_Sound_0.ogg")
		Maintenance_Terminal_year = os.date("%Y") + 45 -- current year + 45
		Maintenance_Terminal_date = os.date("%a %b %d %H:%M:%S") -- emulate os.date() but without the year
		Maintenance_Terminal_prompt = "dixon@maintenance: ~ # "

		cli_says(_"Login : ", "NO_WAIT")
		tux_says(_"dixon", "NO_WAIT")
		cli_says(_"Password : ", "NO_WAIT")
		tux_says("*******", "NO_WAIT")
		npc_says(_"Hello, Dave.")
		if (Maintenance_Terminal_date == nil) then
			npc_says(_"First login from /dev/ttySO on %s %d", Maintenance_Terminal_date_1, Maintenance_Terminal_year, "NO_WAIT")
		else
			npc_says(_"Last login from /dev/ttyS0 on %s %d", Maintenance_Terminal_date, Maintenance_Terminal_year, "NO_WAIT")
		end
		Maintenance_Terminal_date = Maintenance_Terminal_date_1
		cli_says(Maintenance_Terminal_prompt, "NO_WAIT")
		show("node0", "node99")
	end,

	{
		id = "node0",
		text = _"sys_status --all",
		code = function()
			if (not Maintenance_Terminal_devices) then
				Maintenance_Terminal_devices = true
				npc_says(_"Detecting connected devices...")
				npc_says(_"Found 03 connected auto-guns.")
				npc_says(_"Found 02 connected gates.")
				npc_says(_"Found 01 connected automated factories.")
				npc_says(_"Anomalies detected.")
				npc_says("...")
			end

			if (cmp_obstacle_state("Maintenance-gun", "enabled")) then
				npc_says(_"Gun 01 status: ENABLED", "NO_WAIT")
				npc_says(_"Gun 02 status: ENABLED", "NO_WAIT")
				npc_says(_"Gun 03 status: ENABLED", "NO_WAIT")
				if (not Maintenance_Terminal_refusal) then
					show("node3")
				else
					show("node10")
				end
			else
				npc_says(_"Gun 01 status: DISABLED", "NO_WAIT")
				npc_says(_"Gun 02 status: DISABLED", "NO_WAIT")
				npc_says(_"Gun 03 status: DISABLED", "NO_WAIT")
				show("node4")
			end

			if (cmp_obstacle_state("Maintenance-gungate", "opened")) then
				npc_says(_"Access Gate status: OPEN", "NO_WAIT")
				show("node5")
			else
				npc_says(_"Access Gate status: CLOSED", "NO_WAIT")
				show("node6")
			end

			if (cmp_obstacle_state("MiniFactory-Gate", "opened")) then
				npc_says(_"Factory Gate status: OPEN", "NO_WAIT")
			else
				npc_says(_"Factory Gate status: CLOSED", "NO_WAIT")
				show("node12")
			end

			if (Minifactory_online) then
				npc_says(_"Automated Factory status: ONLINE", "NO_WAIT")
			else
				npc_says(_"Automated Factory status: OFFLINE", "NO_WAIT")
			end

			cli_says(Maintenance_Terminal_prompt, "NO_WAIT")

			hide("node0")
		end,
	},
	{
		id = "node3",
		text = _"disable guns",
		code = function()
			npc_says(_"I'm sorry. I'm afraid I can't let you do that.")
			cli_says(Maintenance_Terminal_prompt, "NO_WAIT")
			hide("node3") show("node7")
		end,
	},
	{
		id = "node4",
		text = _"sudo enable guns",
		code = function()
			cli_says(_"[sudo] password for dixon: ")
			tux_says("*******", "NO_WAIT")
			npc_says(_"Enabling gun 01 ...", "NO_WAIT")
			change_obstacle_state("Maintenance-gun1", "enabled")
			npc_says(_"Enabling gun 02 ...", "NO_WAIT")
			change_obstacle_state("Maintenance-gun2", "enabled")
			npc_says(_"Enabling gun 03 ...", "NO_WAIT")
			change_obstacle_state("Maintenance-gun", "enabled")
			npc_says(_"SUCCESS")
			cli_says(Maintenance_Terminal_prompt, "NO_WAIT")
			hide("node4") show("node0")
		end,
	},
	{
		id = "node5",
		text = _"close access gate",
		code = function()
			npc_says(_"Closing Access Gate ...", "NO_WAIT")
			change_obstacle_state("Maintenance-gungate", "closed")
			npc_says(_"SUCCESS")
			cli_says(Maintenance_Terminal_prompt, "NO_WAIT")
			hide("node5") show("node0")
		end,
	},
	{
		id = "node6",
		text = _"open access gate",
		code = function()
			npc_says(_"Opening Access Gate ...", "NO_WAIT")
			if (Singularity_quest_rejected) then
				npc_says(_"Permission denied.")
				if (Maintenance_Terminal_accessgate_nope == "official") then else
					Maintenance_Terminal_accessgate_nope = "true"
				end
			else
				change_obstacle_state("Maintenance-gungate", "opened")
			end
			cli_says(Maintenance_Terminal_prompt, "NO_WAIT")
			hide("node6") show("node0")
		end,
	},
	{
		id = "node7",
		text = _"sudo disable guns",
		code = function()
			cli_says(_"[sudo] password for dixon: ")
			tux_says("*******")
			npc_says(_"Would you like a sandwich with that?")
			cli_says("> ", "NO_WAIT")
			show("node8", "node9")
			push_topic("Make a sandwich")
		end,
	},
	{
		id = "node8",
		text = _"yes",
		topic = "Make a sandwich",
		code = function()
			npc_says_random(_"Now how would I make one of those?",
				_"Sudo doesn't give you magic powers.")
			Maintenance_Terminal_want_sandwich = true
			cli_says(Maintenance_Terminal_prompt, "NO_WAIT")
			pop_topic()
		end,
	},
	{
		id = "node9",
		text = _"no",
		topic = "Make a sandwich",
		code = function()
			npc_says(_"Good choice.")
			Maintenance_Terminal_refusal = true
			hide("node7", "node8", "node9") next("node11")
			pop_topic()
		end,
	},
	{
		id = "node10",
		text = _"sudo disable guns",
		code = function()
			cli_says(_"[sudo] password for dixon: ")
			tux_says("*******")
			hide("node10") next("node11")
		end,
	},
	{
		id = "node11",
		text = _"BUG, REPORT ME! Maintenance-Terminal node11 -- Disable Guns",
		code = function()
			npc_says(_"Disabling gun 01 ...")
			change_obstacle_state("Maintenance-gun1", "disabled")
			npc_says(_"Disabling gun 02 ...")
			change_obstacle_state("Maintenance-gun2", "disabled")
			npc_says(_"Disabling gun 03 ...")
			change_obstacle_state("Maintenance-gun", "disabled")
			npc_says(_"SUCCESS")
			cli_says(Maintenance_Terminal_prompt, "NO_WAIT")
			show("node0")
		end,
	},
	{
		id = "node12",
		text = _"open autofactory gate",
		code = function()
			npc_says(_"Opening Automated Factory Gate ...", "NO_WAIT")
			change_obstacle_state("MiniFactory-Gate", "opened")
			npc_says(_"SUCCESS")
			cli_says(Maintenance_Terminal_prompt, "NO_WAIT")
			hide("node12") show("node0")
		end,
	},
	{
		id = "node99",
		text = _"logout",
		code = function()
			npc_says(_"Exiting", "NO_WAIT")
			npc_says(_"Goodbye Dave.")
			hide("node3", "node4", "node5", "node6", "node7", "node8", "node9", "node10")
			play_sound("effects/Menu_Item_Selected_Sound_1.ogg")
			end_dialog()
		end,
	},
}