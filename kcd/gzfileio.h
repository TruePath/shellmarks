/*
	gzfileio.h	I/O Functions for zlib Interface Header
	Copyright (c) 1996,1997-9,2000 Kriang Lerdsuwanakij
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

#ifndef __K_GZFILEIO_H
#define __K_GZFILEIO_H

#include "config.h"

#include CXX__HEADER_cstring

#ifdef ZLIBAC_NEED_EXTERN_C
extern "C" {
#	include <zlib.h>
}
#else
#	include <zlib.h>
#endif

#include "buffer.h"

USING_NAMESPACE_STD;

gzFile	gzopen(const string &path, const char *mode);

//
// These functions return -1 in case of error, 0 for end of file
//

void	k_gzwrite(gzFile file, const char *str, size_t len);
void	k_gzread(gzFile file, char *str, size_t len);

void	k_gzputc(gzFile file, char c);
void	k_gzgetc(gzFile file, char &c);

template<class T> void k_gzputT(gzFile file, T a)
{
	k_gzwrite(file, reinterpret_cast<const char *>(&a), sizeof(T));
}
	
template<class T> void k_gzgetT(gzFile file, T &a)
{
	k_gzread(file, reinterpret_cast<char *>(&a), sizeof(T));
}

void	k_gzputsu(gzFile file, const char *str);
//void	k_gzgetsu(gzFile file, char *str, size_t n);
//void	k_gzgetsu(gzFile file, Buffer &str, size_t n);
//void	k_gzgetsu(gzFile file, Buffer &str);

void	k_gzputs(gzFile file, const char *str);
void	k_gzputs(gzFile file, const string &str);
//void	k_gzgets(gzFile file, char *str);
//void	k_gzgets(gzFile file, Buffer &str);

string	k_gzgets(gzFile file);
string	k_gzgetsu(gzFile file, size_t n);

#ifndef ZLIBAC_HAVE_GZEOF
// Our own version, must use only after gzread returns <= 0.
inline int	gzeof(gzFile file)
{
	char	c;
	if (gzread(file, &c, 1) == 0)
		return 1;
	return 0;
}
#endif

#endif	/* __K_GZFILEIO_H */
