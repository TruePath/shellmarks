/*
	statwin.cc	Status Window
	Copyright (c) 1997-8,2000,2003,2004,2007,2009 Kriang Lerdsuwanakij
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

#include "statwin.h"
#include "khwin.h"
#include "cstrlib.h"
#include "strmisc.h"

/*************************************************************************
	Helper functions
*************************************************************************/

static string	dots;
static size_t	dots_width = 0;

void	DrawStatusBarHelper(WINDOW *win, int num_col, int hide_size, const char *str)
{
	if (dots_width == 0) {
		dots = _("...");
		dots_width = GetStringWidth(dots);
	}

	if (num_col > hide_size) {
		wmove(win, 0, 2);
					// Unicode supported in k_wadd*
		if (static_cast<int>(GetStringWidth(str)) <= num_col-2)
			k_waddnstr(win,  str, num_col-2);
		else {
			k_waddnstr(win,  str, num_col-2-dots_width);
			k_waddstr(win,  dots);
		}
	}
}

void	DrawStatusBarHelper(WINDOW *win, int num_col, int hide_size, const string &str)
{
	DrawStatusBarHelper(win, num_col, hide_size, str.c_str());
}

/*************************************************************************
	Status window base class
*************************************************************************/

void	StatusWindowBase::NextStatus()
{
	statusNo = (statusNo+1)%MaxStatus();
}

StatusWindowBase::StatusWindowBase(NCScreenManager &scrnMan_, int id_) : 
			NCWindowBase(scrnMan_, id_), statusNo(0)
{
}


/*************************************************************************
	kcd/khyper Status window class
*************************************************************************/

void	khStatusWindow::Init()
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

	find_backward = false;

	NCurses::InitMore(win);
	DrawStatusBar();
}

void	khStatusWindow::DoResize()
{
	delwin(win);
	win = NULL;
	Init();	
}

void	khStatusWindow::DoUpdate()
{
	wnoutrefresh(win);
}

void	khStatusWindow::SetTitle(const char *title_)
{
	title = title_;
	DrawStatusBar();
}

void	khStatusWindow::SetTitle(const string &title_)
{
	title = title_;
	DrawStatusBar();
}

void	khStatusWindow::NextStatus()
{
	StatusWindowBase::NextStatus();
	DrawStatusBar();
}

void	khStatusWindow::DrawStatusBar(int num)
{
	if (num == -1)
		num = statusNo;
	
	wmove(win, 0, 0);
	wattrset(win, barA);		/* Clear bar */
	for (int i = 0; i < GetCol(); i++)
		waddch(win, ' ');

	if (!title.empty()) {
		DrawStatusBarHelper(win, GetCol(), 20, title);
	}
	else {
		string	s = progName;
		s += ' ';
		s += version;
		DrawStatusBarHelper(win, GetCol(), 20, s);
	}

	wnoutrefresh(win);
	scrnMan.RequestRestCursor();
}

khStatusWindow::khStatusWindow(NCScreenManager &scrnMan_, int id_) :
			StatusWindowBase(scrnMan_, id_),
			title("")
{
	Init();
}

/*************************************************************************
	kcd/khyper URL window
*************************************************************************/

void	khURLWindow::Init()
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

	NCurses::InitMore(win);
	DrawStatusBar();
}

void	khURLWindow::DoResize()
{
	delwin(win);
	win = NULL;
	Init();	
}

void	khURLWindow::DoUpdate()
{
	wnoutrefresh(win);
}

void	khURLWindow::SetTitle(const char *title_)
{
	title = title_;
	DrawStatusBar();
}

void	khURLWindow::SetTitle(const string &title_)
{
	title = title_;
	DrawStatusBar();
}

void	khURLWindow::NextStatus()
{
	StatusWindowBase::NextStatus();
	DrawStatusBar();
}

#ifdef NCURSES_MOUSE_VERSION
void	khURLWindow::ProcessMouse(MEVENT &event)
{
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

	NextStatus();
}
#endif

void	khURLWindow::DrawStatusBar(int num)
{
	if (num == -1)
		num = statusNo;

	wmove(win, 0, 0);
	wattrset(win, urlA);		/* Clear bar */
	for (int i = 0; i < GetCol(); i++)
		waddch(win, ' ');

	switch(num) {
		case DISP_TITLE:
			if (GetCol() > 40) {
				wmove(win, 0, 2);
				if (!title.empty()) {
					DrawStatusBarHelper(win, GetCol()-switch_disp_width-3, 20, title);
				}
				else {
					string	s = progName;
					s += ' ';
					s += version;
					DrawStatusBarHelper(win, GetCol()-switch_disp_width-3, 20, s);
				}
			}
			if (GetCol() > 20) {
				wmove(win, 0, GetCol()-switch_disp_width-2);
				k_waddstr(win, switch_disp);
			}
			break;
		case DISP_KEYS1:
			DrawStatusBarHelper(win, GetCol(), 20, _(statKey1));
			break;
		case DISP_KEYS2:
			DrawStatusBarHelper(win, GetCol(), 20, _(statKey2));
			break;
		case DISP_KEYS3:
			DrawStatusBarHelper(win, GetCol(), 20, _(statKey3));
			break;
	}
	wnoutrefresh(win);
	scrnMan.RequestRestCursor();
}

khURLWindow::khURLWindow(NCScreenManager &scrnMan_, int id_) :
			StatusWindowBase(scrnMan_, id_),
			title("")
{
	switch_disp = _("F9=Switch display");
	switch_disp_width = GetStringWidth(switch_disp);

	switch (kcdConfig.cfgKey) {
		case keyBindingDefault:
			statKey1 = _("Keys (1):  Arrows,PgUp,PgDn,Home,End=Move cursor"
				     "  Enter,Space=Select");
			statKey2 = _("Keys (2):  ^R,F8=Repaint screen  F9=Switch display"
				     "  ^C,F10=Quit");
			statKey3 = _("Keys (3):  Keypad Center=Center highlight"
				     "  `,\',`.\'=Pan left/right");
			break;
		case keyBindingVi:
			statKey1 = _("Keys (1v):  Arrows,PgUp,PgDn,Home,End=Move cursor"
				     "  Enter,Space=Select");
			statKey2 = _("Keys (2v):  ^L,F8=Repaint screen  F9=Switch display"
				     "  ^C,F10=Quit");
			statKey3 = _("Keys (3v):  Keypad Center=Center highlight"
				     "  `,\',`.\'=Pan left/right");
			break;
		case keyBindingEmacs:
			statKey1 = _("Keys (1e):  Arrows,PgUp,PgDn,Home,End=Move cursor"
				     "  Enter,Space=Select");
			statKey2 = _("Keys (2e):  F8=Repaint screen  F9=Switch display"
				     "  C-c,F10=Quit");
			statKey3 = _("Keys (3e):  Keypad Center=Center highlight"
				     "  `,\',`.\'=Pan left/right");
			break;
	}

	Init();
}

/*************************************************************************
	kcd/khyper URL window with find key help
*************************************************************************/

void	khURLWindowWithFind::ProcessCtrl(char /*c*/, bool s)
{
	wmove(win, 0, 0);
	wattrset(win, urlA);		/* Clear bar */
	for (int i = 0; i < GetCol(); i++)
		waddch(win, ' ');

	if (s)
		DrawStatusBarHelper(win, GetCol(), 20, _(statKeyCtrlF));
	else
		DrawStatusBar(-1);
	scrnMan.RequestUpdate();
}

void	khURLWindowWithFind::DrawStatusBar(int num)
{
	if (num == -1)
		num = statusNo;
	
	if (num < DISP_KEYS4FIND) {
		khURLWindow::DrawStatusBar(num);	// Never receives -1
		return;
	}
	
	wmove(win, 0, 0);
	wattrset(win, urlA);		/* Clear bar */
	for (int i = 0; i < GetCol(); i++)
		waddch(win, ' ');

	switch(num) {
		case DISP_KEYS4FIND:
			DrawStatusBarHelper(win, GetCol(), 20, _(statKey4WithFind));
			break;
		case DISP_KEYSFIND:
			DrawStatusBarHelper(win, GetCol(), 20, _(statKeyFind));
			break;
	}
	wnoutrefresh(win);
	scrnMan.RequestRestCursor();
}

khURLWindowWithFind::khURLWindowWithFind(NCScreenManager &scrnMan_, int id_) :
			khURLWindow(scrnMan_, id_)
{
	scrnMan_.SetCtrlWindow(this);

	switch (kcdConfig.cfgKey) {
		case keyBindingDefault:
			statKey4WithFind = _("Keys (4):  A-Z,a-z,0-9=Enter find mode");
			statKeyFind = _("Keys in find mode:  Up/Dn=Prev/next match"
					"  TAB=Exit find  ENTER=Select");
			statKeyCtrlF = _("^F:  8=Repaint screen  9=Switch display"
					 "  0=Quit");
			break;
		case keyBindingVi:
			statKey4WithFind = _("Keys (4v):  /,?=Enter find mode");
			statKeyFind = _("Keys in find mode:  Up,?=Prev  Dn,/=Next"
					"  TAB=Exit find  ENTER=Select");
			statKeyCtrlF = _("^F:  8=Repaint screen  9=Switch display"
					 "  0=Quit");
			break;
		case keyBindingEmacs:
			statKey4WithFind = _("Keys (4e):  C-s,C-r,A-Z,a-z,0-9=Enter find mode");
			statKeyFind = _("Keys in find mode:  Up/Dn=Prev/next match"
					"  TAB=Exit find  ENTER=Select");
			statKeyCtrlF = _("C-f:  8=Repaint screen  9=Switch display"
					 "  0=Quit");
			break;
	}
}

/*************************************************************************
	kcd/khyper Status window with find class
*************************************************************************/

/* Static member */

const int	khStatusWindowWithFind::MAX_SEARCHCHAR;

const char	*khStatusWindowWithFind::statFind1 = N_("Find: [");
const char	*khStatusWindowWithFind::statFind2 = N_("]  ");
const char	*khStatusWindowWithFind::statFindOvr = N_("Ovr");
const char	*khStatusWindowWithFind::statFindIns = N_("Ins");

int	khStatusWindowWithFind::spaceSelect = 1;	// Default = true

void	khStatusWindowWithFind::DrawStatusBar(int num)
{
	editBox->SetWin(win);		// Update editBox when called from
					// khStatusWindow::Init() after window
					// resize

	char	*statFind1Str = _(statFind1);
	char	*statFind2Str = _(statFind2);
	char	*statFindOvrStr = _(statFindOvr);
	char	*statFindInsStr = _(statFindIns);

	int	statFind1Width = GetStringWidth(statFind1Str);
	int	statFind2Width = GetStringWidth(statFind2Str);

					// Avoid signed/unsigned warning
	if (statusSearch 
	    && statFind1Width+MAX_SEARCHCHAR+3 <= GetCol()) {

		wmove(win, 0, 0);
		wattrset(win, barA);		/* Clear bar */
		for (int i = 0; i < GetCol(); i++)
			waddch(win, ' ');

		wmove(win, 0, 2);
		k_waddstr(win, statFind1Str);

					// 3 = 2(blank at the beginning)
					//     +1(blank inside [...])
		if (statFind1Width+MAX_SEARCHCHAR+3 < GetCol()) {
			wmove(win, 0, statFind1Width+MAX_SEARCHCHAR+3);
			k_waddnstr(win, statFind2Str, GetCol()-statFind1Width-MAX_SEARCHCHAR-3);
		}

		if (statFind1Width+MAX_SEARCHCHAR+3+statFind2Width < GetCol()) {
			wmove(win, 0, statFind1Width+MAX_SEARCHCHAR+3+statFind2Width);

			if (editBox->GetInsertStatus()) {
				k_waddnstr(win, statFindInsStr, GetCol()-statFind1Width
								-MAX_SEARCHCHAR-3-statFind2Width);
			}
			else {
				k_waddnstr(win, statFindOvrStr, GetCol()-statFind1Width
								-MAX_SEARCHCHAR-3-statFind2Width);
			}
		}

		editBox->Show();
		wnoutrefresh(win);
		scrnMan.RequestRestCursor();
	}
	else
		khStatusWindow::DrawStatusBar(num);
}

void	khStatusWindowWithFind::DoRestCursor()
{
	if (statusSearch) {
		editBox->DoRestCursor();	// Place cursor in edit box
		wnoutrefresh(win);
	}
}

void	khStatusWindowWithFind::ProcessKey(int ch)
{
	if (!findText) {			// findText is NULL
		return;				// exit
	}
		
	if (statusSearch) {
		if (editBox->NeedChar()) {
			if(editBox->ProcessKey(ch)) {	// Recognized key

				if (editBox->NeedUpdate()) {
					edit_string = editBox->GetString();
					findText->FindText(edit_string, find_backward);
				}
			}
			return;
		}
		else if (ch == 9 ||			// Tab
			 ((ch == 0177 || ch == '\b' || ch == KEY_BACKSPACE
			   || ch == KEY_DC)
			  && editBox->GetStringLength() == 0)) {
			statusSearch = 0;

			if (findText->CursorMode()) {
				cursor.Hide();
			}

			DrawStatusBar();	// Paint non-find status bar

					// Return keyboard & cursor control
			scrnMan.SetFocus(NULL);
			scrnMan.SetCursor(NULL);

			scrnMan.RequestUpdate();
			scrnMan.RequestRestCursor();
			return;
		}
		else if ((ch == ' ' && spaceSelect)
			 || ((ch == '\r' || ch == '\n')
			     && (kcdConfig.cfgKey == keyBindingDefault
				 || kcdConfig.cfgKey == keyBindingEmacs))) {
			statusSearch = 0;

					// Return keyboard & cursor control
			scrnMan.SetFocus(NULL);
			scrnMan.SetCursor(NULL);

			findText->FindText(edit_string, find_backward);

			if (findText->CursorMode()) {
				cursor.Hide();
			}

			DrawStatusBar();	// Paint non-find status bar

			scrnMan.RequestUpdate();
			scrnMan.RequestRestCursor();
			return;
		}
		else if ((ch == '\r' || ch == '\n')
			 && kcdConfig.cfgKey == keyBindingVi) {
			statusSearch = 0;

			if (findText->CursorMode()) {
				cursor.Hide();
			}

			if (edit_string.size())
				prev_edit_string = edit_string;
			else if (prev_edit_string.size()) {
				for (size_t i = 0; i < count; ++i) {
					if (find_backward)
						findText->FindPrev(prev_edit_string);
					else
						findText->FindNext(prev_edit_string);
				}
			}

			DrawStatusBar();	// Paint non-find status bar

					// Return keyboard & cursor control
			scrnMan.SetFocus(NULL);
			scrnMan.SetCursor(NULL);

			scrnMan.RequestUpdate();
			scrnMan.RequestRestCursor();
			return;
		}
		else if (ch == KEY_F(8)) {		// Refresh
			scrnMan.RequestRefresh();
			return;
		}
		else if (ch == KEY_F(10) || ch == 'C'-'A'+1) {
					// Return keyboard & cursor control
					// to khHyperWindowWithFind
			scrnMan.SetFocus(NULL);
			scrnMan.SetCursor(NULL);

					// Terminate keyboard loop
			findText->FindProcessKey(ch);
		}
		else if (ch == KEY_DOWN || ch == KEY_NPAGE
			 || (kcdConfig.cfgKey == keyBindingVi && ch == '/')
			 || (kcdConfig.cfgKey == keyBindingEmacs && ch == 'S'-'A'+1)) {
			findText->FindNext(edit_string);
			return;
		}
		else if (ch == KEY_UP || ch == KEY_PPAGE
			 || (kcdConfig.cfgKey == keyBindingVi && ch == '?')
			 || (kcdConfig.cfgKey == keyBindingEmacs && ch == 'R'-'A'+1)) {
			findText->FindPrev(edit_string);
			return;
		}
		else if (ch >= KEY_F(0) && ch <= KEY_F(63)) {
			findText->FindProcessKey(ch);
		}
		else if (ch == KEY_IC) {
			editBox->ProcessKey(ch);
			DrawStatusBar();
	    		return;
		}
		else {
			if(editBox->ProcessKey(ch)) {	// Recognized key

				if (editBox->NeedUpdate()) {
					edit_string = editBox->GetString();
					findText->FindText(edit_string, find_backward);
				}
			}
			return;
		}
	}
	else {
	    	statusSearch = 1;
		editBox->Clear();
		if (kcdConfig.cfgKey == keyBindingVi)
			find_backward = (ch == '?');
		else if (kcdConfig.cfgKey == keyBindingEmacs)
			find_backward = (ch == 'R'-'A'+1);
		else
			editBox->ProcessKey(ch);

	    	if (findText->CursorMode())	// Cursor previously off
			cursor.Show();		// Turn on cursor

		if (editBox->NeedUpdate()) {
			edit_string = editBox->GetString();
			findText->FindText(edit_string, find_backward);
		}

		DrawStatusBar();
	    	return;
	}
}

khStatusWindowWithFind::khStatusWindowWithFind(NCScreenManager &scrnMan_, int id_,
					SomethingWithFind *findText_) :
					khStatusWindow(scrnMan_, id_), 
					findText(findText_)
{
	editBox = new EditBox(win, GetStringWidth(_(statFind1))+2, 0, MAX_SEARCHCHAR);

	editBox->SetAttr(&barA);
	statusSearch = 0;
	count = 1;
}

void	khStatusWindowWithFind::SetFindWindow(SomethingWithFind *findText_)
{
	findText = findText_;
}

khStatusWindowWithFind::~khStatusWindowWithFind()
{
	delete editBox;
}

/*************************************************************************
	Marker bar class
*************************************************************************/

void	khMarkerBar::Init()
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
}

void	khMarkerBar::DoResize()
{
	if (win)
		delwin(win);
	win = NULL;
	Init();	
}

void	khMarkerBar::DoUpdate()
{
	if (win)
		wnoutrefresh(win);
}

khMarkerBar::khMarkerBar(NCScreenManager &scrnMan_, int id_) : 
			NCWindowBase(scrnMan_, id_)
{
	Init();
}

void	khMarkerBar::Mark(int row, chtype c)
{
	if (!win)
		return;
		
	if (row < GetRow()) {
		wmove(win, row, 0);
		waddch(win, c);
	}
}

