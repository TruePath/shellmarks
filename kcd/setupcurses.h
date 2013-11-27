/*
	setupcurses.h	ncurses Header Inclusion
	Copyright (c) 1998,1999,2000,2001 Kriang Lerdsuwanakij
	email:		lerdsuwa@users.sourceforge.net

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __K_SETUPCURSES_H
#define __K_SETUPCURSES_H

#ifdef NCURSESAC_NEW_INCLUDE
# ifdef NCURSESAC_WIDE
#  include <ncursesw/curses.h>
# else
#  include <ncurses/curses.h>
# endif
#else
# include <curses.h>
#endif

//
// Remove conflict macros
//
#undef move			/* Used by C++ string */
#undef clear			/* Used by C++ vector */
#undef erase			/* Used by C++ string */

//
// Other include files
//
#include "config.h"
#include <string>

USING_NAMESPACE_STD;

//
// Decide whether mouse support should be activated
// Some earlier versions of ncurses do not have mousemask(...)
//
#if !defined(NCURSESAC_HAVE_MOUSEMASK) || !defined(NCURSESAC_ENABLE_MOUSE)
# undef NCURSES_MOUSE_VERSION
#endif

//
// Decide whether scroll bar should be enabled by default
// For ncurses version before 4.2, scrolling the screen will cause glitches
// when scroll bar is displayed.
//
#if defined(NCURSES_VERSION_MAJOR) && defined(NCURSES_VERSION_MINOR) \
    && ((NCURSES_VERSION_MAJOR == 4 && NCURSES_VERSION_MINOR >= 2)   \
        || NCURSES_VERSION_MAJOR > 4)
# define KCD_NC_GOOD_SCROLL
#endif

//
// Define new data type to work with
//
typedef int	COLOR_TYPE;
typedef int	ATTR_TYPE;

//
// Handle attr_get and attr_set macros changes
//
#ifdef NCURSESAC_NEW_ATTR_GET_ATTR_SET

// For ncurses-4.2 snapshot 990109 or newer

inline int my_wattr_get(WINDOW *w, attr_t *a, short *p)
{
	return wattr_get(w, a, p, 0);
}

inline int my_wattr_set(WINDOW *w, attr_t a, short p)
{
	return wattr_set(w, a, p, 0);
}

#else

// For ncurses-4.2 snapshot before 990109

inline int my_wattr_get(WINDOW *w, attr_t *a, short *p)
{
	*p = 0;
	*a = wattr_get(w);
	return 0;
}

inline int my_wattr_set(WINDOW *w, attr_t a, short p)
{
	return wattr_set(w, a);
}

#endif

#endif	/* __K_SETUPCURSES_H */
