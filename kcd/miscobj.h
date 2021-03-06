/*
	miscobj.h	Various NCurses C++ Classes Header
	Copyright (c) 1996-8,2000,2007 Kriang Lerdsuwanakij
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

#ifndef __K_MISCOBJ_H
#define __K_MISCOBJ_H

#include "config.h"
#include "setupcurses.h"
#include "cxxlib.h"

USING_NAMESPACE_STD;

/*************************************************************************
	ACS fallback support
*************************************************************************/

enum	entity_id {
	EID_LT = 0, EID_GT, EID_AMP, EID_QUOT,
	EID_BOXUR, EID_BOXDR, EID_BOXDL, EID_BOXUL,
	EID_BOXVL, EID_BOXVR, EID_BOXHU, EID_BOXHD, EID_BOXH, EID_BOXV,
	EID_RARR, EID_LARR, EID_DARR, EID_UARR, 
	EID_DEGREE, EID_PLMINUS, EID_LEQUAL, EID_GEQUAL, EID_NEQUAL,
	EID_BULLET, EID_DIAMOND, 
	EID_CKBOARD, EID_BOARD, EID_LANTERN, EID_BLOCK,
	EID_PI, EID_STERLING, EID_S1, EID_S9, EID_S3, EID_S7, 
	EID_SPACE };

struct	entity_info {
	const char	*name;
	size_t		length;
	chtype		code;
	wchar_t		wc;
};

extern entity_info entity_table[];

void	init_entity_table();
chtype	get_entity(entity_id id);
wchar_t	get_wc_entity(entity_id id);

void	draw_entity(WINDOW *win, entity_id id);

extern bool	isACSFallBack;

/*************************************************************************
	Unicode output support
*************************************************************************/

int	k_waddch_int(WINDOW *win, const char *&s, int max_width = -1);
int	k_waddch(WINDOW *win, const char *s, int max_width = -1);
int	k_waddstr(WINDOW *win, const char *s);
int	k_waddnstr(WINDOW *win, const char *s, int max_width);

inline int	k_waddstr(WINDOW *win, const string &s)
{
	return k_waddstr(win, s.c_str());
}

inline int	k_waddnstr(WINDOW *win, const string &s, int max_width)
{
	return k_waddnstr(win, s.c_str(), max_width);
}

/*************************************************************************
	HTML anchor class
*************************************************************************/

class Anchor {
	public:
		string	name;
		string	linkText;	
		int	rowFrom, colFrom, rowTo, colTo;
		int	type;

		Anchor(const char *str) : name(str), linkText("") {}
		Anchor(const char *str, int len) : name(str, len), linkText("") {}
		Anchor(const string &str) : name(str), linkText("") {}
		Anchor(const char *str, int len, int type_,
			int rowFrom_ = -1, int colFrom_ = -1,
			int rowTo_ = -1, int colTo_ = -1)
				: name(str, len), linkText(""),
				  rowFrom(rowFrom_), colFrom(colFrom_), 
		  		  rowTo(rowTo_), colTo(colTo_), type(type_) {}
		~Anchor() {}
};

/*************************************************************************
	Rectangle class
*************************************************************************/

class	NCRect {
	private:
		int	x1, y1, x2, y2, row, col;
	public:
		int	GetX1()		{ return x1; }
		int	GetY1()		{ return y1; }
		int	GetX2()		{ return x2; }
		int	GetY2()		{ return y2; }
		int	GetX()		{ return x1; }
		int	GetY()		{ return y1; }
		int	GetRow()	{ return row; }
		int	GetCol()	{ return col; }

		void	SetX1(int x1_)	{ x1 = x1_; col = x2-x1+1; }
		void	SetY1(int y1_)	{ y1 = y1_; row = y2-y1+1; }
		void	SetX2(int x2_)	{ x2 = x2_; col = x2-x1+1; }
		void	SetY2(int y2_)	{ y2 = y2_; row = y2-y1+1; }
		void	SetX(int x_)	{ x1 = x_; col = x2-x1+1; }
		void	SetY(int y_)	{ y1 = y_; row = y2-y1+1; }
		void	SetRow(int row_){ row = row_; y2 = y1+row-1; }
		void	SetCol(int col_){ col = col_; x2 = x1+col-1; }

		NCRect()		{ x1 = y1 = x2 = y2 = row = col = 0; }
		NCRect(int x1_, int y1_, int x2_, int y2_)
				{ x1 = x1_; y1 = y1_; x2 = x2_; y2 = y2_;
				  row = y2-y1+1; col = x2-x1+1; }
				  
		// NCRect(const NCRect &) automatically generated by compiler
};

inline int	IsRectXIntersect(NCRect &rect1, NCRect &rect2)
{
	return Max(rect1.GetX1(), rect2.GetX2()) <= Min(rect1.GetX1(), rect2.GetX2());
}

inline int	IsRectYIntersect(NCRect &rect1, NCRect &rect2)
{
	return Max(rect1.GetY1(), rect2.GetY2()) <= Min(rect1.GetY1(), rect2.GetY2());
}

inline int	IsRectIntersect(NCRect &rect1, NCRect &rect2)
{
	return IsRectXIntersect(rect1, rect2) && IsRectYIntersect(rect1, rect2);
}

/*************************************************************************
	Cursor state class
*************************************************************************/

class	CursorState {
	private:
		int	isSave;
		int	oldCursor;
	public:
		CursorState()	{ isSave = 0; }
		void	Hide();
		void	Show();
		void	Restore();
};

/*************************************************************************
	ncurses screen setup and clean up class
*************************************************************************/

typedef void (*notifyFuncType)();

class	NCurses {
	protected:
		static int	isInitScr;
		static int	isInitScrMore;

		static int	isRedirect;

		static notifyFuncType	notifySize;

		static void	CheckTerminal(int exitIfRedir = 1);
		static void	EventSIGINT(int /*sig*/);
		static void	EventSIGWINCH(int /*sig*/);
		
	public:
		static void	Init(int exitIfRedir = 1);
		static void	InitMore(WINDOW *win);
		static void	SetNotify(notifyFuncType func);
		static void	End();
		static int	IsRedirect() { return isRedirect; }
		static int	IsInitScr() { return isInitScr; }
};

#endif	/* __K_MISCOBJ_H */
