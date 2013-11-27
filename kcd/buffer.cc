/*
	buffer.cc	Expandable Buffer Class
	Copyright (c) 1997, 1998, 1999, 2000, 2001-2, 2007 Kriang Lerdsuwanakij
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

#include "buffer.h"
#include "error.h"
#include "cxxlib.h"
#include "strmisc.h"

#include CXX__HEADER_cstdio
#include CXX__HEADER_cstdlib
#include CXX__HEADER_cstdarg
#include CXX__HEADER_cassert

Buffer::Buffer(bool autoDelete, size_t incSize)
{
	bufferAddr = NULL;
	bufferPtr = 0;
	bufferSize = 0;
	bufferIncSize = incSize;

	assert(incSize > 0);		// Catch programmer's error
	
	isAutoDelete = autoDelete;
	Expand();
}

Buffer::~Buffer()
{
	if (isAutoDelete) {
		if (bufferAddr)
			free(bufferAddr);
		bufferAddr = NULL;
	}
}

/*************************************************************************
	Routines for stdio-style buffer reading/writing
*************************************************************************/

void	Buffer::Expand()
{
	if (bufferAddr == NULL)
					// Use malloc since we may
					// need to resize it later
		bufferAddr = static_cast<char *>(malloc(bufferIncSize));
	else
		bufferAddr = static_cast<char *>(realloc(bufferAddr, 
							 bufferSize+bufferIncSize));

	if (bufferAddr == NULL)
		throw bad_alloc();
	bufferSize += bufferIncSize;
}

Buffer&	Buffer::Putc(char c)
{
	while (bufferPtr >= bufferSize)
		Expand();
		
	bufferAddr[bufferPtr++] = c;
	if (c)
		return	TermStr();
	else				// Prevent infinite loop since
					// TermStr() also calls Putc

		return	*this;		// No need to terminate string
}

Buffer&	Buffer::Puts(const char *str)
{
	size_t	len = strlen(str);
	while (bufferPtr+len >= bufferSize)	// NUL char. included here
		Expand();
		
	strcpy(bufferAddr+bufferPtr, str);	// String automatically
						// terminated
	bufferPtr += len;
	return	*this;
}

Buffer&	Buffer::Printf(const char *fmt, ...)
{
	va_list	va;
	Buffer	str(1, 1000);		// Should be more than enough...
	va_start(va, fmt);
	vsprintf(str.GetPtr(), fmt, va);
	va_end(va);
	return	Puts(str.GetPtr());
}

Buffer&	Buffer::Clear()
{
	bufferPtr = 0;
	return	TermStr();
}

char&	Buffer::operator[] (size_t i)
{
	while (i >= bufferSize)
		Expand();
		
	return bufferAddr[i];
}
