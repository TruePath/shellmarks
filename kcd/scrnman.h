/*
	scrnman.h	Screen Manager Header
	Copyright (c) 1997-8,2000,2004,2007 Kriang Lerdsuwanakij
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

#ifndef __K_SCRNMAN_H
#define __K_SCRNMAN_H

#include "config.h"

#include "setupcurses.h"

#include "list.h"
#include "miscobj.h"

/*************************************************************************
	Screen geometry manager base class
*************************************************************************/

class NCGeometryManagerBase {
	protected:
		int			mode;
	public:
		NCGeometryManagerBase(int mode_) : mode(mode_) {}

		void	SetMode(int mode_)	{ mode = mode_; }
		int	GetMode()		{ return mode; }
		
		virtual void	SetWinRect(int id, NCRect &rect) = 0;
		virtual ~NCGeometryManagerBase() {}
};

/*************************************************************************
	Screen manager base class
*************************************************************************/
class NCWindowBase;

class NCScreenManager {
	friend	class NCWindowBase;	// Allow access to 
					// RequestAddWindow(...)
					// and RequestDeleteWindow(...)

	protected:
		NCGeometryManagerBase	&geoMan;
		NCWindowBase		*focusWin, *cursorWin;
		NCWindowBase		*permWin;
		NCWindowBase		*ctrlWin;
		sptr_list<NCWindowBase>	winList, focusStack, cursorStack;

	private:
			// Called by NCWindowBase::Init()

		void	RequestAddWindow(NCWindowBase *win);
		void	RequestDeleteWindow(NCWindowBase *win);

	public:
#ifdef NCURSES_MOUSE_VERSION
		MEVENT	mouseEvent;
#endif
			// Anyone can call these to request screen update, etc.
	
		virtual void	RequestResize();	// Resize all windows
		void	RequestRefresh();	// Clear screen + update all windows
		void	RequestUpdate();	// Update all windows
		void	RequestRestCursor();
		void	RequestUpdateAndRestCursor();
		void	RequestChangeMode(int mode);
		
		void	RequestWinRect(int id, NCRect &rect) { geoMan.SetWinRect(id, rect); }

			// Set window to receive keyboard and cursor control

		void	SetFocus(NCWindowBase *win);	// Set win = NULL to
							// return control to 
							// previous one
		void	SetCursor(NCWindowBase *win);	// Set win = NULL to
							// return control to 
							// previous one
							
		void	SetPermWin(NCWindowBase *win) { permWin = win; }
							// Set win that will
							// stay on screen
							// permanently

	private:

			// No definition for this !!!

		NCScreenManager(const NCScreenManager &);

			// Window size change notifier, used to support any 
			// derived class of NCScreenManager

		class	SizeChangeNotify {
			protected:
				static NCScreenManager	*notifyObject;
			public:
				static void	Callback();
				static void	InstallObject(NCScreenManager *obj);
		};

		static unsigned mouseScrollRate;

	public:
		NCScreenManager(NCGeometryManagerBase &geoMan_);
		virtual	~NCScreenManager();
		virtual int	ReadKeyboard();
		virtual void	KeyboardLoop();
		virtual void	DoRestCursor();

		void		SetCtrlWindow(NCWindowBase *ctrlWin_) { ctrlWin = ctrlWin_; }
		static void	SetMouseScrollRate(unsigned rate) { mouseScrollRate = rate; }
};

/*************************************************************************
	Window base class
*************************************************************************/

class NCWindowBase {
	friend	class	NCScreenManager;
	
	protected:
		int		id;
	
				// Init by derived class
		WINDOW		*win;
		
				// To be set by InitGeometry()
		NCRect		rect;
		
		
		NCScreenManager	&scrnMan;
		CursorState	cursor;

	public:
		virtual void	DoTouch();
		virtual void	DoResize() = 0;
		virtual void	DoUpdate() = 0;
		virtual void	DoRestCursor();
		virtual void	ProcessCtrl(char /*c*/, bool /*set*/);
		virtual void	ProcessKey(int /*ch*/);
#ifdef NCURSES_MOUSE_VERSION
		virtual void	ProcessMouse(MEVENT & /*event*/);
#endif

	protected:
				// Inline functions to
				// simplify window sizing

		void	SetX1(int x1)	{ rect.SetX1(x1); }
		void	SetY1(int y1)	{ rect.SetY1(y1); }
		void	SetX2(int x2)	{ rect.SetX2(x2); }
		void	SetY2(int y2)	{ rect.SetY2(y2); }
		void	SetX(int x)	{ rect.SetX(x); }
		void	SetY(int y)	{ rect.SetY(y); }
		void	SetRow(int row)	{ rect.SetRow(row); }
		void	SetCol(int col)	{ rect.SetCol(col); }

				// Used by Init();
				
		void	InitGeometry();
	public:
				// Inline functions to
				// simplify window size request
				
		int	GetX1()		{ return rect.GetX1(); }
		int	GetY1()		{ return rect.GetY1(); }
		int	GetX2()		{ return rect.GetX2(); }
		int	GetY2()		{ return rect.GetY2(); }
		int	GetX()		{ return rect.GetX(); }
		int	GetY()		{ return rect.GetY(); }
		int	GetRow()	{ return rect.GetRow(); }
		int	GetCol()	{ return rect.GetCol(); }

		bool	IsInWin(int x, int y);
	
		NCWindowBase(NCScreenManager &scrnMan_, int id_);
		virtual	~NCWindowBase();
		virtual void	Init() = 0;
		virtual void	KeyboardLoop();
		
	private:
			// No definition for these

		NCWindowBase(const NCWindowBase &);
		NCWindowBase& operator=(const NCWindowBase &);
		NCWindowBase* operator()();
		const NCWindowBase* operator()() const;

		bool	registered;
};

#endif	/* __K_SCRNMAN_H */
