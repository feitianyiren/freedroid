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

local Tux = FDrpg.get_tux()

return {
	{
		id = "614_sub.everytime",
		code = function()
			npc_says(_"Entering Interactive Product Information Database")
			npc_says(_"I am a 614 security bot, once one of the best-selling products of Nicholson, Inc..")
			show("614_sub.node1", "614_sub.node2", "614_sub.node3", "614_sub.node99")
		end,
	},
	{
		id = "614_sub.node1",
		text = _"Why are you not hostile as are all the other bots?",
		code = function()
			npc_says(_"I belong to an older generation of bots, running a custom-made security guard operating system.")
			npc_says(_"None of the bots of this generation were affected by the proceedings that led to bots attacking, generally known as the Great Assault.")
			npc_says(_"Even though discontinued by Nicholson, Inc. long ago, today, some people have picked up the work and are even trying to improve the operating system running on my series.")
			npc_says(_"Don't forget to buy products from Nicholson, Inc. They are the best.")
			hide("614_sub.node1")
		end,
	},
	{
		id = "614_sub.node2",
		text = _"What can you tell me about the Nicholson company?",
		code = function()
			npc_says(_"Nicholson, Inc. produces security bots.")
			npc_says(_"It was founded by Karl Nicholson, a former Kernel hacker who was not satisfied with the development of the GPL license.")
			npc_says(_"Nicholson decided to stick to one GPL version because he saw things getting worse.")
			npc_says(_"Others were warned, but nobody listened to Nicholsons announcements.")
			npc_says(_"He built up a team and developed his own Kernel called 'Nkernel'.")
			npc_says(_"The Nkernel development team grew fast and technicians started to produce their own droids optimized for the Nkernel.")
			npc_says(_"The team became a company which released their first security droid, called 610.")
			npc_says(_"I am the 5th generation version of their security droid line.")
			hide("614_sub.node2")
		end,
	},
	{
		id = "614_sub.node3",
		text = _"What can you tell me about the 614 type?",
		code = function()
			npc_says(_"The official manual classifies the 614 as a high class security droid.")
			npc_says(_"It was mainly used within ships to protect certain areas from intruders.")
			npc_says(_"It is considered an old but reliable device.")
			npc_says(_"So don't worry. I'm still in pretty good shape.")
			npc_says(_"The latest version of the security line is the 615.")
			-- probably some when a cookie can check whether tux already saw such a droid and trigger/switch on the following dialog part
			npc_says(_"Unfortunately its development was not finished when the great assault started.")
			npc_says(_"The lack of security options on the system of the 615 led to a malfunction.")
			npc_says(_"Circumstances that led to the great assault even made the 615 go crazy and attack people.")
			npc_says(_"For the sake of your health, you should stay away from the 615 droids.")
			npc_says(_"You have been warned!")
			hide("614_sub.node3")
		end,
	},
	{
		id = "614_sub.node99",
		text = _"I think, that was enough product information, thank you.",
		code = function()
			npc_says(_"Leaving Interactive Product Information Database")
			next("after-614_sub")
		end,
	},
}
