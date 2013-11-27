/*
	dirutil.h	Directory Utility Functions Header
	Copyright (c) 2000, 2001, 2003, 2004, 2005 Kriang Lerdsuwanakij
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

#ifndef __K_DIRUTIL_H
#define __K_DIRUTIL_H

#include "config.h"
#include <string>

USING_NAMESPACE_STD;

string	ResolveSymlink(const string &src);
string	RelativeToAbsolutePath(const string &path, const string &cwd);
string	ExpandDirectory(const string &src, bool quote = false, bool regex = false);

#endif	/* __K_DIRUTIL_H */
