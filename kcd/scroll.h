/*
	scroll.h	Scroll Bar Header
	Copyright (c) 1997-8,2000 Kriang Lerdsuwanakij
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

#ifndef __K_SCROLL_H
#define __K_SCROLL_H

#include "config.h"
#include "scrnman.h"

/*************************************************************************
	Scroll bar base class
*************************************************************************/

class ScrollBarBase : public NCWindowBase {
	protected:
		int	barSize;
		int	visualSize;
		int	totalSize;
		int	curPos;
		
		int	beforeThumb, thumb, afterThumb;

		int	lastScrollDirection;
		int	lastScrollPosition;
		NCWindowBase	*controlledWin;

		virtual void	DrawScrollBar()	= 0;

		int	BeforeThumb()	{ return beforeThumb; }
		int	Thumb()		{ return thumb; }
		int	AfterThumb()	{ return afterThumb; }

		void	CalcSize();

	public:
		ScrollBarBase(NCScreenManager &scrnMan_, int id_);
		void	SetVisualSize(int visualSize_);
		void	SetTotalSize(int totalSize_);
		void	SetPosition(int curPos_);
		void	SetWin(NCWindowBase *win) { controlledWin = win; }
};

/*************************************************************************
	Scroll bar with left/right or up/down arrows at the ends
	of the bar
*************************************************************************/

class HScrollBar : public ScrollBarBase {
	protected:
		virtual void	DrawScrollBar();

		virtual void	DoResize();
		virtual void	DoUpdate();
	public:
		HScrollBar(NCScreenManager &scrnMan_, int id_);
		virtual void	Init();
};

class VScrollBar : public ScrollBarBase {
	protected:
		virtual void	DrawScrollBar();

		virtual void	DoResize();
		virtual void	DoUpdate();
	public:
		VScrollBar(NCScreenManager &scrnMan_, int id_);
		virtual void	Init();
};

/*************************************************************************
	Our own scroll bar
*************************************************************************/

class khHScrollBar : public HScrollBar {
	protected:
#ifdef NCURSES_MOUSE_VERSION
		virtual void	ProcessMouse(MEVENT &event);
#endif
	public:
		khHScrollBar(NCScreenManager &scrnMan_, int id_);
};

class khVScrollBar : public VScrollBar {
	protected:
#ifdef NCURSES_MOUSE_VERSION
		virtual void	ProcessMouse(MEVENT &event);
#endif
	public:
		khVScrollBar(NCScreenManager &scrnMan_, int id_);
};

/*************************************************************************
	Our tiny box between horizontal and vertical scroll bar
*************************************************************************/

class khScrollBox : public NCWindowBase {
	protected:
		virtual void	DrawBox();

		virtual void	DoResize();
		virtual void	DoUpdate();
		virtual void	DoRestCursor();
	public:
		khScrollBox(NCScreenManager &scrnMan_, int id_);
		virtual void	Init();
};

#endif	/* __K_SCROLL_H */
