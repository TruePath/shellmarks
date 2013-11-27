/*
	editbox.h	Edit Box Header
	Copyright (c) 1996-9,2000 Kriang Lerdsuwanakij
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

#ifndef	__K_EDITBOX_H
#define	__K_EDITBOX_H

#include "config.h"

#include "setupcurses.h"

USING_NAMESPACE_STD;

#include "buffer.h"
#include "strmisc.h"
#include "utf8dec.h"

class EditBox {
	protected:
		WINDOW	*win;
		int	xPos, yPos;
		int	normAttr;
		int	*normAttrPtr;
		size_t	boxLength;

		k_wstring	buffer;
		string		out_buffer;

		size_t	curPos;
		bool	insert;

		bool	modified;	// Input modified?

		wchar_t		unicode_key;
		utf8_decode	decoder;
	public:
		EditBox(WINDOW *w, int x, int y, size_t len);
		void	SetWin(WINDOW *w);
		void	SetAttr(int norm);
		void	SetAttr(int *normPtr);
		void	Show();
		void	DoRestCursor();
		void	Clear();
		int	ProcessKey(int key);
		int	GetInsertStatus() { return insert; }
		const string	&GetString();
		size_t	GetStringLength() { return GetString().size(); }

		bool	NeedChar() { return decoder.is_need_char(); }
		bool	NeedUpdate() { return decoder.is_need_update(); }
};

#endif	/* __K_EDITBOX_H */
