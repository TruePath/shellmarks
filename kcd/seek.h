/*
	seek.h		Char Seek Functions Header
	Copyright (c) 1996-8, 2000-2 Kriang Lerdsuwanakij
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

#ifndef __K_SEEK_H
#define __K_SEEK_H

#include "config.h"

#include CXX__HEADER_cstddef

USING_NAMESPACE_STD;

// nest = 0, no nested " ' `
// nest = 1, nested " ' `
// nest = 2, nested " ' ` with \" \' \` not treated as quotes

size_t	SeekChar(char c, const char *buffer, size_t from, size_t length, int nest = 0);
size_t	SeekCaseChar(char c, const char *buffer, size_t from, size_t length, int nest = 0);
size_t	SeekString(const char *str, const char *buffer, size_t from, size_t length, int nest = 0);
size_t	SeekCaseString(const char *str, const char *buffer, size_t from, size_t length, int nest = 0);
size_t	SeekTokenEnd(const char *buffer, size_t from, size_t length);
size_t	SeekConfigTokenEnd(const char *buffer, size_t from, size_t length);
size_t	SeekNonSpace(const char *buffer, size_t from, size_t length);
size_t	SeekPastEqualSign(const char *buffer, size_t from, size_t length);

bool	IsEmpty(const char *buffer, size_t from, size_t length);

#endif	/* __K_SEEK_H */
