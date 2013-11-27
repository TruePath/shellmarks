/*
	cstrlib.cc	Add C++ String Handling To C Library Functions
	Copyright (c) 2000, 2001, 2004, 2009 Kriang Lerdsuwanakij
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

#include "cstrlib.h"
#include "list.h"

#if HAVE_UNISTD_H
# include <unistd.h>
# include <sys/types.h>
#endif

// fnmatch.h that comes with Apache has different function name
#define ap_fnmatch fnmatch
#include <fnmatch.h>

#include <sys/stat.h>
#include CXX__HEADER_cstdio
#include CXX__HEADER_climits

#include CXX__HEADER_cstring

int	k_access(const string &pathname, int mode)
{
	return access(pathname.c_str(), mode);
}

int	k_chdir(const string &path)
{
	return chdir(path.c_str());
}

struct passwd *k_getpwnam(const string &name)
{
	return getpwnam(name.c_str());
}

FILE *k_fopen(const string &path, const char *mode)
{
	return fopen(path.c_str(), mode);
}

int	k_fputs(const string &s, FILE *stream)
{
	return fputs(s.c_str(), stream);
}

int	k_fnmatch(const string &pattern, const string &str, int flags)
{
//Problem with autoconf 2.50 tests
//#ifdef HAVE_FNMATCH
	return fnmatch(pattern.c_str(), str.c_str(), flags);
//#else
				// FIXME: Add our own fnmatch here?
	return pattern == str;
//#endif
}

int	k_stat(const string &filename, struct stat *buf)
{
	return stat(filename.c_str(), buf);
}

int	k_lstat(const string &filename, struct stat *buf)
{
	return lstat(filename.c_str(), buf);
}

string	k_readlink(const string &path)
{
					// readlink(...) limit
	saptr<char> temp_array(new char[PATH_MAX+1]);
	size_t len = readlink(path.c_str(), temp_array(), PATH_MAX);
	if (len) {
					// Note: readlink does not append
					// NUL to the output string.
		temp_array[len] = 0;
		return string(temp_array());
	}
	else
		return string("");
}

int	k_system(const string &str)
{
	return system(str.c_str());
}

string	k_getcwd()
{
	saptr<char> temp_array(new char[PATH_MAX+1]);
	if (getcwd(temp_array(), PATH_MAX))
		return string(temp_array());
	else
		return string("");
}
