/*
	cxxlib.h	Correct C++ Library Problems Header
	Copyright (c) 2000,2001,2002,2007 Kriang Lerdsuwanakij
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

#ifndef __K_CXXLIB_H
#define __K_CXXLIB_H

#include "config.h"

#include CXX__HEADER_iostream

USING_NAMESPACE_STD;

//
// string I/O stream
//

#ifndef CXX__STRING_IO
inline ostream &operator << (ostream &os, const string &s)
{
	return os << s.c_str();
}
#endif

//
// Min/Max functions
//

template<class T> T Min(T a, T b)
{
	if (a > b)
		return b;
	return a;
}

template<class T> T Max(T a, T b)
{
	if (a < b)
		return b;
	return a;
}

#endif	/* __K_CXXLIB_H */
