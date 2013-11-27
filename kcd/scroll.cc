/*
	scroll.cc	Scroll Bar
	Copyright (c) 1997-8,2000,2002,2004,2007 Kriang Lerdsuwanakij
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

#include "scroll.h"
#include "strmisc.h"

/*************************************************************************
	Scroll bar base class
*************************************************************************/

void	ScrollBarBase::CalcSize()
{
					// Min value = 0 when curPos = 0,
					// No max limit (to be recalc
					// in the if block below.)
	beforeThumb = (curPos*barSize+totalSize/2)/totalSize;
					// Min value = 1, max = barSize
	thumb = Min(Max((visualSize*barSize+totalSize/2)/totalSize, 1),
		    barSize);
	afterThumb = barSize-beforeThumb-thumb;
	
	if (afterThumb < 0) {
		afterThumb = 0;
		beforeThumb = barSize-thumb;
	}
}

ScrollBarBase::ScrollBarBase(NCScreenManager &scrnMan_, int id_)
				: NCWindowBase(scrnMan_, id_)
{
	controlledWin = NULL;
	lastScrollDirection = 0;
	lastScrollPosition = 0;
	
	barSize = visualSize = totalSize = 1;
	curPos = 0;
	CalcSize();
}

void	ScrollBarBase::SetVisualSize(int visualSize_)
{
	visualSize = visualSize_;
	if (visualSize > totalSize)
		totalSize = visualSize;
	CalcSize();
	DrawScrollBar();
}

void	ScrollBarBase::SetTotalSize(int totalSize_)
{
	totalSize = totalSize_;
	if (totalSize < visualSize)
		visualSize = totalSize;
	CalcSize();
	DrawScrollBar();
}

void	ScrollBarBase::SetPosition(int curPos_)
{
	curPos = curPos_;
	if (curPos < 0)			// Position beyond lower limit
		curPos = 0;
					// Position beyond upper limit
	else if (curPos > totalSize-visualSize)
		curPos = totalSize-visualSize;
	CalcSize();
	DrawScrollBar();
}

/*************************************************************************
	Scroll bar with left/right or up/down arrows at the ends
	of the bar
*************************************************************************/

extern	entity_id	scrollBarID, scrollBlockID;
extern	attr_t	scrollArrowA, scrollBarA, scrollBlockA;

void	HScrollBar::Init()
{
	NCWindowBase::Init();
	
	if (GetRow() && GetCol()) {
		win = newwin(GetRow(), GetCol(), GetY(), GetX());
		if (win == NULL) {		// Error
			throw bad_alloc();
		}
	}
	else
		win = NULL;

	barSize = GetCol()-2;
	CalcSize();

	DrawScrollBar();
}

void	HScrollBar::DrawScrollBar()
{
	int	i;
	
	if (!win)
		return;
		
	wmove(win, 0, 0);
	wattrset(win, scrollArrowA);
	draw_entity(win, EID_LARR);

	wattrset(win, scrollBarA);
	for (i = 0; i < BeforeThumb(); i++)
		draw_entity(win, scrollBarID);

	wattrset(win, scrollBlockA);
	for (i = 0; i < Thumb(); i++)
		draw_entity(win, scrollBlockID);

	wattrset(win, scrollBarA);
	for (i = 0; i < AfterThumb(); i++)
		draw_entity(win, scrollBarID);

	wattrset(win, scrollArrowA);
	draw_entity(win, EID_RARR);
	wnoutrefresh(win);
}

void	HScrollBar::DoResize()
{
	if (win)
		delwin(win);
	win = NULL;
	Init();
}

void	HScrollBar::DoUpdate()
{
	if (win)
		wnoutrefresh(win);
}

HScrollBar::HScrollBar(NCScreenManager &scrnMan_, int id_)
			: ScrollBarBase(scrnMan_, id_)
{
}

void	VScrollBar::Init()
{
	NCWindowBase::Init();
	
	if (GetRow() && GetCol()) {
		win = newwin(GetRow(), GetCol(), GetY(), GetX());
		if (win == NULL) {		// Error
			throw bad_alloc();
		}
	}
	else
		win = NULL;

	barSize = GetRow()-2;
	CalcSize();

	DrawScrollBar();
}

void	VScrollBar::DrawScrollBar()
{
	int	i;
	int	j = 0;
	
	if (!win)
		return;
		
	wmove(win, 0, 0);
	wattrset(win, scrollArrowA);
	draw_entity(win, EID_UARR);
	j++;

	wattrset(win, scrollBarA);
	for (i = 0; i < BeforeThumb(); i++) {
		wmove(win, j, 0);
		draw_entity(win, scrollBarID);
		j++;
	}

	wattrset(win, scrollBlockA);
	for (i = 0; i < Thumb(); i++) {
		wmove(win, j, 0);
		draw_entity(win, scrollBlockID);
		j++;
	}

	wattrset(win, scrollBarA);
	for (i = 0; i < AfterThumb(); i++) {
		wmove(win, j, 0);
		draw_entity(win, scrollBarID);
		j++;
	}

	wmove(win, j, 0);
	wattrset(win, scrollArrowA);
	draw_entity(win, EID_DARR);
	wnoutrefresh(win);
}

void	VScrollBar::DoResize()
{
	if (win)
		delwin(win);
	win = NULL;
	Init();
}

void	VScrollBar::DoUpdate()
{
	if (win)
		wnoutrefresh(win);
}

VScrollBar::VScrollBar(NCScreenManager &scrnMan_, int id_)
			: ScrollBarBase(scrnMan_, id_)
{
}

/*************************************************************************
	Our own scroll bar
*************************************************************************/

#ifdef NCURSES_MOUSE_VERSION
void	khHScrollBar::ProcessMouse(MEVENT &event)
{
	if (!win)	// Window hiden
		return;
		
			// Only accepts mouse button 1
	if (!(event.bstate & (BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED
			      | BUTTON1_TRIPLE_CLICKED | BUTTON1_PRESSED)))
		return;

	if (event.bstate & BUTTON1_TRIPLE_CLICKED) {
		// Convert triple click to 3 separate clicks.
		event.bstate ^= BUTTON1_TRIPLE_CLICKED;
		event.bstate |= BUTTON1_CLICKED;
		ProcessMouse(event);
		scrnMan.RequestUpdate();
		ProcessMouse(event);
		scrnMan.RequestUpdate();
		ProcessMouse(event);
		scrnMan.RequestUpdate();
		return;
	}
	if (event.bstate & BUTTON1_DOUBLE_CLICKED) {
		// Convert double click to 2 separate clicks.
		event.bstate ^= BUTTON1_DOUBLE_CLICKED;
		event.bstate |= BUTTON1_CLICKED;
		ProcessMouse(event);
		scrnMan.RequestUpdate();
		ProcessMouse(event);
		scrnMan.RequestUpdate();
		return;
	}

	if (controlledWin) {
		if (event.x == GetX1()) {
			controlledWin->ProcessKey(KEY_SLEFT);
			lastScrollDirection = 0;
		}
		else if (event.x == GetX2()) {
			controlledWin->ProcessKey(KEY_SRIGHT);
			lastScrollDirection = 0;
		}
		else {
			if (event.x < GetX1()+1+BeforeThumb()) {
				controlledWin->ProcessKey(KEY_SLEFT);
				lastScrollPosition = event.x;
				lastScrollDirection = -1;
			}
			else if (event.x >= GetX1()+1+BeforeThumb()+Thumb()) {
				controlledWin->ProcessKey(KEY_SRIGHT);
				lastScrollPosition = event.x;
				lastScrollDirection = 1;
			}
			else if (lastScrollPosition == event.x) {
				if (lastScrollDirection == -1) {
					controlledWin->ProcessKey(KEY_SLEFT);
				}
				else if (lastScrollDirection == 1) {
					controlledWin->ProcessKey(KEY_SRIGHT);
				}
			}
		}
	}
}
#endif

khHScrollBar::khHScrollBar(NCScreenManager &scrnMan_, int id_)
			: HScrollBar(scrnMan_, id_)
{
	Init();
}

#ifdef NCURSES_MOUSE_VERSION
void	khVScrollBar::ProcessMouse(MEVENT &event)
{
	if (!win)	// Window hiden
		return;
		
			// Only accepts mouse button 1
	if (!(event.bstate & (BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED
			      | BUTTON1_TRIPLE_CLICKED | BUTTON1_PRESSED)))
		return;

	if (event.bstate & BUTTON1_TRIPLE_CLICKED) {
		// Convert triple click to 3 separate clicks.
		event.bstate ^= BUTTON1_TRIPLE_CLICKED;
		event.bstate |= BUTTON1_CLICKED;
		ProcessMouse(event);
		scrnMan.RequestUpdate();
		ProcessMouse(event);
		scrnMan.RequestUpdate();
		ProcessMouse(event);
		scrnMan.RequestUpdate();
		return;
	}
	if (event.bstate & BUTTON1_DOUBLE_CLICKED) {
		// Convert double click to 2 separate clicks.
		event.bstate ^= BUTTON1_DOUBLE_CLICKED;
		event.bstate |= BUTTON1_CLICKED;
		ProcessMouse(event);
		scrnMan.RequestUpdate();
		ProcessMouse(event);
		scrnMan.RequestUpdate();
		return;
	}

	if (controlledWin) {
		if (event.y == GetY1()) {	// Scroll reverse
			controlledWin->ProcessKey(KEY_SR);
			lastScrollDirection = 0;
		}
		else if (event.y == GetY2()) {	// Scroll forward
			controlledWin->ProcessKey(KEY_SF);
			lastScrollDirection = 0;
		}
		else {
			if (event.y < GetY1()+1+BeforeThumb()) {
				controlledWin->ProcessKey(KEY_PPAGE);
				lastScrollPosition = event.y;
				lastScrollDirection = -1;
			}
			else if (event.y >= GetY1()+1+BeforeThumb()+Thumb()) {
				controlledWin->ProcessKey(KEY_NPAGE);
				lastScrollPosition = event.y;
				lastScrollDirection = 1;
			}
			else if (lastScrollPosition == event.y) {
				if (lastScrollDirection == -1) {
					controlledWin->ProcessKey(KEY_PPAGE);
				}
				else if (lastScrollDirection == 1) {
					controlledWin->ProcessKey(KEY_NPAGE);
				}
			}
		}
	}
}
#endif

khVScrollBar::khVScrollBar(NCScreenManager &scrnMan_, int id_)
			: VScrollBar(scrnMan_, id_)
{
	Init();
}

/*************************************************************************
	Our tiny box between horizontal and vertical scroll bar
*************************************************************************/

void	khScrollBox::Init()
{
	NCWindowBase::Init();
	
	if (GetRow() && GetCol()) {
		win = newwin(GetRow(), GetCol(), GetY(), GetX());
		if (win == NULL) {		// Error
			throw bad_alloc();
		}
	}
	else
		win = NULL;
	
	NCurses::InitMore(win);		// Set keyboard mode
					// Required since this is the 
					// only window that stays
					// permanently even when screen
					// size changes

	DrawBox();
}

void	khScrollBox::DrawBox()
{
	if (win) {
		wattrset(win, scrollArrowA);
		waddch(win, ' ');
	}
}

void	khScrollBox::DoResize()
{
	InitGeometry();			// Compute new location
	if (win)
		mvwin(win, GetY(), GetX());	// Move there
}

void	khScrollBox::DoUpdate()
{
	if (win)
		wnoutrefresh(win);
}

void	khScrollBox::DoRestCursor()
{
	if (win) {
		wmove(win, 0, 0);
		wnoutrefresh(win);
	}
}

khScrollBox::khScrollBox(NCScreenManager &scrnMan_, int id_)
			: NCWindowBase(scrnMan_, id_)
{
	Init();
}

