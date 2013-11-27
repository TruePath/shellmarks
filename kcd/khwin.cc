/*
	khwin.cc	Hypertext Window
	Copyright (c) 1997-8,2000,2002,2003,2004,2007,2009 Kriang Lerdsuwanakij
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

#include "khwin.h"
#include "cstrlib.h"

/*************************************************************************
	khGeometryManager
*************************************************************************/

bool	khGeometryManager::disableScrollBar = false;

void	khGeometryManager::SetWinRect(int id, NCRect &rect)
{
	if (mode == modeNoScroll || disableScrollBar) {
		switch(id) {
			case idHyperWindow:
				rect.SetX(0);
				rect.SetY(0);
				rect.SetRow(LINES-2);
				rect.SetCol(COLS);
				break;
			case idStatWindow:
				rect.SetX(0);
				rect.SetY(LINES-2);
				rect.SetCol(COLS);
				rect.SetRow(1);
				break;
			case idURLWindow:
				rect.SetX(0);
				rect.SetY(LINES-1);
				rect.SetCol(COLS);
				rect.SetRow(1);
				break;
			case idHScrollBar:
				rect.SetX(COLS-20);
				rect.SetY(LINES-2);
				rect.SetCol(0);
				rect.SetRow(1);
				break;
			case idVScrollBar:
				rect.SetX(COLS-1);
				rect.SetY(0);
				rect.SetCol(1);
				rect.SetRow(0);
				break;
			case idScrollBox:
				rect.SetX(COLS-1);
				rect.SetY(LINES-2);
				rect.SetCol(0);
				rect.SetRow(0);
				break;
			case idMoreBar:
				rect.SetX(COLS-2);
				rect.SetY(0);
				rect.SetCol(1);
				rect.SetRow(0);
				break;
		}
	}
	else if (mode == modeHScroll) {
		switch(id) {
			case idHyperWindow:
				rect.SetX(0);
				rect.SetY(0);
				rect.SetRow(LINES-2);
				rect.SetCol(COLS-1);	// Reserve More bar
				break;
			case idStatWindow:
				rect.SetX(0);
				rect.SetY(LINES-2);
				rect.SetCol(COLS-20);
				rect.SetRow(1);
				break;
			case idURLWindow:
				rect.SetX(0);
				rect.SetY(LINES-1);
				rect.SetCol(COLS);
				rect.SetRow(1);
				break;
			case idHScrollBar:
				rect.SetX(COLS-20);
				rect.SetY(LINES-2);
				rect.SetCol(19);
				rect.SetRow(1);
				break;
			case idVScrollBar:
				rect.SetX(COLS-1);
				rect.SetY(0);
				rect.SetCol(1);
				rect.SetRow(0);
				break;
			case idScrollBox:
				rect.SetX(COLS-1);
				rect.SetY(LINES-2);
				rect.SetCol(1);
				rect.SetRow(1);
				break;
			case idMoreBar:
				rect.SetX(COLS-1);
				rect.SetY(0);
				rect.SetCol(1);
				rect.SetRow(LINES-2);
				break;
		}
	}
	else if (mode == modeVScroll) {
		switch(id) {
			case idHyperWindow:
				rect.SetX(0);
				rect.SetY(0);
				rect.SetRow(LINES-2);
				rect.SetCol(COLS-1);	// Vertical scroll bar
				break;
			case idStatWindow:
				rect.SetX(0);
				rect.SetY(LINES-2);
				rect.SetCol(COLS);
				rect.SetRow(1);
				break;
			case idURLWindow:
				rect.SetX(0);
				rect.SetY(LINES-1);
				rect.SetCol(COLS);
				rect.SetRow(1);
				break;
			case idHScrollBar:
				rect.SetX(COLS-20);
				rect.SetY(LINES-2);
				rect.SetCol(0);
				rect.SetRow(1);
				break;
			case idVScrollBar:
				rect.SetX(COLS-1);
				rect.SetY(0);
				rect.SetCol(1);
				rect.SetRow(LINES-2);
				break;
			case idScrollBox:
				rect.SetX(COLS-1);
				rect.SetY(LINES-2);
				rect.SetCol(0);
				rect.SetRow(0);
				break;
			case idMoreBar:
				rect.SetX(COLS-2);
				rect.SetY(0);
				rect.SetCol(1);
				rect.SetRow(0);
				break;
		}
	}
	else if (mode == modeHVScroll) {
		switch(id) {
			case idHyperWindow:
				rect.SetX(0);
				rect.SetY(0);
				rect.SetRow(LINES-2);
				rect.SetCol(COLS-2);
				break;
			case idStatWindow:
				rect.SetX(0);
				rect.SetY(LINES-2);
				rect.SetCol(COLS-20);
				rect.SetRow(1);
				break;
			case idURLWindow:
				rect.SetX(0);
				rect.SetY(LINES-1);
				rect.SetCol(COLS);
				rect.SetRow(1);
				break;
			case idHScrollBar:
				rect.SetX(COLS-20);
				rect.SetY(LINES-2);
				rect.SetCol(19);
				rect.SetRow(1);
				break;
			case idVScrollBar:
				rect.SetX(COLS-1);
				rect.SetY(0);
				rect.SetCol(1);
				rect.SetRow(LINES-2);
				break;
			case idScrollBox:
				rect.SetX(COLS-1);
				rect.SetY(LINES-2);
				rect.SetCol(1);
				rect.SetRow(1);
				break;
			case idMoreBar:
				rect.SetX(COLS-2);
				rect.SetY(0);
				rect.SetCol(1);
				rect.SetRow(LINES-2);
				break;
		}
	}
}

/*************************************************************************
	khScreenManager
*************************************************************************/

ATTR_TYPE	normalA, linkA, highlightA, barA, urlA, headerA, boldA, 
		italicA, markA, linkBoldA, linkItalicA, highlightBoldA,
		highlightItalicA;
ATTR_TYPE	scrollArrowA, scrollBarA, scrollBlockA;
entity_id	scrollBarID, scrollBlockID;

void	khScreenManager::RequestResize()
{
	if (COLS < 50 || LINES < 10) {
		lockScreen = true;
		clear();
		move(0, 0);
		addstr(_("Screen too small - please resize to make it bigger."));
		refresh();
	}
	else {
		lockScreen = false;
		NCScreenManager::RequestResize();
	}
}

/* Preprocess any key pressed before passing to ProcessKey functions.  */

int	khScreenManager::ReadKeyboard()
{
	bool	isCtrlF = false;
	for ( ; ; ) {
		if (!lockScreen) {
			int	c  = NCScreenManager::ReadKeyboard();
			if (c == KEY_F(8))		// Refresh screen
				RequestRefresh();
							// ^R or ^L
			else if ((c == 'R'-'A'+1 && kcdConfig.cfgKey == keyBindingDefault)
				 || (c == 'L'-'A'+1 && kcdConfig.cfgKey == keyBindingVi))
				RequestRefresh();
							// ^F
			else if (c == 'F'-'A'+1 && kcdConfig.cfgKey == keyBindingDefault) {
				isCtrlF = !isCtrlF;	// Toggle state
				if (ctrlWin)
					ctrlWin->ProcessCtrl('F', isCtrlF);
			}
			else if (isCtrlF) {
				isCtrlF = false;
				if (c >= '0' && c <= '9') {
					if (c == '0')
						return KEY_F(10);
					else
						return KEY_F(c-'0');
				}
			}
			else
				return c;
		}
		else {
					// No display when window is too small

			isCtrlF = false;		// Clear any previous
							// ^F

					// Read and discard all input
			NCScreenManager::ReadKeyboard();
		}
	}	
}

void	khScreenManager::InitAttr(int id, const AttrConfig &cfg, ATTR_TYPE& attr)
{
	if (has_colors() == TRUE) {
		int	colorBackground = cfg.colorBackground;
		if (colorBackground == DEFAULT_COLOR)
			colorBackground = kcdConfig.cfgDefaultBackground;
		init_pair(id, cfg.colorForeground, colorBackground);
		attr = static_cast<ATTR_TYPE>(COLOR_PAIR(id) | cfg.colorAttr);
	}
	else
		attr = cfg.bwAttr;
}

khScreenManager::khScreenManager(NCGeometryManagerBase &geoMan_) : 
			NCScreenManager(geoMan_), lockScreen(false)
{
	NCurses::InitMore(stdscr);	// Set term. echo, etc. for stdscr

	InitAttr(1, kcdConfig.cfgAttrNormal, normalA);
	InitAttr(3, kcdConfig.cfgAttrLink, linkA);
	InitAttr(5, kcdConfig.cfgAttrHighlight, highlightA);
	InitAttr(7, kcdConfig.cfgAttrURL, urlA);
	InitAttr(9, kcdConfig.cfgAttrHeader, headerA);
	InitAttr(11, kcdConfig.cfgAttrBold, boldA);
	InitAttr(13, kcdConfig.cfgAttrItalic, italicA);
	InitAttr(15, kcdConfig.cfgAttrScrollArrow, scrollArrowA);
	InitAttr(17, kcdConfig.cfgAttrScrollBlock, scrollBlockA);
	InitAttr(19, kcdConfig.cfgAttrScrollBar, scrollBarA);
	InitAttr(21, kcdConfig.cfgAttrTitle, barA);
	InitAttr(23, kcdConfig.cfgAttrMore, markA);
	InitAttr(25, kcdConfig.cfgAttrLinkBold, linkBoldA);
	InitAttr(27, kcdConfig.cfgAttrLinkItalic, linkItalicA);
	InitAttr(29, kcdConfig.cfgAttrHighlightBold, highlightBoldA);
	InitAttr(31, kcdConfig.cfgAttrHighlightItalic, highlightItalicA);

	scrollBlockID = EID_SPACE;
	if (isACSFallBack)		// No special symbols
		scrollBarID = EID_SPACE;
	else
		scrollBarID = EID_CKBOARD;
}


/*************************************************************************
	HyperWindow - the complete hypertext viewer
*************************************************************************/
		
int	HyperWindow::GetDocRow()
{
	return Max(GetRow(), html.GetNumRow());
}

int	HyperWindow::GetDocColumn()
{
	return Max(GetCol(), html.GetNumColumn());
}

int	HyperWindow::GetNumChar(int y)
{
	if (y >= html.GetNumRow())
		return 0;
	return html.numChar[y];
}

HyperWindow::HyperWindow(NCScreenManager &scrn_, int id_, HyperDocument &html_,
			StatusWindowBase &status_, StatusWindowBase &url_,
			ScrollBarBase &hscroll_, ScrollBarBase &vscroll_,
			const string &startSection_, int lockKey_, int row, int col)
			: NCWindowBase(scrn_, id_), marker(scrn_, idMoreBar), 
			  html(html_), status(status_),
			  url(url_), hscroll(hscroll_), vscroll(vscroll_),
			  blankWin(NULL), newSelected("", 0, TYPE_NULL)
{
	viFoundG = false;
	viFoundZ = false;
	viFoundCount = false;
	viCount = 1;

	emacsFoundAlt = false;

	startSection = startSection_;
	lockKey = lockKey_;

	initSize = 0;
	
	isInitScreen = 0;
	InitObject();

	if (row > 0 && col > 0) {
		initRow = row;
		initColumn = col;
		initSize = 1;		// Have size info
	}

	PrepareDisplay(false);
}

HyperWindow::~HyperWindow()
{
	if (blankWin) {
		delwin(blankWin);
		blankWin = NULL;
	}
}

void	HyperWindow::InitObject()		// Clear document-dependent
						// variables
{
	anchorSelected = NULL;
	prevHighlight = NULL;
}

void	HyperWindow::Init()
{
	NCWindowBase::Init();
	if (win) {			// Free previously used pad
		delwin(win);
		win = NULL;
	}
	if (blankWin) {			// Free previously used pad
		delwin(blankWin);
		blankWin = NULL;
	}

	int	useHScroll = 0, useVScroll = 0;
	int	colLeft = COLS;
	int	rowLeft = LINES-2;	// Reserve 2 rows for status & URL
					// bar
	if (html.GetNumRow() > rowLeft) {
		useVScroll = 1;
		colLeft--;		// Vertical scroll bar taken
	}
	if (html.GetNumColumn() > colLeft) {
		useHScroll = 1;
		colLeft--;		// More bar taken
	}
	scrnMan.RequestChangeMode(useVScroll*2+useHScroll);

					// From now on, GetRow() and
					// GetCol() returns correct
					// values

	hscroll.SetTotalSize(GetDocColumn());
	hscroll.SetVisualSize(GetCol());
	vscroll.SetTotalSize(GetDocRow());
	vscroll.SetVisualSize(GetRow());

					// Save actual columns used
	numRealColumn = html.GetNumColumn();

					// Our new pad
	win = newpad(GetDocRow(), numRealColumn);
	if (win == NULL)
		throw bad_alloc();

	if (win->_line == NULL) {	// Some macros are broken due to
					// different bool size
					// Better check available ?
		throw ErrorBadNCurses();
	}

	NCurses::InitMore(win);

	if (numRealColumn < GetCol()) {
		if (GetRow() && GetCol())
			blankWin = newwin(GetRow(), GetCol()-numRealColumn,
						0, numRealColumn);
		else
			blankWin = NULL;
			
					// This shouldn't happen
		if (blankWin == NULL)
			throw bad_alloc();

		wbkgd(blankWin, normalA);		// Set normal attr as background
		wattrset(blankWin, normalA);		// Set normal text attr
		werase(blankWin);			// Use werase(...) instead
							// of wclear(...) to 
							// remove flicker
	}
	else
		blankWin = NULL;

	wbkgd(win, normalA);		// Set normal attr as background
	wattrset(win, normalA);		// Set normal text attr
}

void	HyperWindow::Highlight()
{
	int	curR, curC;
	int	row = curRow+padRow, column = curColumn+padColumn;
	int	prev = 0, next = 0;	// Linked-list traverse direction

	hscroll.SetPosition(padColumn);
	vscroll.SetPosition(padRow);

	int	i, j;
	for (i = padRow, j = 0; j < GetRow(); i++, j++) {
		if (GetNumChar(i) <= padColumn + GetCol())
			marker.Mark(j, ' '|normalA);	// Use normalA so that
							// when the text is
							// selected (using mouse
							// under xterm/rxvt), the
							// attr is the same as
							// text in dir tree
		else
			marker.Mark(j, '+'|markA);
	}
	marker.DoUpdate();

	if (prevHighlight) {
		curR = prevHighlight->rowFrom;
		curC = prevHighlight->colFrom;
		wmove(win, curR, curC);

		wattrset(win, linkA);		// wattrset required to 
						// force spaces to have the
						// same attribute as this
		
		while (curR < prevHighlight->rowTo || 
		       (curR == prevHighlight->rowTo && 
			curC <= prevHighlight->colTo)) {

			if (curC == GetNumChar(curR)) {	// End of row
				curC = 0;
				curR++;
				wmove(win, curR, curC);
			}
			else {
								// Replace attribute
#ifdef USE_UTF8_MODE
				if (IsUTF8Mode()) {
					cchar_t cc;
					win_wch(win, &cc);
					chtype	attr = cc.attr;
					if (attr == static_cast<chtype>(highlightA)) {
						wattrset(win, linkA);
						cc.attr = linkA;
					}
					else if (attr == static_cast<chtype>(highlightBoldA)) {
						wattrset(win, linkBoldA);
						cc.attr = linkBoldA;
					}
					else if (attr == static_cast<chtype>(highlightItalicA)) {
						wattrset(win, linkItalicA);
						cc.attr = linkItalicA;
					}

					wadd_wch(win, &cc);
					curC++;
				}
				else
#endif
				{
					chtype	c, chr, attr;
					c = winch(win);
					attr = c & ~(A_CHARTEXT|A_ALTCHARSET);
					chr = c & (A_CHARTEXT|A_ALTCHARSET);
					if (attr == static_cast<chtype>(highlightA)) {
						wattrset(win, linkA);
					}
					else if (attr == static_cast<chtype>(highlightBoldA)) {
						wattrset(win, linkBoldA);
					}
					else if (attr == static_cast<chtype>(highlightItalicA)) {
						wattrset(win, linkItalicA);
					}

					waddch(win, chr);
					curC++;
				}
			}
		}
		prevHighlight = NULL;
	}

	for ( ; ; ) {
		if (prev && next) {		// Found nearest anchor
			url.SetTitle("");
			DoUpdate();
			return;
		}

		if (PosAtAnchor(curAnchorIter, column, row)) {
					/* Found */

			prevHighlight = (*curAnchorIter)();

			curR = prevHighlight->rowFrom;
			curC = prevHighlight->colFrom;
			wmove(win, curR, curC);
		
			wattrset(win, highlightA);

			while (curR < prevHighlight->rowTo || 
			       (curR == prevHighlight->rowTo && 
				curC <= prevHighlight->colTo)) {

				if (curC == GetNumChar(curR)) {	// End of row
					curC = 0;
					curR++;
					wmove(win, curR, curC);
				}
				else {
									// Replace attribute
									// Fixme Unicode
#ifdef USE_UTF8_MODE
					if (IsUTF8Mode()) {
						cchar_t cc;
						win_wch(win, &cc);
						chtype	attr = cc.attr;
						if (attr == static_cast<chtype>(linkA)) {
							wattrset(win, highlightA);
							cc.attr = highlightA;
						}
						else if (attr == static_cast<chtype>(linkBoldA)) {
							wattrset(win, highlightBoldA);
							cc.attr = highlightBoldA;
						}
						else if (attr == static_cast<chtype>(linkItalicA)) {
							wattrset(win, highlightItalicA);
							cc.attr = highlightItalicA;
						}

						wadd_wch(win, &cc);
						curC++;
					}
					else
#endif
					{
						chtype	c, chr, attr;
						c = winch(win);
						attr = c & ~(A_CHARTEXT|A_ALTCHARSET);
						chr = c & (A_CHARTEXT|A_ALTCHARSET);
						if (attr == static_cast<chtype>(linkA)) {
							wattrset(win, highlightA);
						}
						else if (attr == static_cast<chtype>(linkBoldA)) {
							wattrset(win, highlightBoldA);
						}
						else if (attr == static_cast<chtype>(linkItalicA)) {
							wattrset(win, highlightItalicA);
						}

						waddch(win, chr);
						curC++;
					}
				}
			}
			url.SetTitle((*curAnchorIter)->name);
			DoUpdate();
			return;
		}
		
		if (PosBeforeAnchor(curAnchorIter, column, row)) {
				/* Have to use prev node of linked-list */
			if (curAnchorIter == html.GetAnchorList().begin()) {
					/* Ignore */
				url.SetTitle("");
				DoUpdate();
				return;
			}
			else
				--curAnchorIter;
			prev = 1;
			continue;
		}

		if (PosAfterAnchor(curAnchorIter, column, row)) {
				/* Have to use next node of linked-list */
			if (curAnchorIter == --(html.GetAnchorList().end())) {
					/* Ignore */
				url.SetTitle("");
				DoUpdate();
				return;
			}
			else
				++curAnchorIter;
			next = 1;
			continue;
		}
	}
}

void	HyperWindow::PrepareDisplay(bool sameDocument)
{
	if (! sameDocument) {

		InitObject();		// Clear document-dependent varialbles
	
		InitGeometry();		// We need window size for GetRow and
					// GetCol below
		if (!initSize) {	// No size info available
			html.FormatDocument(NULL, LINES-2, COLS-2);
		}
		else {
			initSize = 0;		// size is used
			html.SetDocumentSize(initRow, initColumn, GetRow(), GetCol());
		}

		Init();		// Allocate a new pad
	
				// Output document
		html.FormatDocument(win, LINES-2, COLS-2);

		status.SetTitle(html.GetTitle());

	}
	else {
		anchorSelected = NULL;
					// Do not reset prevHighlight
	}
	
	padRow = 0, padColumn = 0;	/* Pad position */
	curRow = 0, curColumn = 0;	/* Cursor position */
	const Anchor	*curSection = NULL;

	curAnchorIter = html.GetAnchorList().begin();

	if (html.GetAnchorList().size() == 0 && lockKey) {
		throw ErrorGenericSyntax(_("no <A HREF=...> tags present\n"
			   		   "Do not use the `-c\' option for this file"));
	}

	if (startSection.size()) {
		for (iterType iter = html.GetSectionList().begin();
		     iter != html.GetSectionList().end(); ++iter) {
			if ((*iter)->name == startSection) {
				curSection = (*iter)();
				break;
			}
		}

		if (curSection == NULL) {
			throw ErrorGenericSyntax(_("cannot find <A NAME=\"%$\">"),
						 startSection);
		}
	}

	if (lockKey || curSection) {
		if (!lockKey) {
				/* Put cursor at specified section */

				/* Center of the screen */
			padRow = (curSection->rowTo+curSection->rowFrom)/2 - 
					(GetRow()-1)/2;
			padColumn = (curSection->colTo+curSection->colFrom)/2 - 
					(GetCol()-1)/2;

				/* Check lower limit */
			if (padRow < 0)
				padRow = 0;
			if (padColumn < 0)
				padColumn = 0;

				/* Check upper limit */
			padRow = Min(padRow, GetDocRow()-GetRow());
			padColumn = Min(padColumn, GetDocColumn()-GetCol());

				/* Calc. cursor position */
			curRow = curSection->rowFrom-padRow;
			curColumn = curSection->colFrom-padColumn;
		}

		if (curSection) {
				/* Find anchor directly after curSection */
			for ( ; curAnchorIter != --(html.GetAnchorList().end()); ++curAnchorIter) {
				if ((*curAnchorIter)->rowFrom > curSection->rowFrom ||
				    ((*curAnchorIter)->rowFrom == curSection->rowFrom &&
				     (*curAnchorIter)->colFrom >= curSection->colFrom))
					break;
			}
			
		}

			/* Found first goto/exec */
		if (lockKey) {
				/* Put cursor at the link directly
				   after section name */
			CenterPad(curAnchorIter);

				/* Check lower limit */
			if (padRow < 0)
				padRow = 0;
			if (padColumn < 0)
				padColumn = 0;

				/* Check upper limit */
			padRow = Min(padRow, GetDocRow()-GetRow());
			padColumn = Min(padColumn, GetDocColumn()-GetCol());

				/* Calc. cursor position */
			curRow = (*curAnchorIter)->rowFrom-padRow;
			curColumn = (*curAnchorIter)->colFrom-padColumn;

			if ((*curAnchorIter)->colTo-(*curAnchorIter)->colFrom > GetCol()-1) {
					// Highlight bar too long - show only
					// left portion and discard parts 
					// that go beyond the screen width
					
					// Scroll left
				padColumn += curColumn;
				curColumn = 0;
			}
		}
	}

	if (lockKey)		/* No need for cursor */
		cursor.Hide();

	Highlight();
	scrnMan.RequestRestCursor();
}

bool	HyperWindow::MatchSubString(iterType &curA, const string &findText)
{
	if ((*curA)->linkText.find(findText) != string::npos)
		return true;	// Found
	return false;		// Not found
}

void	HyperWindow::DoResize()
{
	InitGeometry();

	if (!win)	// Window not initialized yet, due to mode changing
			// in Init().  This also prevents from infinite loop
			// since Init() is called below.
		return;

	int	curAnchorIndex = 0;
	int	cursorRow = 0;
	int	cursorColumn = 0;

			// Try to preserve cursor position after screen resize
	if (lockKey) {
			// Save the index of current highlighted anchor
			
		for (iterType iter = html.GetAnchorList().begin();
		     iter != curAnchorIter; ++iter) {
			curAnchorIndex++;
		}
	}
	else {		// The best we can do now - may be changed later
		cursorRow = curRow+padRow;
		cursorColumn = curColumn+padColumn;
	}

	InitObject();		// Clear document-dependent varialbles
	
	html.FormatDocument(NULL, LINES-2, COLS-2);
	Init();
	html.FormatDocument(win, LINES-2, COLS-2);

			// Restore cursor position
	if (lockKey) {
			// Seek to current highlighted anchor
			
		for (curAnchorIter = html.GetAnchorList().begin();
		     curAnchorIndex > 0; ++curAnchorIter) {
			curAnchorIndex--;
		}

				/* Put cursor at the link directly
				   after section name */

				/* Center of the screen */
		CenterPad(curAnchorIter);

				/* Check lower limit */
		if (padRow < 0)
			padRow = 0;
		if (padColumn < 0)
			padColumn = 0;

				/* Check upper limit */
		padRow = Min(padRow, GetDocRow()-GetRow());
		padColumn = Min(padColumn, GetDocColumn()-GetCol());

				/* Calc. cursor position */
		curRow = (*curAnchorIter)->rowFrom-padRow;
		curColumn = (*curAnchorIter)->colFrom-padColumn;

		if ((*curAnchorIter)->colTo-(*curAnchorIter)->colFrom > GetCol()-1) {
				// Highlight bar too long - show only
				// left portion and discard parts 
				// that go beyond the screen width
				
				// Scroll left
			padColumn += curColumn;
			curColumn = 0;
		}
	}
	else {
				// Previous cursor position is no longer 
				//	valid
		if (GetDocRow() < cursorRow)
			cursorRow = GetDocRow()-1;
		if (GetDocColumn() < cursorColumn)
			cursorColumn = GetDocColumn()-1;

				/* Center of the screen */
		padRow = cursorRow-(GetRow()-1)/2;
		padColumn = cursorColumn-(GetCol()-1)/2;

				/* Check lower limit */
		if (padRow < 0)
			padRow = 0;
		if (padColumn < 0)
			padColumn = 0;

				/* Check upper limit */
		padRow = Min(padRow, GetDocRow()-GetRow());
		padColumn = Min(padColumn, GetDocColumn()-GetCol());

				/* Calc. cursor position */
		curRow = cursorRow-padRow;
		curColumn = cursorColumn-padColumn;
	}

	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::DoUpdate()
{
	if (numRealColumn < GetCol())
		pnoutrefresh(win, padRow, padColumn, GetY1(), GetX1(),
				GetY2(), GetX1()+numRealColumn-1);
	else
		pnoutrefresh(win, padRow, padColumn, GetY1(), GetX1(),
				GetY2(), GetX2());
	if (blankWin)
		wnoutrefresh(blankWin);
}

void	HyperWindow::FitHighlightHorizontal(iterType &newA)
{
	// Six cases to consider
	// Legends: |      | = screen boundary, **** = highlight
	// 1 **|**    |		==>   |****  |
	// 2 **|***** |		==>  *|******|
	// 3 **|******|*	==> **|******|*
	// 4   | **** |		=>    | **** |
	// 5   |    **|**	==>   |  ****|
	// 6   | *****|**	==>   |******|*

	if (curColumn < 0) {
		if ((*newA)->colTo-(*newA)->colFrom <= GetCol()-1) {
				// Case 1
				// Scroll right
			padColumn += curColumn;
			curColumn = 0;
		}
		else if (curColumn+(*newA)->colTo-(*newA)->colFrom <= GetCol()-1) {
				// Case 2
			int diff = GetCol()-1
				   -curColumn-(*newA)->colTo+(*newA)->colFrom;
			padColumn += diff;
			curColumn -= diff;
		}
		else {
				// Case 3
			;
		}
	}
	else {
		if (curColumn+(*newA)->colTo-(*newA)->colFrom <= GetCol()-1) {
				// Case 4
			;
		}
		else if ((*newA)->colTo-(*newA)->colFrom <= GetCol()-1) {
				// Case 5
				// Scroll left
			int diff = curColumn+(*newA)->colTo-(*newA)->colFrom
				   -GetCol()+1;
			padColumn += diff;
			curColumn -= diff;
		}
		else {
				// Case 5
			padColumn += curColumn;
			curColumn = 0;
		}
	}
}

void	HyperWindow::FitHighlightTop(iterType &newA)
{
	if (curRow < 0) {
					/* Scroll up */
		padRow += curRow;
		curRow = 0;
	}
	FitHighlightHorizontal(newA);
}

void	HyperWindow::CenterRowPad(iterType &newA)
{
					/* Center of the screen */
	padRow = ((*newA)->rowTo+(*newA)->rowFrom)/2 - (GetRow()-1)/2;
}

void	HyperWindow::CenterRowPadIfScroll(iterType &newA)
{
	if ((*newA)->rowFrom < padRow || (*newA)->rowTo > padRow+GetRow()-1)
		CenterRowPad(newA); 
}

void	HyperWindow::CenterPad(iterType &newA)
{
					/* Center of the screen */
	CenterRowPad(newA);
	padColumn = ((*newA)->colTo+(*newA)->colFrom)/2 - 
			(GetCol()-1)/2;
}

void	HyperWindow::FitHighlightMiddle(iterType &newA)
{
					/* Check lower limit */
	if (padRow < 0)
		padRow = 0;

					/* Check upper limit */
	padRow = Min(padRow, GetDocRow()-GetRow());

					/* Calc. cursor position */
	curRow = (*newA)->rowFrom-padRow;
	curColumn = (*newA)->colFrom-padColumn;

	FitHighlightHorizontal(newA);
}

void	HyperWindow::FitHighlightBottom(iterType &newA)
{
	if (curRow+(*newA)->rowTo-(*newA)->rowFrom > GetRow()-1) {
					/* Scroll down */
		padRow += curRow+(*newA)->rowTo-(*newA)->rowFrom-(GetRow()-1);
		curRow = GetRow()-1-((*newA)->rowTo-(*newA)->rowFrom);
	}

	FitHighlightHorizontal(newA);
}

bool	HyperWindow::PosBeforeAnchor(iterType &p, int x, int y)
{
	if (y < (*p)->rowFrom || (y == (*p)->rowFrom && x < (*p)->colFrom))
		return true;
	return false;
}

bool	HyperWindow::PosAfterAnchor(iterType &p, int x, int y)
{
	if (y > (*p)->rowTo || (y == (*p)->rowTo && x > (*p)->colTo))
		return true;
	return false;
}

bool	HyperWindow::PosAtAnchor(iterType &p, int x, int y)
{
	return !(PosAfterAnchor(p, x, y) || PosBeforeAnchor(p, x, y));
}

Anchor	HyperWindow::ProcessCall(Anchor * /*p*/)
{
	return Anchor("", 0, TYPE_NULL);		// Do nothing
}

#ifdef NCURSES_MOUSE_VERSION
void	HyperWindow::ProcessMouse(MEVENT &event)
{
			// Only accepts mouse button 1
	if (!(event.bstate & (BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED
			      | BUTTON1_TRIPLE_CLICKED | BUTTON1_RELEASED)))
		return;

	iterType saveIter = curAnchorIter;
	if (html.GetAnchorList().empty())		// No links found
		return;

	int	absRow = event.y-GetY1()+padRow;
	int	absColumn = event.x-GetX1()+padColumn;
	if (PosAtAnchor(curAnchorIter, absColumn, absRow)) {
		curRow = absRow-padRow;
		curColumn = absColumn-padColumn;
		ProcessKey('\r');
	}
	else if (PosBeforeAnchor(curAnchorIter, absColumn, absRow)) {
		while (curAnchorIter != html.GetAnchorList().begin()
		       && PosBeforeAnchor(curAnchorIter, absColumn, absRow)) {
			--curAnchorIter;
		} 

		if (PosAtAnchor(curAnchorIter, absColumn, absRow)) {
			curRow = absRow-padRow;
			curColumn = absColumn-padColumn;
			ProcessKey('\r');
		}
		else
			curAnchorIter = saveIter;
	}
	else {
		while (curAnchorIter != --(html.GetAnchorList().end())
		       && PosBeforeAnchor(curAnchorIter, absColumn, absRow)) {
			++curAnchorIter;
		} 
		
		if (PosAtAnchor(curAnchorIter, absColumn, absRow)) {
			curRow = absRow-padRow;
			curColumn = absColumn-padColumn;
			ProcessKey('\r');
		}
		else
			curAnchorIter = saveIter;
	}
}
#endif

void	HyperWindow::ProcessKeySelect()
{
	int	absRow = curRow+padRow;
	int	absCol = curColumn+padColumn;

					/* Look for anchor under
					   cursor */
	if (html.GetAnchorList().empty())
		return;			/* Ignore */
					
	if (PosAtAnchor(curAnchorIter, absCol, absRow)) {
		anchorSelected = (*curAnchorIter)();
	}
	else			/* Cursor not on any link */
		return;

	if (anchorSelected) {
		if (anchorSelected->type == TYPE_CALL) {
			newSelected = *anchorSelected;
			while (newSelected.type == TYPE_CALL) {
				newSelected = ProcessCall(&newSelected);
				if (newSelected.type == TYPE_NULL)
					return;		// Ignore
			}
			anchorSelected = &newSelected;
		}
			
		if (anchorSelected->type == TYPE_GOTO) {

			string	newHyperFile;

			string::size_type s = anchorSelected->name.find('#');
			if (s != string::npos) {	
				startSection = string(anchorSelected->name, s+1);
				newHyperFile = string(anchorSelected->name, 0, s);
			}
			else {
				startSection = "";
				newHyperFile = anchorSelected->name;
			}
			anchorSelected = NULL;

					/* Link to another file */
			if (newHyperFile.size()) {
#ifndef TRIM_NO_DOC_FILE
				html.LoadDocument(newHyperFile);
				PrepareDisplay(false);
#else
				;
#endif
			}
			else {	// Link to the same file
				PrepareDisplay(true);
			}

			Highlight();
			scrnMan.RequestRestCursor();
		}
		else if (anchorSelected->type == TYPE_EXEC) {
			scrnMan.SetFocus(NULL);
			return;
		}
		else if (anchorSelected->type == TYPE_EXIT) {
			scrnMan.SetFocus(NULL);
			return;
		}
		/* else unknown anchor type */
	}
}

void	HyperWindow::ProcessKeyCenter()
{
	if (html.GetAnchorList().empty())
		return;		/* Ignore */
		
	int	absRow = curRow+padRow;
	int	absCol = curColumn+padColumn;

	if (PosAtAnchor(curAnchorIter, absCol, absRow)) {
		CenterRowPad(curAnchorIter);
		FitHighlightMiddle(curAnchorIter);
		Highlight();
		scrnMan.RequestRestCursor();
	}
	else		/* Cursor not on any link */
			// FIXME: center screen
		return;
}

void	HyperWindow::ProcessKeyNextStatus()
{
	url.NextStatus();
	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::ProcessKeyExit()
{
	scrnMan.SetFocus(NULL);
}

void	HyperWindow::ProcessKeyPanLeft()
{
	if (lockKey) {
		if ((*curAnchorIter)->rowFrom == (*curAnchorIter)->rowTo) {

				// Highlight bar not as wide as screen				
			if ((*curAnchorIter)->colTo-(*curAnchorIter)->colFrom+1 < GetCol()) {

				// Make sure the entire
				// highlight bar is displayed
			
				if (padColumn+GetCol()-1 > (*curAnchorIter)->colTo) {
					if (padColumn > 0) {
						padColumn--;
						curColumn++;
					}
				}
			}
			
				// Highlight bar wider than screen
			else if ((*curAnchorIter)->colTo-(*curAnchorIter)->colFrom+1 > GetCol()) {
				if (padColumn > (*curAnchorIter)->colFrom) {
					padColumn--;
					curColumn++;
				}
			}
		}
		else {
				// Multi-line highlight
				
			if (padColumn > 0) {
				padColumn--;
				curColumn++;
			}
		}
	}
	else {
		if (padColumn > 0)
			padColumn--;
	}
	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::ProcessKeyPanRight()
{
	if (lockKey) {
		if ((*curAnchorIter)->rowFrom == (*curAnchorIter)->rowTo) {

				// Highlight bar not as wide as screen				
			if ((*curAnchorIter)->colTo-(*curAnchorIter)->colFrom+1 < GetCol()) {
				
				// Make sure the entire
				// highlight bar is displayed
			
				if (padColumn < (*curAnchorIter)->colFrom)
					if (padColumn+GetCol() < GetDocColumn()) {
						padColumn++;
						curColumn--;
					}
			}
			
				// Highlight bar wider than screen
			else if ((*curAnchorIter)->colTo-(*curAnchorIter)->colFrom+1 > GetCol()) {
				if (padColumn+GetCol()-1 < (*curAnchorIter)->colTo) {
					padColumn++;
					curColumn--;
				}
			}
		}
		else {
				// Multi-line highlight
				
			if (padColumn+GetCol() < GetDocColumn()) {
				padColumn++;
				curColumn--;
			}
		}
	}
	else {
		if (padColumn+GetCol() < GetDocColumn())
			padColumn++;
	}
	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::ProcessKeyUp()
{
	if (lockKey) {
		iterType saveIter = curAnchorIter;
		if (curAnchorIter != html.GetAnchorList().begin()) {
					/* Skip links in current line */
			do {
				--curAnchorIter;
			} while (curAnchorIter != html.GetAnchorList().begin() 
				 && ((*curAnchorIter)->rowFrom == (*saveIter)->rowFrom));

						// Initialize in case this is the
						// topmost anchor
			iterType bestAnc = html.GetAnchorList().begin();
			int	bestDiff = INT_MAX, diff;
			int	bestOverlap = 0, overlap, overlapRow = -1;

			/* Now link not in current line */
			for ( ; ; ) {
				int	from1 = (*curAnchorIter)->colFrom;
				int	to1 = (*curAnchorIter)->colTo;
				int	wrap1 = (*curAnchorIter)->rowTo-(*curAnchorIter)->rowFrom;
				int	from2 = (*saveIter)->colFrom;
				int	to2 = (*saveIter)->colTo;
				int	wrap2 = (*saveIter)->rowTo-
						(*saveIter)->rowFrom;

				overlap = 0;

				if (!wrap1 && !wrap2 &&
				    (Max(from1, from2) <= Min(to1, to2)))
					overlap = Min(to1, to2)-Max(from1, from2)+1;
				else if (wrap1 && !wrap2 && 
					 (from2 <= to1))
					overlap = to1-from2+1;
				else if (wrap1 && !wrap2 && 
					 (to2 >= from1))
					overlap = to2-from1+1;
				else if (!wrap1 && wrap2 &&
					 (from1 <= to2))
					overlap = to2-from1+1;
				else if (!wrap1 && wrap2 &&
					 (to1 >= from2))
					overlap = to1-from2+1;
				else if (wrap1 && wrap2)
					overlap = Min(to1, to2)+1+
						  (GetCol()-Max(from1, from2)+1);

				if (overlap) {
			     			/* Found */
			     		bestDiff = 0;
			     		if (overlapRow == -1)
			     			overlapRow = (*curAnchorIter)->rowFrom;
			     		
			     		if (overlap > bestOverlap) {
				     		
			     			/* Already have an overlap, ignore anchors in 
			     			   further lines.  */
						if (overlapRow != (*curAnchorIter)->rowFrom)
							break;

						bestOverlap = overlap;
						bestAnc = curAnchorIter;
					}
				}
				else {
					if (bestDiff == 0)
						break;

					diff = Min(abs(from2-to1),
						   abs(from1-to2));
					if (diff < bestDiff) {
						bestDiff = diff;
						bestAnc = curAnchorIter;
					}
				}
					
				if (curAnchorIter == html.GetAnchorList().begin())
					break;

				--curAnchorIter;
			}						
						/* Cannot find links aligned */
			curAnchorIter = bestAnc;

			curRow -= (*saveIter)->rowFrom - (*curAnchorIter)->rowFrom;
			curColumn -= (*saveIter)->colFrom - (*curAnchorIter)->colFrom;
			FitHighlightTop(curAnchorIter);
		}
	}
	else {
		if (curRow+padRow > 0) {
			curRow--;
			if (curRow < 0) {
				curRow = 0;
				padRow--;
			}
		}
		else
			return;
	}
				
	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::ProcessKeyDown()
{
	if (lockKey) {
		iterType saveIter = curAnchorIter;
		if (curAnchorIter != --(html.GetAnchorList().end())) {

					/* Skip links in current line */
			do {
				++curAnchorIter;
			} while (curAnchorIter != --(html.GetAnchorList().end())
				 && ((*curAnchorIter)->rowFrom == (*saveIter)->rowFrom));

						// Initialize in case this is the
						// bottommost anchor
			iterType bestAnc = --(html.GetAnchorList().end());
			int	bestDiff = INT_MAX, diff;
			int	bestOverlap = 0, overlap, overlapRow = -1;
				/* Now link not in current line */
			for ( ; ; ) {
				int	from1 = (*curAnchorIter)->colFrom;
				int	to1 = (*curAnchorIter)->colTo;
				int	wrap1 = (*curAnchorIter)->rowTo-(*curAnchorIter)->rowFrom;
				int	from2 = (*saveIter)->colFrom;
				int	to2 = (*saveIter)->colTo;
				int	wrap2 = (*saveIter)->rowTo-
						(*saveIter)->rowFrom;

				overlap = 0;

				if (!wrap1 && !wrap2 &&
				    (Max(from1, from2) <= Min(to1, to2)))
					overlap = Min(to1, to2)-Max(from1, from2)+1;
				else if (wrap1 && !wrap2 && 
					 (from2 <= to1))
					overlap = to1-from2+1;
				else if (wrap1 && !wrap2 && 
					 (to2 >= from1))
					overlap = to2-from1+1;
				else if (!wrap1 && wrap2 &&
					 (from1 <= to2))
					overlap = to2-from1+1;
				else if (!wrap1 && wrap2 &&
					 (to1 >= from2))
					overlap = to1-from2+1;
				else if (wrap1 && wrap2)
					overlap = Min(to1, to2)+1+
						  (GetCol()-Max(from1, from2)+1);

				if (overlap) {
			     			/* Found */
			     		bestDiff = 0;
			     		if (overlapRow == -1)
			     			overlapRow = (*curAnchorIter)->rowFrom;
			     		
			     		if (overlap > bestOverlap) {
				     		
			     			/* Already have an overlap, ignore anchors in 
			     			   further lines.  */
						if (overlapRow != (*curAnchorIter)->rowFrom)
							break;

						bestOverlap = overlap;
						bestAnc = curAnchorIter;
					}
				}
				else {
					if (bestDiff == 0)
						break;

					diff = Min(abs(from2-to1),
						   abs(from1-to2));
					if (diff < bestDiff) {
						bestDiff = diff;
						bestAnc = curAnchorIter;
					}
				}

				if (curAnchorIter == --(html.GetAnchorList().end()))
					break;

				++curAnchorIter;

			}					

						/* Cannot find links aligned */
				curAnchorIter = bestAnc;

			curRow -= (*saveIter)->rowFrom - (*curAnchorIter)->rowFrom;
			curColumn -= (*saveIter)->colFrom - (*curAnchorIter)->colFrom;
			FitHighlightBottom(curAnchorIter);
		}
	}	
	else {
		if (curRow+padRow < GetDocRow()-1) {
			curRow++;
			if (curRow >= GetRow()) {
				curRow = GetRow()-1;
				padRow++;
			}
		}
		else
			return;
	}
				
	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::ProcessKeyFirstLine()
{
	if (lockKey) {
		iterType saveIter = curAnchorIter;
		if (curAnchorIter != html.GetAnchorList().begin()) {

					/* Skip links till the first row of screen */
			do {
				--curAnchorIter;
			} while (curAnchorIter != html.GetAnchorList().begin()
				 && ((*curAnchorIter)->rowFrom == (*saveIter)->rowFrom
					|| (*curAnchorIter)->rowFrom > padRow));
			curRow -= (*saveIter)->rowFrom - (*curAnchorIter)->rowFrom;
			curColumn -= (*saveIter)->colFrom - (*curAnchorIter)->colFrom;
			FitHighlightTop(curAnchorIter);
		}
	}
	else {
		if (curRow+padRow > 0) {
			curRow--;
			if (curRow < 0) {
				curRow = 0;
				padRow--;
			}
		}
		else
			return;
	}
				
	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::ProcessKeyLastLine()
{
	if (lockKey) {
		iterType saveIter = curAnchorIter;
		if (curAnchorIter != --(html.GetAnchorList().end())) {

				/* Skip links till the last row of screen */
			do {
				++curAnchorIter;
			} while (curAnchorIter != --(html.GetAnchorList().end())
				 && ((*curAnchorIter)->rowFrom == (*saveIter)->rowFrom
					|| (*curAnchorIter)->rowFrom < padRow+GetRow()-1));

			curRow -= (*saveIter)->rowFrom - (*curAnchorIter)->rowFrom;
			curColumn -= (*saveIter)->colFrom - (*curAnchorIter)->colFrom;
			FitHighlightBottom(curAnchorIter);
		}
	}	
	else {
		if (curRow+padRow < GetDocRow()-1) {
			curRow++;
			if (curRow >= GetRow()) {
				curRow = GetRow()-1;
				padRow++;
			}
		}
		else
			return;
	}
				
	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::ProcessKeyScrollUpLines(size_t lines)
{
	if (!lines)
		return;
	if (lockKey) {
		iterType saveIter = curAnchorIter;
		if (curAnchorIter != html.GetAnchorList().begin()) {
			do {
				--curAnchorIter;

				if ((*curAnchorIter)->rowFrom <= (*saveIter)->rowFrom-static_cast<int>(lines)) {
					padRow -= lines;
					if (padRow < 0)
						padRow = 0;
					break;
				}
			} while(curAnchorIter != html.GetAnchorList().begin());

			if (curAnchorIter == html.GetAnchorList().begin())
				padRow = 0;
					
			curRow = (*curAnchorIter)->rowFrom-padRow;
			curColumn = (*curAnchorIter)->colFrom-padColumn;

			if (curRow < 0) {
				/* Scroll up */
				padRow += curRow;
				curRow = 0;
			}
			if (curRow+(*curAnchorIter)->rowTo-(*curAnchorIter)->rowFrom > GetRow()-1) {
				/* Scroll down */
				padRow += curRow+(*curAnchorIter)->rowTo-(*curAnchorIter)->rowFrom-
						(GetRow()-1);
				curRow = GetRow()-1-((*curAnchorIter)->rowTo-(*curAnchorIter)->rowFrom);
			}
			if (curColumn < 0) {
				/* Scroll left */
				padColumn += curColumn;
				curColumn = 0;
			}
			if (curColumn+(*curAnchorIter)->colTo-(*curAnchorIter)->colFrom > GetCol()-1) {
				if ((*curAnchorIter)->colTo-(*curAnchorIter)->colFrom > GetCol()-1) {
					// Highlight bar too long - show only
					// left portion and discard parts 
					// that go beyond the screen width
										
					// Scroll left
					padColumn += curColumn;
					curColumn = 0;
				}
				else {	// Can fit highlight on the screen
					/* Scroll right */
					padColumn += curColumn+(*curAnchorIter)->colTo
						-(*curAnchorIter)->colFrom-(GetCol()-1);
					curColumn = GetCol()-1-((*curAnchorIter)->colTo-(*curAnchorIter)->colFrom);
				}
			}
		}
	}
	else {
		if (curRow+padRow > 0) {
			padRow -= lines;
			if (padRow < 0) {
				padRow = 0;
				curRow = 0;
			}
		}
		else
			return;
	}
			
	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::ProcessKeyScrollDownLines(size_t lines)
{
	if (!lines)
		return;
	if (lockKey) {
		iterType saveIter = curAnchorIter;
		if (curAnchorIter != --(html.GetAnchorList().end())) {
			do {
				++curAnchorIter;

				if ((*curAnchorIter)->rowFrom >= (*saveIter)->rowFrom+static_cast<int>(lines)) {
					padRow += lines;
					if (padRow+GetRow() > GetDocRow())
						padRow = GetDocRow()-GetRow();
					break;
				}
			} while(curAnchorIter != --(html.GetAnchorList().end()));

			if (curAnchorIter == --(html.GetAnchorList().end()))
				padRow = GetDocRow()-GetRow();

			curRow = (*curAnchorIter)->rowFrom-padRow;
			curColumn = (*curAnchorIter)->colFrom-padColumn;
	
			if (curRow+(*curAnchorIter)->rowTo-(*curAnchorIter)->rowFrom > GetRow()-1) {
				/* Scroll down */
				padRow += curRow+(*curAnchorIter)->rowTo-(*curAnchorIter)->rowFrom-
						(GetRow()-1);
				curRow = GetRow()-1-((*curAnchorIter)->rowTo-(*curAnchorIter)->rowFrom);
			}
			if (curRow < 0) {
				/* Scroll up */
				padRow += curRow;
				curRow = 0;
			}
			if (padRow+GetRow() > GetDocRow()) {
				/* Scroll down */
				padRow = GetDocRow()-GetRow();
				curRow = (*curAnchorIter)->rowFrom-padRow;
			}
			if (curColumn < 0) {
				/* Scroll left */
				padColumn += curColumn;
				curColumn = 0;
			}
			if (curColumn+(*curAnchorIter)->colTo-(*curAnchorIter)->colFrom > GetCol()-1) {
				if ((*curAnchorIter)->colTo-(*curAnchorIter)->colFrom > GetCol()-1) {
					// Highlight bar too long - show only
					// left portion and discard parts 
					// that go beyond the screen width
										
					// Scroll left
					padColumn += curColumn;
					curColumn = 0;
				}
				else {	// Can fit highlight on the screen
					/* Scroll right */
					padColumn += curColumn+(*curAnchorIter)->colTo
						-(*curAnchorIter)->colFrom-(GetCol()-1);
					curColumn = GetCol()-1-((*curAnchorIter)->colTo-(*curAnchorIter)->colFrom);
				}
			}
		}
	}
	else {
		if (curRow+padRow < GetDocRow()-1) {
			padRow += lines;
			if (padRow+GetRow() > GetDocRow()) {
				curRow += padRow+GetRow()-GetDocRow();
				padRow = GetDocRow()-GetRow();
			}
			curRow = Min(curRow, GetRow()-1);
		}
		else
			return;
	}
			
	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::ProcessKeyFirstPage()
{
	if (lockKey) {
		if (curAnchorIter != html.GetAnchorList().begin()) {
			curAnchorIter = html.GetAnchorList().begin();
			
			padRow = 0;
			padColumn = 0;
			curRow = (*curAnchorIter)->rowFrom-padRow;
			curColumn = (*curAnchorIter)->colFrom-padColumn;
			FitHighlightTop(curAnchorIter);
		}
	}
	else {
		padRow = 0;
		padColumn = 0;
		curRow = 0;
		curColumn = 0;
	}
			
	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::ProcessKeyLastPage()
{
	if (lockKey) {
		if (curAnchorIter != --(html.GetAnchorList().end())) {
			curAnchorIter = html.GetAnchorList().end();
			--curAnchorIter;

			padRow = GetDocRow()-GetRow();
			padColumn = 0;
			curRow = (*curAnchorIter)->rowFrom-padRow;
			curColumn = (*curAnchorIter)->colFrom-padColumn;
			FitHighlightTop(curAnchorIter);
		}
	}
	else {
		padRow = GetDocRow()-GetRow();
		padColumn = 0;
		curRow = GetRow()-1;
		curColumn = 0;
	}
				
	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::ProcessKeyPrev()
{
	if (lockKey) {
		iterType saveIter = curAnchorIter;
		if (curAnchorIter != html.GetAnchorList().begin()) {
			--curAnchorIter;
			curRow -= (*saveIter)->rowFrom - (*curAnchorIter)->rowFrom;
			curColumn -= (*saveIter)->colFrom - (*curAnchorIter)->colFrom;
			FitHighlightTop(curAnchorIter);
		}
	}
	else {
		if (curColumn+padColumn > 0) {
			curColumn--;
			if (curColumn < 0) {
				curColumn = 0;
				padColumn--;
			}
		}
		else	/* Already reach left limit */
			return;
	}	
				
	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::ProcessKeyNext()
{
	if (lockKey) {
		iterType saveIter = curAnchorIter;
		++curAnchorIter;
		if (curAnchorIter != html.GetAnchorList().end()) {
			curRow -= (*saveIter)->rowFrom - (*curAnchorIter)->rowFrom;
			curColumn -= (*saveIter)->colFrom - (*curAnchorIter)->colFrom;
			FitHighlightBottom(curAnchorIter);
		}
		else	/* Do not update curAnchorIter */
			curAnchorIter = saveIter;
	}
	else {
		if (curColumn+padColumn < GetDocColumn()-1) {
			curColumn++;
			if (curColumn >= GetCol()) {
				curColumn = GetCol()-1;
				padColumn++;
			}
		}
		else	/* Already reach right limit */
			return;
	}
		
	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::ProcessKeyBeginLine()
{
	if (lockKey) {
		iterType saveIter = curAnchorIter;
		while (curAnchorIter != html.GetAnchorList().begin()
		       && (*curAnchorIter)->rowFrom == (*saveIter)->rowFrom)
			--curAnchorIter;

		if ((*curAnchorIter)->rowFrom != (*saveIter)->rowFrom)
			++curAnchorIter;

		if (curAnchorIter != saveIter) {
			curRow -= (*saveIter)->rowFrom - (*curAnchorIter)->rowFrom;
			curColumn -= (*saveIter)->colFrom - (*curAnchorIter)->colFrom;
			FitHighlightTop(curAnchorIter);
		}
	}
	else {
		if (curColumn+padColumn > 0) {
			curColumn = 0;
			padColumn = 0;
		}
		else	/* Already reach left limit */
			return;
	}	
				
	Highlight();
	scrnMan.RequestRestCursor();
}

void	HyperWindow::ProcessKeyEndLine()
{
	if (lockKey) {
		iterType saveIter = curAnchorIter;
		while (curAnchorIter != html.GetAnchorList().end()
		       && (*curAnchorIter)->rowFrom == (*saveIter)->rowFrom)
			++curAnchorIter;

				// Rewind always required
				// because html.GetAnchorList().end()
				// is not valid
		--curAnchorIter;

		if (curAnchorIter != saveIter) {
			curRow -= (*saveIter)->rowFrom - (*curAnchorIter)->rowFrom;
			curColumn -= (*saveIter)->colFrom - (*curAnchorIter)->colFrom;
			FitHighlightBottom(curAnchorIter);
		}
	}
	else {
		int diff = GetDocColumn()-1-curColumn-padColumn;
		if (diff > 0) {
			curColumn += diff;
			if (curColumn >= GetCol()) {
				curColumn = GetCol()-1;
				padColumn += diff;
			}
		}
		else	/* Already reach right limit */
			return;
	}
		
	Highlight();
	scrnMan.RequestRestCursor();
}


void	HyperWindow::ProcessKey(int ch)
{
			// Key shared for all key binding

	if (ch == ' ' || ch == '\r' || ch == '\n')
		ProcessKeySelect();
	else if (ch == KEY_B2)		// Center of keypad
		ProcessKeyCenter();
	else if (ch == KEY_F(9))
		ProcessKeyNextStatus();
	else if (ch == KEY_F(10) || ch == 'C'-'A'+1)
		ProcessKeyExit();
					// comma when shifted is `<'
	else if (ch == ',' || ch == KEY_SLEFT)
		ProcessKeyPanLeft();
					// stop when shifted is `>'
	else if (ch == '.' || ch == KEY_SRIGHT)
		ProcessKeyPanRight();
	else if (ch == KEY_UP || ch == KEY_SR)
		ProcessKeyUp();
	else if (ch == KEY_DOWN || ch == KEY_SF)
		ProcessKeyDown();
	else if (ch == KEY_SR)
		ProcessKeyFirstLine();
	else if (ch == KEY_SF)
		ProcessKeyLastLine();
	else if (ch == KEY_PPAGE)
		ProcessKeyPrevPage();
	else if (ch == KEY_NPAGE)
		ProcessKeyNextPage();
	else if (ch == KEY_HOME)
		ProcessKeyFirstPage();
	else if (ch == KEY_END)
		ProcessKeyLastPage();
	else if (ch == KEY_LEFT)
		ProcessKeyPrev();
	else if (ch == KEY_RIGHT)
		ProcessKeyNext();

	else if (kcdConfig.cfgKey == keyBindingVi) {
		bool	processG = false;
		bool	processZ = false;
		bool	processCount = false;

		if (ch >= '1' && ch <= '9') {
			if (viFoundCount)
				viCount = viCount*10 + ch-'0';
			else {
				viFoundCount = true;
				viCount = ch-'0';
			}
			processCount = true;
		}
		else if (ch == '0' && viFoundCount) {
			viCount = viCount*10;
			processCount = true;
		}
		else if (ch == 'q' || ch == 'Q')	// q, ZQ
			ProcessKeyExit();
		else if (ch == 'h' || ch == 'b' || ch == 'B') {
			for (size_t i = 0; i < viCount; ++i)
				ProcessKeyPrev();
		}
		else if (ch == 'l' || ch == 'w' || ch == 'W') {
			for (size_t i = 0; i < viCount; ++i)
				ProcessKeyNext();
		}
		else if (ch == '0' || ch == '^')
			ProcessKeyBeginLine();
		else if (ch == '$')
			ProcessKeyEndLine();
		else if (ch == 'k' || ch == '-') {
			for (size_t i = 0; i < viCount; ++i)
				ProcessKeyUp();
		}
		else if (ch == 'j' || ch == '+' || ch == '_') {
			for (size_t i = 0; i < viCount; ++i)
				ProcessKeyDown();
		}
		else if (ch == 'B'-'A'+1) {	// ^B
			for (size_t i = 0; i < viCount; ++i)
				ProcessKeyPrevPage();
		}
		else if (ch == 'F'-'A'+1) {	// ^F
			for (size_t i = 0; i < viCount; ++i)
				ProcessKeyNextPage();
		}
		else if (ch == 'U'-'A'+1) {	// ^U
			if (viFoundCount)
				ProcessKeyScrollUpLines(viCount);
			else
				ProcessKeyScrollUpLines(GetRow()/2-1);
		}
		else if (ch == 'D'-'A'+1) {	// ^D
			if (viFoundCount)
				ProcessKeyScrollDownLines(viCount);
			else
				ProcessKeyScrollDownLines(GetRow()/2-1);
		}
		else if (ch == 'E'-'A'+1) {	// ^E
			if (viCount) {
				if (viCount > static_cast<size_t>(curRow)) {
					ProcessKeyScrollDownLines(viCount-curRow);
					viCount = curRow;
				}

				curRow -= viCount;
				padRow += viCount;
				Highlight();
				scrnMan.RequestRestCursor();
			}
		}
		else if (ch == 'Y'-'A'+1) {	// ^Y
			if (viCount) {
				if (viCount > static_cast<size_t>(GetRow()-1-curRow)) {
					ProcessKeyScrollUpLines(viCount-(GetRow()-1-curRow));
					viCount = GetRow()-1-curRow;
				}

				curRow += viCount;
				padRow -= viCount;
				Highlight();
				scrnMan.RequestRestCursor();
			}
		}
		else if (ch == 'g') {
			processG = true;
			if (viFoundG)		// gg
				ProcessKeyFirstPage();
			else
				viFoundG = true;
		}
		else if (ch == 'G')
			ProcessKeyLastPage();
		else if (ch == 'Z') {
			processZ = true;
			if (viFoundZ)
				ProcessKeyExit();
			else
				viFoundZ = true;
		}

		// FIXME: Maybe add zh, zl, zH, zL

		if (!processG)			// Ignore the `g' in `gX'
						// key sequence where X is
						// not `g'
			viFoundG = false;
		if (!processZ)
			viFoundZ = false;
		if (!processCount) {
			viFoundCount = false;
			viCount = 1;
		}
	}
	else if (kcdConfig.cfgKey == keyBindingEmacs) {
		bool	processAlt = false;

		if (ch == 27) {				// Esc, M-
			emacsFoundAlt = !emacsFoundAlt;
			processAlt = true;
		}
		else if (ch == 'A'-'A'+1)		// C-a
			ProcessKeyBeginLine();
		else if (ch == 'E'-'A'+1)		// C-e
			ProcessKeyEndLine();
		else if (ch == 'B'-'A'+1 || (ch == 'b' && emacsFoundAlt))	// C-b, M-b
			ProcessKeyPrev();
		else if (ch == 'F'-'A'+1 || (ch == 'f' && emacsFoundAlt))	// C-f, M-f
			ProcessKeyNext();
		else if (ch == 'P'-'A'+1)		// C-p
			ProcessKeyUp();
		else if (ch == 'N'-'A'+1)		// C-n
			ProcessKeyDown();
		else if (ch == 'v' && emacsFoundAlt)	// M-v
			ProcessKeyPrevPage();
		else if (ch == 'V'-'A'+1)		// C-v
			ProcessKeyNextPage();
		// FIXME: Emulate ^X^S, ^X^C

		if (!processAlt)
			emacsFoundAlt = false;
	}
}

void	HyperWindow::KeyboardLoop()
{
	NCWindowBase::KeyboardLoop();

					// Exit keyboard loop

	if (win) {			// Delete windows
		delwin(win);
		win = NULL;
	}
	if (blankWin) {
		delwin(blankWin);
		blankWin = NULL;
	}

					// Exit program, don't have
					// to delete other windows

	clear();			// Clear screen
	refresh();
	cursor.Restore();

	NCurses::End();

	startSection = "";		// No longer used

	if (anchorSelected) {
		if (anchorSelected->type == TYPE_EXEC) {
			k_system(anchorSelected->name);
			return;
		}
	}
}

void	HyperWindow::DoRestCursor()
{
	if (win == NULL)
		return;
		
					// blankWin must be refreshed first
					// to avoid interfering the cursor
					// position
	if (blankWin)
		wnoutrefresh(blankWin);

	if (lockKey)
		wmove(win, padRow+GetRow()-1, padColumn+GetCol()-1);
	else
		wmove(win, padRow+curRow, padColumn+curColumn);

					// win's cursor position will be
					// used by doupdate().
	if (numRealColumn < GetCol())
		pnoutrefresh(win, padRow, padColumn, GetY1(), GetX1(),
				GetY2(), GetX1()+numRealColumn-1);
	else
		pnoutrefresh(win, padRow, padColumn, GetY1(), GetX1(),
				GetY2(), GetX2());
}

int	khHyperWindowWithFind::CursorMode()
{
	if (lockKey)
		return 1;
	else
		return 0;
}

void	khHyperWindowWithFind::FindText(const string &text, bool find_backward)
{
	if (text.size() == 0)
		return;

	iterType newAnchorIter = curAnchorIter;

#ifdef	CLIB_HAVE_REGEX
	string str = MakeString(text);
	regex_t *reg = MakePathRegex(text);
#endif

	do {
		if (MatchSubString(newAnchorIter, text)
#ifdef	CLIB_HAVE_REGEX
		    || (MatchStr(str, (*newAnchorIter)->name)
			&& MatchPathRegex(reg, (*newAnchorIter)->name))
#endif
		    ) {
			CenterRowPadIfScroll(newAnchorIter);
			FitHighlightMiddle(newAnchorIter);
			curAnchorIter = newAnchorIter;
			break;
		}
		if (find_backward)
			prev_loop(html.GetAnchorList(), newAnchorIter);
		else
			next_loop(html.GetAnchorList(), newAnchorIter);

	} while (newAnchorIter != curAnchorIter);	/* Loop not repeated */

	Highlight();
	scrnMan.RequestRestCursor();

#ifdef	CLIB_HAVE_REGEX
	if (reg)
		regfree(reg);
#endif
}

void	khHyperWindowWithFind::FindPrev(const string &text)
{
	iterType newAnchorIter = curAnchorIter;

	prev_loop(html.GetAnchorList(), newAnchorIter);

#ifdef	CLIB_HAVE_REGEX
	string str = MakeString(text);
	regex_t *reg = MakePathRegex(text);
#endif

	do {
		if (MatchSubString(newAnchorIter, text)
#ifdef	CLIB_HAVE_REGEX
		    || (MatchStr(str, (*newAnchorIter)->name)
			&& MatchPathRegex(reg, (*newAnchorIter)->name))
#endif
		    ) {
			CenterRowPadIfScroll(newAnchorIter);
			FitHighlightMiddle(newAnchorIter);
			curAnchorIter = newAnchorIter;
			break;
		}
		prev_loop(html.GetAnchorList(), newAnchorIter);

	} while (newAnchorIter != curAnchorIter);	/* Loop not repeated */

	Highlight();
	scrnMan.RequestRestCursor();

#ifdef	CLIB_HAVE_REGEX
	if (reg)
		regfree(reg);
#endif
}

void	khHyperWindowWithFind::FindNext(const string &text)
{
	iterType newAnchorIter = curAnchorIter;

	next_loop(html.GetAnchorList(), newAnchorIter);

#ifdef	CLIB_HAVE_REGEX
	string str = MakeString(text);
	regex_t *reg = MakePathRegex(text);
#endif

	do {
		if (MatchSubString(newAnchorIter, text)
#ifdef	CLIB_HAVE_REGEX
		    || (MatchStr(str, (*newAnchorIter)->name)
			&& MatchPathRegex(reg, (*newAnchorIter)->name))
#endif
		    ) {
			CenterRowPadIfScroll(newAnchorIter);
			FitHighlightMiddle(newAnchorIter);
			curAnchorIter = newAnchorIter;
			break;
		}
		next_loop(html.GetAnchorList(), newAnchorIter);

	} while (newAnchorIter != curAnchorIter);	/* Loop not repeated */

	Highlight();
	scrnMan.RequestRestCursor();

#ifdef	CLIB_HAVE_REGEX
	if (reg)
		regfree(reg);
#endif
}

void	khHyperWindowWithFind::SelectText(const string & /*text*/)
{
	HyperWindow::ProcessKey('\n');
}

void	khHyperWindowWithFind::FindProcessKey(int ch)
{
	if (ch == KEY_F(10) || ch == KEY_F(9) || ch == 'C'-'A'+1)
		HyperWindow::ProcessKey(ch);
}

void	khHyperWindowWithFind::ProcessKey(int ch)
{
	bool	start_find = false;
				// Non-special keys

	if (kcdConfig.cfgKey == keyBindingDefault) {
#ifdef USE_UTF8_MODE
		if (IsUTF8Mode()) {
			if ((ch <= 0x7F && isalnum(ch)) || (ch > 0x7F && ch <= 0xFF))
				start_find = true;
		}
		else {
			if (ch <= 0xFF && isalnum(ch))
				start_find = true;
		}
#else
		if (ch <= 0xFF && isalnum(ch))
			start_find = true;
#endif

		if (start_find) {
				// Start find

				// Pass control of keyboard & cursor to
				// status
			scrnMan.SetFocus(&status);
			scrnMan.SetCursor(&status);

			status.ProcessKey(ch);
		}
		else
			HyperWindow::ProcessKey(ch);
	}
	else if (kcdConfig.cfgKey == keyBindingVi) {

		// FIXME: Add *, #

		if (ch == '/' || ch == '?') {
				// Pass control of keyboard & cursor to
				// status
			scrnMan.SetFocus(&status);
			scrnMan.SetCursor(&status);

			status.SetCount(viCount);
			status.ProcessKey(ch);

			viFoundG = false;
			viFoundZ = false;
			viFoundCount = false;
			viCount = 1;
		}
		else
			HyperWindow::ProcessKey(ch);
	}
	else if (kcdConfig.cfgKey == keyBindingEmacs) {
		if (ch == 'S'-'A'+1 || ch == 'R'-'A'+1) {		// C-s, C-r
				// Pass control of keyboard & cursor to
				// status
			scrnMan.SetFocus(&status);
			scrnMan.SetCursor(&status);
			status.ProcessKey(ch);

			emacsFoundAlt = false;
		}
#ifdef USE_UTF8_MODE
		else if (IsUTF8Mode()) {
			if ((ch <= 0x7F && isalnum(ch)) || (ch > 0x7F && ch <= 0xFF))
				start_find = true;
		}
		else {
			if (ch <= 0xFF && isalnum(ch))
				start_find = true;
		}
#else
		else if (ch <= 0xFF && isalnum(ch))
			start_find = true;
#endif

		if (start_find) {
				// Pass control of keyboard & cursor to
				// status
			scrnMan.SetFocus(&status);
			scrnMan.SetCursor(&status);
			status.ProcessKey('S'-'A'+1);	// Assume C-s
			status.ProcessKey(ch);

			emacsFoundAlt = false;

			emacsFoundAlt = false;
		}
		else
			HyperWindow::ProcessKey(ch);
	}
}

Anchor	khHyperWindowWithFind::ProcessCall(Anchor *p)
{
	extern	string	dirFile;

	FILE	*file;

	file = k_fopen(dirFile, "wb");
	if (file == NULL) {			// Cannot create or truncate
						// ~/.kcd.newdir
		throw ErrorGenericFile(_("cannot create file %$"), dirFile);
	}
	
						// Output desired dir
	k_fputs(p->name, file);
	fclose(file);

	return Anchor("", 0, TYPE_EXIT);		// Exit immediately
}

khHyperWindowWithFind::khHyperWindowWithFind(NCScreenManager &scrn_, int id_,
				HyperDocument &html_,
				StatusWindowBase &status_,
				StatusWindowBase &url_,
				ScrollBarBase &hscroll_,
				ScrollBarBase &vscroll_,
				const string &startSection_,
				int lockKey_, int row, int col)
					: HyperWindow(scrn_, id_, html_, 
						status_, url_, hscroll_,
						vscroll_, startSection_,
						lockKey_, row, col)
{
}

khHyperWindowWithFind::~khHyperWindowWithFind()
{
}
