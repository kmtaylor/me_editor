/* Midi address dumper - requires roland manual
 *
 * Boss ME-5 Editor
 *
 * Copyright (C) 2012 Kim Taylor <kmtaylor@gmx.com>
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License as published by the
 *	Free Software Foundation version 2 of the License.
 * 
 *	This program is distributed in the hope that it will be useful, but
 *	WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	General Public License for more details.
 * 
 *	You should have received a copy of the GNU General Public License along
 *	with this program; if not, write to the Free Software Foundation, Inc.,
 *	675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: class_map.c,v 1.3 2012/06/07 14:33:33 kmtaylor Exp $
 */


#include <stdio.h>
#include "midi_tree.h"

struct s_class_map class_map[] = {
        { .parent_name = "TOPLEVEL",
          .child_names = (const char * []) {
	    "Patch 1 / 1 / 1 (01)", "Patch",
	    "Patch 1 / 1 / 2 (02)", "Patch",
	    "Patch 1 / 1 / 3 (03)", "Patch",
	    "Patch 1 / 1 / 4 (04)", "Patch",
	    "Patch 1 / 2 / 1 (05)", "Patch",
	    "Patch 1 / 2 / 2 (06)", "Patch",
	    "Patch 1 / 2 / 3 (07)", "Patch",
	    "Patch 1 / 2 / 4 (08)", "Patch",
	    "Patch 1 / 3 / 1 (09)", "Patch",
	    "Patch 1 / 3 / 2 (10)", "Patch",
	    "Patch 1 / 3 / 3 (11)", "Patch",
	    "Patch 1 / 3 / 4 (12)", "Patch",
	    "Patch 1 / 4 / 1 (13)", "Patch",
	    "Patch 1 / 4 / 2 (14)", "Patch",
	    "Patch 1 / 4 / 3 (15)", "Patch",
	    "Patch 1 / 4 / 4 (16)", "Patch",
	    "Patch 2 / 1 / 1 (17)", "Patch",
	    "Patch 2 / 1 / 2 (18)", "Patch",
	    "Patch 2 / 1 / 3 (19)", "Patch",
	    "Patch 2 / 1 / 4 (20)", "Patch",
	    "Patch 2 / 2 / 1 (21)", "Patch",
	    "Patch 2 / 2 / 2 (22)", "Patch",
	    "Patch 2 / 2 / 3 (23)", "Patch",
	    "Patch 2 / 2 / 4 (24)", "Patch",
	    "Patch 2 / 3 / 1 (25)", "Patch",
	    "Patch 2 / 3 / 2 (26)", "Patch",
	    "Patch 2 / 3 / 3 (27)", "Patch",
	    "Patch 2 / 3 / 4 (28)", "Patch",
	    "Patch 2 / 4 / 1 (29)", "Patch",
	    "Patch 2 / 4 / 2 (30)", "Patch",
	    "Patch 2 / 4 / 3 (31)", "Patch",
	    "Patch 2 / 4 / 4 (32)", "Patch",
	    "Patch 3 / 1 / 1 (33)", "Patch",
	    "Patch 3 / 1 / 2 (34)", "Patch",
	    "Patch 3 / 1 / 3 (35)", "Patch",
	    "Patch 3 / 1 / 4 (36)", "Patch",
	    "Patch 3 / 2 / 1 (37)", "Patch",
	    "Patch 3 / 2 / 2 (38)", "Patch",
	    "Patch 3 / 2 / 3 (39)", "Patch",
	    "Patch 3 / 2 / 4 (40)", "Patch",
	    "Patch 3 / 3 / 1 (41)", "Patch",
	    "Patch 3 / 3 / 2 (42)", "Patch",
	    "Patch 3 / 3 / 3 (43)", "Patch",
	    "Patch 3 / 3 / 4 (44)", "Patch",
	    "Patch 3 / 4 / 1 (45)", "Patch",
	    "Patch 3 / 4 / 2 (46)", "Patch",
	    "Patch 3 / 4 / 3 (47)", "Patch",
	    "Patch 3 / 4 / 4 (48)", "Patch",
	    "Patch 4 / 1 / 1 (49)", "Patch",
	    "Patch 4 / 1 / 2 (50)", "Patch",
	    "Patch 4 / 1 / 3 (51)", "Patch",
	    "Patch 4 / 1 / 4 (52)", "Patch",
	    "Patch 4 / 2 / 1 (53)", "Patch",
	    "Patch 4 / 2 / 2 (54)", "Patch",
	    "Patch 4 / 2 / 3 (55)", "Patch",
	    "Patch 4 / 2 / 4 (56)", "Patch",
	    "Patch 4 / 3 / 1 (57)", "Patch",
	    "Patch 4 / 3 / 2 (58)", "Patch",
	    "Patch 4 / 3 / 3 (59)", "Patch",
	    "Patch 4 / 3 / 4 (60)", "Patch",
	    "Patch 4 / 4 / 1 (61)", "Patch",
	    "Patch 4 / 4 / 2 (62)", "Patch",
	    "Patch 4 / 4 / 3 (63)", "Patch",
	    "Patch 4 / 4 / 4 (64)", "Patch",
	    NULL
	    }
        },
	{ NULL }
};
