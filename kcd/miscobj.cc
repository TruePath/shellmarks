/*
	miscobj.cc	Various NCurses C++ Classes
	Copyright (c) 1996-8,2000,2001,2003,2007,2009 Kriang Lerdsuwanakij
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

#include "miscobj.h"
#include "error.h"

//
// Remove conflict macros introduced by <curses.h>
//
#undef ERR
#undef OK

#include <signal.h>
#include CXX__HEADER_cstdio
#include CXX__HEADER_cstdlib
#include <sys/stat.h>

#ifdef NCURSESAC_NEW_INCLUDE
# ifdef NCURSESAC_WIDE
#  include <ncursesw/term.h>
# else
#  include <ncurses/term.h>
# endif
#else
# include <term.h>
#endif

#include <termios.h>
#include <sys/ioctl.h>

#if HAVE_UNISTD_H
# include <unistd.h>
# include <sys/types.h>
#endif

#ifdef USE_UTF8_MODE
# ifdef HAVE_WCHAR_H
#  ifndef __USE_XOPEN
#   define __USE_XOPEN
#  endif
#  include CXX__HEADER_cwchar
# endif
#endif

#include "strmisc.h"

			// We still need ncurses's ERR
#define ERR (-1)

/*************************************************************************
	Entity info
*************************************************************************/

entity_info entity_table[] = {
	{ "lt", 2, '<', '<' },
	{ "gt", 2, '>', '>' },
	{ "amp", 3, '&', '&' },
	{ "quot", 4, '\"', '\"' },

#ifndef TRIM_NO_LINEDRAW
	{ "boxur", 5, 0, 0x2514 },			// 4
	{ "boxdr", 5, 0, 0x250C },
	{ "boxdl", 5, 0, 0x2510 },
	{ "boxul", 5, 0, 0x2518 },

	{ "boxvl", 5, 0, 0x2524 },
	{ "boxvr", 5, 0, 0x251C },
	{ "boxhu", 5, 0, 0x2534 },
	{ "boxhd", 5, 0, 0x252C },

	{ "boxh", 4, 0, 0x2500 },
	{ "boxv", 4, 0, 0x2502 },

	{ "rarr", 4, 0, 0x2192 },
	{ "larr", 4, 0, 0x2190 },
	{ "darr", 4, 0, 0x2193 },
	{ "uarr", 4, 0, 0x2191 },

	{ "degree", 6, 0, 0x00B0 },
	{ "plminus", 7, 0, 0x00B1 },
	{ "lequal", 6, 0, 0x2264 },
	{ "gequal", 6, 0, 0x2265 },
	{ "nequal", 6, 0, 0x2262 },

	{ "bullet", 6, 0, 0x2022 },
	{ "diamond", 7, 0, 0x25C6 },

	{ "ckboard", 7, 0, 0x2592 },
	{ "board", 5, 0, 0x2591 },
	{ "lantern", 7, 0, 0x256C },
	{ "block", 5, 0, 0x2588 },

	{ "pi", 2, 0, 0x00B6 },
	{ "sterling", 8, 0, 0x00A3 },
	{ "s1", 2, 0, 0x2594 },
	{ "s9", 2, 0, 0x2581 },
	{ "s3", 2, 0, 0x2501 },
	{ "s7", 2, 0, 0x2501 },
	{ "space", 5, ' ', ' ' },	// For alternate entity display
#endif

#ifndef TRIM_NO_LATIN
	{ "iexcl", 5, 161, 161 },
	{ "cent", 4, 162, 162 },
	{ "pound", 5, 163, 163 },
	{ "curren", 6, 164, 164 },
	{ "yen", 3, 165, 165 },
	{ "brvbar", 6, 166, 166 },
	{ "sect", 4, 167, 167 },
	{ "uml", 3, 168, 168 },
	{ "copy", 4, 169, 169 },
	{ "ordf", 4, 170, 170 },
	{ "laquo", 5, 171, 171 },
	{ "not", 3, 172, 172 },
	{ "shy", 3, 173, 173 },
	{ "reg", 3, 174, 174 },
	{ "macr", 3, 175, 175 },
	{ "deg", 3, 176, 176 },
	{ "plusmn", 6, 177, 177 },
	{ "sup2", 4, 178, 178 },
	{ "sup3", 4, 179, 179 },
	{ "acute", 5, 180, 180 },
	{ "micro", 5, 181, 181 },
	{ "para", 4, 182, 182 },
	{ "middot", 6, 183, 183 },
	{ "cedil", 5, 184, 184 },
	{ "sup1", 4, 185, 185 },
	{ "ordm", 4, 186, 186 },
	{ "raquo", 5, 187, 187 },
	{ "frac14", 6, 188, 188 },
	{ "frac12", 6, 189, 189 },
	{ "frac34", 6, 190, 190 },
	{ "iquest", 6, 191, 191 },

	{ "Agrave", 6, 192, 192 },
	{ "Acute", 5, 193, 193 },
	{ "Acirc", 5, 194, 194 },
	{ "Atilde", 6, 195, 195 },
	{ "Auml", 4, 196, 196 },
	{ "Aring", 5, 197, 197 },
	{ "AElig", 5, 198, 198 },
	{ "Ccedil", 6, 199, 199 },
	{ "Egrave", 6, 200, 200 },
	{ "Eacute", 6, 201, 201 },
	{ "Ecirc", 5, 202, 202 },
	{ "Euml", 4, 203, 203 },
	{ "Igrave", 6, 204, 204 },
	{ "Iacute", 6, 205, 205 },
	{ "Icirc", 5, 206, 206 },
	{ "Iuml", 4, 207, 207 },
	{ "ETH", 3, 208, 208 },
	{ "Ntilde", 6, 209, 209 },
	{ "Ograve", 6, 210, 210 },
	{ "Oacute", 6, 211, 211 },
	{ "Ocirc", 5, 212, 212 },
	{ "Otilde", 6, 213, 213 },
	{ "Ouml", 4, 214, 214 },
	{ "times", 5, 215, 215 },
	{ "Oslash", 6, 216, 216 },
	{ "Ugrave", 6, 217, 217 },
	{ "Uacute", 6, 218, 218 },
	{ "Ucirc", 5, 219, 219 },
	{ "Uuml", 4, 220, 220 },
	{ "Yacute", 6, 221, 221 },
	{ "THORN", 5, 222, 222 },

	{ "szlig", 5, 223, 223 },
	{ "agrave", 6, 224, 224 },
	{ "aacute", 6, 225, 225 },
	{ "acirc", 5, 226, 226 },
	{ "atilde", 6, 227, 227 },
	{ "auml", 4, 228, 228 },
	{ "aring", 5, 229, 229 },

	{ "aelig", 5, 230, 230 },
	{ "ccedil", 6, 231, 231 },
	{ "egrave", 6, 232, 232 },
	{ "eacute", 6, 233, 233 },
	{ "ecirc", 5, 234, 234 },
	{ "euml", 4, 235, 235 },
	{ "igrave", 6, 236, 236 },
	{ "iacute", 6, 237, 237 },
	{ "icirc", 5, 238, 238 },
	{ "iuml", 4, 239, 239 },
	{ "eth", 3, 240, 240 },
	{ "ntilde", 6, 241, 241 },
	{ "ograve", 6, 242, 242 },
	{ "oacute", 6, 243, 243 },
	{ "ocirc", 5, 244, 244 },
	{ "otilde", 6, 245, 245 },
	{ "ouml", 4, 246, 246 },
	{ "divide", 6, 247, 247 },
	{ "oslash", 6, 248, 248 },
	{ "ugrave", 6, 249, 249 },
	{ "uacute", 6, 250, 250 },
	{ "ucirc", 5, 251, 251 },
	{ "uuml", 4, 252, 252 },
	{ "yacute", 6, 253, 253 },
	{ "thorn", 5, 254, 254 },
	{ "yuml", 4, 255, 255 },
#endif

	{ 0, 0, 0, 0 }
};

void	init_entity_table()
{
	static bool init = false;
	if (!init) {
#ifndef TRIM_NO_LINEDRAW
				// Need to initialized here since ACS_*
				// is not a compile time constant.
		entity_table[EID_BOXUR].code = isACSFallBack ? '+' : ACS_LLCORNER;
		entity_table[EID_BOXDR].code = isACSFallBack ? '+' : ACS_ULCORNER;
		entity_table[EID_BOXDL].code = isACSFallBack ? '+' : ACS_URCORNER;
		entity_table[EID_BOXUL].code = isACSFallBack ? '+' : ACS_LRCORNER;

		entity_table[EID_BOXVL].code = isACSFallBack ? '+' : ACS_RTEE;
		entity_table[EID_BOXVR].code = isACSFallBack ? '+' : ACS_LTEE;
		entity_table[EID_BOXHU].code = isACSFallBack ? '+' : ACS_BTEE;
		entity_table[EID_BOXHD].code = isACSFallBack ? '+' : ACS_TTEE;

		entity_table[EID_BOXH].code = isACSFallBack ? '-' : ACS_HLINE;
		entity_table[EID_BOXV].code = isACSFallBack ? '|' : ACS_VLINE;

		entity_table[EID_RARR].code = isACSFallBack ? '>' : ACS_RARROW;
		entity_table[EID_LARR].code = isACSFallBack ? '<' : ACS_LARROW;
		entity_table[EID_DARR].code = isACSFallBack ? 'v' : ACS_DARROW;
		entity_table[EID_UARR].code = isACSFallBack ? '^' : ACS_UARROW;

		entity_table[EID_DEGREE].code = isACSFallBack ? '\'' : ACS_DEGREE;
		entity_table[EID_PLMINUS].code = isACSFallBack ? '#' : ACS_PLMINUS;
		entity_table[EID_LEQUAL].code = isACSFallBack ? '<' : ACS_LEQUAL;
		entity_table[EID_GEQUAL].code = isACSFallBack ? '>' : ACS_GEQUAL;
		entity_table[EID_NEQUAL].code = isACSFallBack ? '!' : ACS_NEQUAL;

		entity_table[EID_BULLET].code = isACSFallBack ? 'o' : ACS_BULLET;
		entity_table[EID_DIAMOND].code = isACSFallBack ? '+' : ACS_DIAMOND;

		entity_table[EID_CKBOARD].code = isACSFallBack ? '#' : ACS_CKBOARD;
		entity_table[EID_BOARD].code = isACSFallBack ? '#' : ACS_BOARD;
		entity_table[EID_LANTERN].code = isACSFallBack ? '#' : ACS_LANTERN;
		entity_table[EID_BLOCK].code = isACSFallBack ? '#' : ACS_BLOCK;

		entity_table[EID_PI].code = isACSFallBack ? '*' : ACS_PI;
		entity_table[EID_STERLING].code = isACSFallBack ? 'f' : ACS_STERLING;

		entity_table[EID_S1].code = isACSFallBack ? '-' : ACS_S1;
		entity_table[EID_S9].code = isACSFallBack ? '-' : ACS_S9;
		entity_table[EID_S3].code = isACSFallBack ? '-' : ACS_S3;
		entity_table[EID_S7].code = isACSFallBack ? '-' : ACS_S7;
#endif
		init = true;
	}
}

chtype	get_entity(entity_id id)
{
	return entity_table[id].code;
}

wchar_t	get_wc_entity(entity_id id)
{
	return entity_table[id].wc;
}

void	draw_entity(WINDOW *win, entity_id id)
{
#ifdef USE_UTF8_MODE
	if (IsUTF8Mode()) {
		wchar_t wc = get_wc_entity(id);
		waddnwstr(win, &wc, 1);
	}
	else
#endif
		waddch(win, get_entity(id));
}

/*************************************************************************
	ACS fallback support
*************************************************************************/

bool	isACSFallBack = false;

/*************************************************************************
	Unicode output support
*************************************************************************/

// Return width
int	k_waddch_int(WINDOW *win, const char *&s, int max_width)
{
#ifdef USE_UTF8_MODE
	static mbstate_t state;

	if (IsUTF8Mode()) {
		wchar_t wc;
		memset(&state, 0, sizeof(mbstate_t));
		int ret = mbrtowc(&wc, s, MB_CUR_MAX, &state);
		if (ret <= 0)
			throw ErrorGenericSyntax(_("bad multibyte sequence %$"), s);

		int	width = wcwidth(wc);
		if (max_width < 0 || (max_width >= 0 && width <= max_width)) {

			int curColumn = getcurx(win);
			int curRow = getcury(win);

			if (width == 0 && curColumn) {
				cchar_t	cc;
				wmove(win, curRow, curColumn-1);
				win_wch(win, &cc);

				bool done = false;
				for (size_t i = 0; i < CCHARW_MAX; ++i) {
					if (!cc.chars[i]) {
						cc.chars[i] = wc;
						done = true;
						break;
					}
				}

				if (done) {
					wmove(win, curRow, curColumn-1);
					wadd_wch(win, &cc);
				}
				else {
					wmove(win, curRow, curColumn);
					waddnwstr(win, &wc, 1);
				}
			}
			else {
				waddnwstr(win, &wc, 1);
			}

					// Modify caller's s
			s += ret;
		}
		return width;
	}
	else
#endif
	     	if (max_width) {
			if (isprint(*s) || *s == ' ') {
				waddch(win, static_cast<unsigned char>(*s));
			}
			else {
				waddch(win, '?');
			}
					// Modify caller's s
			s++;
		}
	return 1;
}

// Return number of bytes consumed
int	k_waddch(WINDOW *win, const char *s, int max_width)
{
	const char *ss = s;
	k_waddch_int(win, ss, max_width);
	return ss-s;
}

// Return number of bytes consumed
int	k_waddstr(WINDOW *win, const char *s)
{
	const char *ss = s;
	while (*ss != 0) {
		k_waddch_int(win, ss);
	}
	return ss-s;
}

// Return number of bytes consumed
int	k_waddnstr(WINDOW *win, const char *s, int max_width)
{
	const char *ss = s;
	while (*ss != 0) {
		const char *save_ss = ss;
		int width = k_waddch_int(win, ss, max_width);
		if (ss == save_ss)	// Not enough space to print
			break;
		max_width -= width;
	}
	return ss-s;
}

/*************************************************************************
	Cursor state class
*************************************************************************/

void	CursorState::Hide()
{
	if (isSave)
		curs_set(0);
	else {				// Original state not saved yet
		oldCursor = curs_set(0);
		isSave = 1;
	}
}

void	CursorState::Show()
{
	if (isSave)
		curs_set(1);
	else {				// Original state not saved yet
		oldCursor = curs_set(1);
		isSave = 1;
	}
}

void	CursorState::Restore()
{
	if (isSave)
		curs_set(oldCursor);	// Restore original cursor state
}

/*************************************************************************
	ncurses screen setup and clean up class
*************************************************************************/

notifyFuncType	NCurses::notifySize = NULL;	// For window size notification
int	NCurses::isInitScr = 0;			// For ncurses initscr
int	NCurses::isInitScrMore = 0;		// Terminal modes, attributes
int	NCurses::isRedirect = 1;		// Start with redirect

void	NCurses::Init(int exitIfRedir)
{
	if (isInitScr == 0) {
		CheckTerminal(exitIfRedir);

		if (! IsRedirect()) {
			initscr();

			signal(SIGINT, EventSIGINT);
#ifdef HAVE_RESIZETERM
			signal(SIGWINCH, EventSIGWINCH);
#endif
#ifdef NCURSES_MOUSE_VERSION
			mmask_t oldMouseMask;
				// No REPORT_MOUSE_POSITION yet since 
				// ncurses does not support it
			mousemask(ALL_MOUSE_EVENTS, &oldMouseMask);
#endif
		}
		isInitScr = 1;
	}
}

void	NCurses::InitMore(WINDOW *win)
{
	if (IsRedirect())
		return;

	if (win)
		keypad(win, TRUE);

	if (isInitScrMore == 0) {
		isInitScrMore = 1;
		nonl();
		cbreak();
		noecho();
		typeahead(-1);
		raw();
		if (has_colors() == TRUE) {
			start_color();
		}
	}
	
	if (win)
		intrflush(win, FALSE);
}

void	NCurses::End()
{
	if (isInitScr) {
		SetNotify(NULL);	// No more size change notification
		
		isInitScr = 0;

		if (!IsRedirect()) {
			if (isInitScrMore) {
				// Opposite of ncurses start_color()
				putp(tigetstr("sgr0"));		// Exit attribute mode
				noraw();
				echo();
				nocbreak();
				nl();
			}
			endwin();
		}
	}
}

void	NCurses::CheckTerminal(int exitIfRedir)
{
				// Check if stdin and stdout are tty
	if (!isatty(fileno(stdout)) || !isatty(fileno(stdin))) {
		isRedirect = 1;
		if (exitIfRedir) {
			/* Pipe/Redirection detected for stdout */
			throw ErrorIORedirect();
		}
	}
	else
		isRedirect = 0;
}

void	NCurses::SetNotify(notifyFuncType func)
{
	notifySize = func;
}

void	NCurses::EventSIGINT(int /*sig*/)
{
	if (isInitScr) {
		if (! IsRedirect()) {
			clear();
			refresh();
		}

		// Don't throw exception here yet.  C library may not 
		// support it.
		NCurses::End();
	}
	exit(RET_OK);
}

void	NCurses::EventSIGWINCH(int /*sig*/)
{
	if (IsRedirect())
		return;
		
#ifdef HAVE_RESIZETERM
	// These stuffs is adapted from ncurses view.c test program
	struct winsize	size;
	if (notifySize && ioctl(fileno(stdout), TIOCGWINSZ, &size) ==0) {
		resizeterm(size.ws_row, size.ws_col);
		wrefresh(curscr); 		// For Linux
		(*notifySize)();
	}
	signal(SIGWINCH, EventSIGWINCH);	// Reinstall signal
#endif
}
