/*
	error.cc	Error Report Functions
	Copyright (c) 1996-9,2000,2001,2002 Kriang Lerdsuwanakij
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

#include "error.h"

#include CXX__HEADER_cctype

/*************************************************************************
	Generic exception classes
*************************************************************************/

		// Define constructors here since they are used very
		// often.  This reduce executable size.
ErrorGeneric::ErrorGeneric(const char *msg) : str(msg)
{
}

ErrorGeneric::ErrorGeneric(Buffer &buf) : str(buf.GetPtr())
{
}

ErrorGeneric::ErrorGeneric(gtstream &buf)
{
	str = buf.str();
}


/*************************************************************************
	Error report functions
*************************************************************************/

ostream& operator<<(ostream &os, const dump_bytes& d)
{
	for (size_t i = d.index; i < d.length && i < d.index+d.numbytes; i++) {
		if (isprint(d.buffer[i]))
			os << d.buffer[i];
		else
			os << '.';
	}
	os << '\n';
	return os;
}
