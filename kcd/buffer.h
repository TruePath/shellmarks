/*
	buffer.h	Expandable Buffer Header
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

#ifndef __K_BUFFER_H
#define __K_BUFFER_H

#include "config.h"

#include "gtstream.h"

const size_t BUF_INC = 20000;	// Default size to grow when Buffer
				// becomes full

class Buffer {
	protected:
		char	*bufferAddr;
		size_t	bufferPtr;	// Index into buffer where next 
					// output commands writes to
		size_t	bufferSize;	// Current buffer size
		bool	isAutoDelete;
		size_t	bufferIncSize;

		void	Expand();

	public:
		Buffer(bool autoDelete = true, size_t incSize = BUF_INC);
		~Buffer();

		Buffer&	Putc(char c);
		Buffer&	Puts(const char *str);
		Buffer&	Puts(const string &str) { return Puts(str.c_str()); }
		Buffer&	Printf(const char *fmt, ...);
		size_t	GetSize()	{ return bufferPtr; }
		size_t	GetAllocSize()	{ return bufferSize; }

		char	*GetPtr()	{ return bufferAddr; }
		char&	operator[] (size_t i);
				// Terminate the string w/o changing length
		Buffer&	TermStr()	{ Putc(0); bufferPtr--; return *this; }
		Buffer&	Clear();
};

#endif	/* __K_BUFFER_H */
