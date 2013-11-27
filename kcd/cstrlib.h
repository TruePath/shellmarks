/*
	cstrlib.h	Add C++ String Handling To C Library Functions Header
	Copyright (c) 2000, 2001, 2009 Kriang Lerdsuwanakij
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

#ifndef __K_CSTRLIB_H
#define __K_CSTRLIB_H

#include "config.h"

#include <string>

#include <pwd.h>
#include CXX__HEADER_cstdlib
#include CXX__HEADER_cstdio

USING_NAMESPACE_STD;

int	k_access(const string &pathname, int mode);
int	k_chdir(const string &path);
struct passwd *k_getpwnam(const string &name);
FILE *k_fopen(const string &path, const char *mode);
int	k_fputs(const string &s, FILE *stream);
int	k_fnmatch(const string &pattern, const string &str, int flags);
int	k_stat(const string &filename, struct stat *buf);
int	k_lstat(const string &filename, struct stat *buf);
string	k_readlink(const string &path);
int	k_system(const string &str);
string	k_getcwd();

#endif	/* __K_CSTRLIB_H */
