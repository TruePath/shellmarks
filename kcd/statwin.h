/*
	statwin.h	Status Window Header
	Copyright (c) 1997-8,2000,2003,2007 Kriang Lerdsuwanakij
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

#ifndef __K_STATWIN_H
#define __K_STATWIN_H

#include "config.h"

#include "scrnman.h"
#include "editbox.h"
#include "khdoc.h"		/* For attributes */

USING_NAMESPACE_STD;

/*************************************************************************
	Status window base class
*************************************************************************/

class StatusWindowBase : public NCWindowBase {
	protected:
		size_t	statusNo;
	public:
		virtual size_t	MaxStatus() = 0;
		virtual void	SetTitle(const char *title_) = 0;
		virtual void	SetTitle(const string &title_) = 0;
		virtual void	NextStatus() = 0;
		virtual void	SetCount(size_t /*count_*/) {}
		StatusWindowBase(NCScreenManager &scrnMan_, int id_);
};


/*************************************************************************
	kcd/khyper Status window class
*************************************************************************/

class khStatusWindow : public StatusWindowBase {
	protected:
		string		title;
		string		edit_string;
		string		prev_edit_string;
		bool		find_backward;
		
	public:
		virtual void	DrawStatusBar(int num = -1);
		virtual void	DoResize();
		virtual void	DoUpdate();
		
		virtual size_t	MaxStatus() { return 1; }
		virtual void	SetTitle(const char *title_);
		virtual void	SetTitle(const string &title_);
		virtual void	NextStatus();
		khStatusWindow(NCScreenManager &scrnMan_, int id_);
		virtual ~khStatusWindow() {}
		virtual void	Init();
};

/*************************************************************************
	kcd/khyper URL window
*************************************************************************/

class khURLWindow : public StatusWindowBase {
	protected:
		enum { DISP_TITLE = 0, DISP_KEYS1, DISP_KEYS2, DISP_KEYS3,
			DISP_NUMDISPLAY };
		string		title;
		string		switch_disp;
		size_t		switch_disp_width;

		char	*statKey1;
		char	*statKey2;
		char	*statKey3;

#ifdef NCURSES_MOUSE_VERSION
		virtual void	ProcessMouse(MEVENT &event);
#endif
	public:
		virtual void	DrawStatusBar(int num = -1);
		virtual void	DoResize();
		virtual void	DoUpdate();
		
		virtual size_t	MaxStatus() { return DISP_NUMDISPLAY; }
		virtual void	SetTitle(const char *title_);
		virtual void	SetTitle(const string &title_);
		virtual void	NextStatus();
		khURLWindow(NCScreenManager &scrnMan_, int id_);
		virtual ~khURLWindow() {}
		virtual void	Init();
};

/*************************************************************************
	kcd/khyper URL window with find key help
*************************************************************************/

class khURLWindowWithFind : public khURLWindow {
	protected:
		enum { DISP_KEYS4FIND = khURLWindow::DISP_NUMDISPLAY,
			DISP_KEYSFIND,
			DISP_NUMDISPLAYFIND };
		char	*statKey4WithFind;
		char	*statKeyFind;
		char	*statKeyCtrlF;

	public:
		virtual size_t	MaxStatus() { return DISP_NUMDISPLAYFIND; }
		void	ProcessCtrl(char c, bool s);
		virtual void	DrawStatusBar(int num = -1);
		khURLWindowWithFind(NCScreenManager &scrnMan_, int id_);
};

/*************************************************************************
	kcd/khyper Status window with find class
*************************************************************************/

class SomethingWithFind;

class khStatusWindowWithFind : public khStatusWindow {
	public:
		static const int	MAX_SEARCHCHAR = 15;
	protected:
		EditBox		*editBox;
		int		statusSearch;
		size_t		count;
		char		searchStr[MAX_SEARCHCHAR+1];
		SomethingWithFind	*findText;
		
		static const char	*statFind1;
		static const char	*statFind2;
		static const char	*statFindOvr;
		static const char	*statFindIns;
		
	public:
		static	int	spaceSelect;

		virtual void	DrawStatusBar(int num = -1);
		virtual void	DoRestCursor();
		virtual void	ProcessKey(int ch);
		void		SetCount(size_t count_) { count = count_; }
		khStatusWindowWithFind(NCScreenManager &scrnMan_, int id_,
					SomethingWithFind *findText_);
		virtual ~khStatusWindowWithFind();
		void	SetFindWindow(SomethingWithFind *findText_);
};

/*************************************************************************
	Marker bar class
*************************************************************************/

class khMarkerBar : public NCWindowBase {
	public:
		virtual void	DoResize();
		virtual void	DoUpdate();
		virtual void	Init();
		
		khMarkerBar(NCScreenManager &scrnMan_, int id_);

		void	Mark(int row, chtype c);
};

#endif	/* __K_STATWIN_H */
