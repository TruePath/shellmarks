/*
	scrnman.cc	Screen Manager
	Copyright (c) 1997-8,2000,2004 Kriang Lerdsuwanakij
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

#include "scrnman.h"

/*************************************************************************
	Screen manager base class
*************************************************************************/

void	NCScreenManager::RequestAddWindow(NCWindowBase *win)
{
	winList.push_back(win);
}

void	NCScreenManager::RequestDeleteWindow(NCWindowBase *win)
{
	for (sptr_list<NCWindowBase>::iterator iter = winList.begin(); 
	     iter != winList.end(); ++iter) {
		if ((*iter)() == win) {
			(*iter).release();	// Since this is called from
						// the destructor, release
						// the ref count to avoid
						// calling the same destructor
						// again.
			winList.erase(iter);
			break;
		}
	}
}

void	NCScreenManager::RequestResize()
{
	for (sptr_list<NCWindowBase>::iterator iter = winList.begin(); 
	     iter != winList.end(); ++iter) {
		(*iter)->DoResize();
	}
	RequestRefresh();
}

void	NCScreenManager::RequestRefresh()
{
	for (sptr_list<NCWindowBase>::iterator iter = winList.begin(); 
	     iter != winList.end(); ++iter) {
		(*iter)->DoTouch();
	}
	clearok(curscr, TRUE);
	wrefresh(stdscr);
	RequestUpdateAndRestCursor();	// Redraw all windows
}

void	NCScreenManager::RequestUpdate()
{
	for (sptr_list<NCWindowBase>::iterator iter = winList.begin(); 
	     iter != winList.end(); ++iter) {
		(*iter)->DoUpdate();
	}
	doupdate();			// Transfer to terminal
}

void	NCScreenManager::RequestRestCursor()
{
	if (cursorWin)
		cursorWin->DoRestCursor();
	else
		DoRestCursor();
	doupdate();			// Change cursor position
}

void	NCScreenManager::RequestUpdateAndRestCursor()
{
	for (sptr_list<NCWindowBase>::iterator iter = winList.begin(); 
	     iter != winList.end(); ++iter) {
		(*iter)->DoUpdate();
	}
	RequestRestCursor();
}

void	NCScreenManager::RequestChangeMode(int mode)
{
				// Changing sizes for all window is very
				// costly, make sure we really need it
				
	if (mode != geoMan.GetMode()) {		// Change required
		geoMan.SetMode(mode);
		RequestResize();		// Notify all window
	}
}

void	NCScreenManager::SetFocus(NCWindowBase *win)
{
	if (win) {
		focusStack.push_back(focusWin);
		focusWin = win;
	}
	else {
		if (focusStack.size()) {
						// Release ref count
			focusWin = focusStack.back().release();
			focusStack.pop_back();
		}
		else
			focusWin = 0;
	}
}

void	NCScreenManager::SetCursor(NCWindowBase *win)
{
	if (win) {
		cursorStack.push_back(cursorWin);
		cursorWin = win;
	}
	else {
		if (cursorStack.size()) {
						// Release ref count
			cursorWin = cursorStack.back().release();
			cursorStack.pop_back();
		}
		else
			cursorWin = 0;
	}
}

NCScreenManager::NCScreenManager(NCGeometryManagerBase &geoMan_) : 
				geoMan(geoMan_),
				focusWin(NULL), cursorWin(NULL),
				permWin(NULL), ctrlWin(NULL),
				winList(), focusStack(), 
				cursorStack()
{
	SizeChangeNotify::InstallObject(this);
	NCurses::SetNotify(SizeChangeNotify::Callback);
}

unsigned NCScreenManager::mouseScrollRate = 2;

NCScreenManager::~NCScreenManager()
{
}

int	NCScreenManager::ReadKeyboard()
{
	int	c;
	if (permWin)
		c = wgetch(permWin->win);
	else
		c = wgetch(focusWin->win);

#ifdef NCURSES_MOUSE_VERSION
	if (c == KEY_MOUSE)
		getmouse(&mouseEvent);
#endif
	return c;		
}

void	NCScreenManager::KeyboardLoop()
{
	NCWindowBase *prevMousePressedWin = 0;
	MEVENT prevMouseEvent;
#ifdef NCURSES_MOUSE_VERSION
	if (mouseScrollRate > 0)
		halfdelay(mouseScrollRate);
#endif
	for ( ; ; ) {
		if (focusWin) {
			int	c = ReadKeyboard();
#ifdef NCURSES_MOUSE_VERSION
			if (c == KEY_MOUSE) {
				// FIXME: Locate top window in case of overlapping windows
				for (sptr_list<NCWindowBase>::iterator iter = winList.begin(); 
				     iter != winList.end(); ++iter) {
					(*iter)->DoTouch();
					if ((*iter)->IsInWin(mouseEvent.x, mouseEvent.y)) {
						if (mouseEvent.bstate
						    & (BUTTON1_PRESSED | BUTTON2_PRESSED
						       | BUTTON3_PRESSED | BUTTON4_PRESSED)) {
							if (prevMousePressedWin) {
								// Replace BUTTON?_PRESSED with
								// BUTTON?_RELEASED
								if (prevMouseEvent.bstate & BUTTON1_PRESSED) {
									prevMouseEvent.bstate &= ~BUTTON1_PRESSED;
									prevMouseEvent.bstate |= BUTTON1_RELEASED;
								}
								if (prevMouseEvent.bstate & BUTTON2_PRESSED) {
									prevMouseEvent.bstate &= ~BUTTON2_PRESSED;
									prevMouseEvent.bstate |= BUTTON2_RELEASED;
								}
								if (prevMouseEvent.bstate & BUTTON3_PRESSED) {
									prevMouseEvent.bstate &= ~BUTTON3_PRESSED;
									prevMouseEvent.bstate |= BUTTON3_RELEASED;
								}
								if (prevMouseEvent.bstate & BUTTON4_PRESSED) {
									prevMouseEvent.bstate &= ~BUTTON4_PRESSED;
									prevMouseEvent.bstate |= BUTTON4_RELEASED;
								}
								prevMousePressedWin->ProcessMouse(prevMouseEvent);
							}
							prevMousePressedWin = (*iter)();
							prevMouseEvent = mouseEvent;
						}

						(*iter)->ProcessMouse(mouseEvent);

						if (mouseEvent.bstate
						    & (BUTTON1_RELEASED | BUTTON2_RELEASED
						       | BUTTON3_RELEASED | BUTTON4_RELEASED))
							prevMousePressedWin = 0;
						break;
					}
				}
			}
			else if (c == ERR) {
				if (prevMousePressedWin)
					prevMousePressedWin->ProcessMouse(prevMouseEvent);
			}
			else {
#endif
				prevMousePressedWin = 0;
				focusWin->ProcessKey(c);
#ifdef NCURSES_MOUSE_VERSION
			}
#endif
		}
		else
			break;		// Exit loop if focusWin is NULL
	}
}

void	NCScreenManager::DoRestCursor()
{
}

NCScreenManager	*NCScreenManager::SizeChangeNotify::notifyObject = NULL;

void	NCScreenManager::SizeChangeNotify::Callback()
{
	if (notifyObject)
		notifyObject->RequestResize();
}

void	NCScreenManager::SizeChangeNotify::InstallObject(NCScreenManager *obj)
{
	notifyObject = obj;
}

/*************************************************************************
	Window base class
*************************************************************************/

void	NCWindowBase::DoTouch()
{
	if (win)
		touchwin(win);
}

void	NCWindowBase::DoRestCursor()
{
}

void	NCWindowBase::ProcessCtrl(char /*c*/, bool /*set*/)
{
}

void	NCWindowBase::ProcessKey(int /*ch*/)
{
}

#ifdef NCURSES_MOUSE_VERSION
void	NCWindowBase::ProcessMouse(MEVENT & /*event*/)
{
}
#endif

bool	NCWindowBase::IsInWin(int x, int y)
{
	return (x >= GetX1() && x <= GetX2() && y >= GetY1() && y <= GetY2());
}

void	NCWindowBase::InitGeometry()
{
	scrnMan.RequestWinRect(id, rect);
}

void	NCWindowBase::Init()
{
	InitGeometry();
	if (!registered) {
		scrnMan.RequestAddWindow(this);
		registered = true;
	}
}

NCWindowBase::NCWindowBase(NCScreenManager &scrnMan_, int id_) : id(id_),
							win(NULL), rect(),
							scrnMan(scrnMan_),
							registered(false)
{
}

NCWindowBase::~NCWindowBase()
{
	if (registered)
		scrnMan.RequestDeleteWindow(this);
	if (win)
		delwin(win);
}

void	NCWindowBase::KeyboardLoop()
{
	scrnMan.KeyboardLoop();
}
