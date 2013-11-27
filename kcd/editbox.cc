/*
	editbox.cc	Edit Box
	Copyright (c) 1996-9,2000,2001,2004,2009 Kriang Lerdsuwanakij
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

#include "editbox.h"
#include "confobj.h"
#include CXX__HEADER_cctype

EditBox::EditBox(WINDOW *w, int x, int y, size_t len)
{
	win = w;
	xPos = x;
	yPos = y;

	normAttr = A_NORMAL;		// Default attribute
	normAttrPtr = &normAttr;	// Point to our own copy of attribute

	modified = true;

	boxLength = len;
	Clear();
}

void	EditBox::SetWin(WINDOW *w)
{
	win = w;
}

void	EditBox::SetAttr(int norm)
{
	normAttr = norm;
	normAttrPtr = &normAttr;	// Point to our own copy of attribute
}

void	EditBox::SetAttr(int *normPtr)
{
	normAttrPtr = normPtr;
}

void	EditBox::Show()
{
	attr_t	oldAttr;
	short oldPair;
	my_wattr_get(win, &oldAttr, &oldPair);
	
	wattrset(win, *normAttrPtr);
	wmove(win, yPos, xPos);		// Clear space
	for (size_t i = 0; i <= boxLength; i++)
		waddch(win, ' ');
		
	wmove(win, yPos, xPos);		// Print input

#ifdef USE_UTF8_MODE
	if (IsUTF8Mode()) {
			waddnwstr(win, buffer.data(), buffer.size());
	}
	else
#endif
	{
		for (size_t j = 0; j < buffer.size(); j++)
			waddch(win, buffer[j]);
	}
	wmove(win, yPos, xPos+curPos);		// Place cursor
	my_wattr_set(win, oldAttr, oldPair);
}

void	EditBox::DoRestCursor()
{
	wmove(win, yPos, xPos+curPos);		// Place cursor
}

void	EditBox::Clear()
{
	buffer = "";

	decoder.new_seq();			// Discard partially decoded sequence
	modified = true;
	decoder.set_need_update();
	unicode_key = 0;
		
	curPos = 0;
	insert = true;
}

int	EditBox::ProcessKey(int key)
{
	if (key <= 0xFF) {
		for ( ; ; ) {
			decoder.decode(unicode_key, key);
			if (decoder.is_need_update())		// Complete sequence
				break;				// Continue after this
								// if block
			else if (decoder.is_need_char())	// Incomplete sequence
				return 0;
			else {
				bool	is_extra_char = decoder.is_extra_char();
				decoder.new_seq();		// Reset errors
				unicode_key = 0;

				ProcessKey('?');		// Display '?'
				if (!is_extra_char)
					return 0;
			}
		}
	}
	else {					// ncurses control keys
		bool incomplete = decoder.is_need_char();

		decoder.new_seq();		// Discard partially decoded sequence
		unicode_key = 0;

		if (incomplete)
			ProcessKey('?');	// Display '?'

						// Continue processing control keys
	}

	int	ret = 0;

	if (key == 0 || key == '\r' || key == '\n' || key == '\t')
		;
	else if (key == KEY_HOME
		 || (kcdConfig.cfgKey == keyBindingEmacs && key == 'A'-'A'+1)) { // C-a
		curPos = 0;
		wmove(win, yPos, xPos+curPos);
	}
	else if (key == KEY_END
		 || (kcdConfig.cfgKey == keyBindingEmacs && key == 'E'-'A'+1)) { // C-e
		curPos = GetWStringWidth(buffer);
		wmove(win, yPos, xPos+curPos);
	}
	else if (key == KEY_LEFT
		 || (kcdConfig.cfgKey == keyBindingEmacs && key == 'B'-'A'+1)) { // C-b
		if(curPos > 0)
			curPos = GetWStringWidthLastCharAtPos(buffer, curPos-1);
		wmove(win, yPos, xPos+curPos);
	}
	else if (key == KEY_RIGHT
		 || (kcdConfig.cfgKey == keyBindingEmacs && key == 'F'-'A'+1)) { // C-f
		if(curPos < GetWStringWidth(buffer))
			curPos = GetWStringWidthLastCharAtPos(buffer, curPos+1);
		wmove(win, yPos, xPos+curPos);
	}
	// FIXME: Add emacs M-f, M-b to forward/backword word
	// M-d M-<DEL> kill forward/backward word
	// FIXME: Add vi CTRL-W delete word in front of cursor
	// Shift-left/right backward/forward word
	// CTRL-V char - insert char
	// CTRL-V num num num - 
	else if (key == KEY_IC)
		insert = !insert;
	else if (key == KEY_DC
		 || (kcdConfig.cfgKey == keyBindingEmacs && key == 'D'-'A'+1)) { // C-d
		if(curPos < GetWStringWidth(buffer) && buffer.size()) {

			size_t	del_pos = GetWStringLastCharAtPos(buffer, curPos+1);
			buffer.erase(del_pos-1, 1);

			Show();
			ret = 1;

			modified = true;
			decoder.set_need_update();
		}
	}
	else if (kcdConfig.cfgKey == keyBindingVi && key == 'U'-'A'+1) { // ^U
						// Empty buffer
		buffer.erase(0, buffer.size());
		curPos = 0;

		Show();
		ret = 1;

		modified = true;
		decoder.set_need_update();
	}
	else if (key == 0177			// Backspace
		 || key == '\b' || key == KEY_BACKSPACE) {
		if(curPos > 0) {

			size_t	del_pos = GetWStringLastCharAtPos(buffer, curPos);
			buffer.erase(del_pos-1, 1);
			curPos = GetWStringWidth(buffer, del_pos-1);

			Show();
			ret = 1;

			modified = true;
			decoder.set_need_update();
		}
	}
	else if (key > 255)	// Ignore control keys, cannot be part of UTF-8 sequence
		;
	else {
					// Insert or overtype at the end of string
		if (insert || curPos == GetWStringWidth(buffer)) {

					// Enough room for a new character
#ifdef USE_UTF8_MODE
			if (GetWStringWidth(buffer)+wcwidth(unicode_key) <= boxLength) {
				if (iswprint(unicode_key)) {
#else
			if (GetWStringWidth(buffer)+1 <= boxLength) {
				if (isprint(unicode_key)) {
#endif
								// Get this character position
					size_t	ins_pos = GetWStringLastCharAtPos(buffer, curPos);
					buffer.insert(ins_pos, 1, unicode_key);
					curPos = GetWStringWidth(buffer, ins_pos+1);
					Show();
					ret = 1;

					modified = true;
					decoder.set_need_update();
				}
			}
		}
					// Overtype at the beginning/middle of string
		else if (buffer.size()) {

#ifdef USE_UTF8_MODE
			if (iswprint(unicode_key)) {
#else
			if (isprint(unicode_key)) {
#endif

				size_t	new_pos = curPos;
				size_t	del_pos;
				do {
					del_pos = GetWStringLastCharAtPos(buffer, curPos+1);
					buffer.erase(del_pos-1, 1);
					new_pos = GetWStringWidth(buffer, del_pos-1);
				} while (new_pos > curPos);
				buffer.insert(del_pos-1, 1, unicode_key);
				curPos = GetWStringWidth(buffer, del_pos);

				Show();
				ret = 1;

				modified = true;
				decoder.set_need_update();
			}
		}
	}

	return ret;
}

const string	&EditBox::GetString()
{
#ifdef USE_UTF8_MODE
	static mbstate_t state;
#endif
	if (modified) {
#ifdef USE_UTF8_MODE
		if (IsUTF8Mode()) {
			out_buffer = "";
			Buffer	buf(1000);
			for (size_t i = 0; i < buffer.size(); ++i) {
				memset(&state, 0, sizeof(mbstate_t));
				if (wcrtomb(buf.GetPtr(), buffer[i], &state)
				    == static_cast<size_t>(-1))
					throw ErrorBadSequence();
				out_buffer += buf.GetPtr();
			}
		}
		else
#endif
		     {
			out_buffer = "";
			for (size_t i = 0; i < buffer.size(); ++i)
				out_buffer += buffer[i];
		}
		modified = false;
	}
	return out_buffer;
}
