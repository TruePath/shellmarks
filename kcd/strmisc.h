/*
	strmisc.h	String Header
	Copyright (c) 1997-8,2000,2003,2004,2009 Kriang Lerdsuwanakij
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

#ifndef __K_STRMISC_H
#define __K_STRMISC_H

#include "config.h"

#include "error.h"

#ifdef	CLIB_HAVE_REGEX
# include <sys/types.h>
# include <regex.h>
#else
  typedef int regex_t;
#endif

USING_NAMESPACE_STD;

// Throw ErrorRange if error
unsigned	StringToUnsigned(const char *str, char term);

// Case-insensitive comparison
// Replace GNU libc's strcasecmp(...) and strncasecmp(...) if they are
// not available (for non-Linux systems)
#ifndef CLIB_HAVE_STRCASECMP
int	CompareStringCase(const char *str1, const char *str2);
#else
inline int	CompareStringCase(const char *str1, const char *str2)
{
	return strcasecmp(str1, str2);
}
#endif	/* CLIB_HAVE_STRCASECMP */
#ifndef CLIB_HAVE_STRNCASECMP
int	CompareStringCase(const char *str1, const char *str2, size_t n);
#else
inline int	CompareStringCase(const char *str1, const char *str2, size_t n)
{
	return strncasecmp(str1, str2, n);
}
#endif	/* CLIB_HAVE_STRNCASECMP */

inline int	CompareStringCase(const string &str1, const string &str2)
{
	return CompareStringCase(str1.c_str(), str2.c_str());
}

inline int	CompareStringCase(const string &str1, const string &str2,
				  size_t n)
{
	return CompareStringCase(str1.c_str(), str2.c_str(), n);
}

string 	QuoteShellChars(const string &src);	// Quotes special symbols
string	UnquoteShellChars(const string &src, bool quoteGlob = false);	// Remove quotes
string	UnquoteConfigChars(const string &src);	// Remove quotes
string 	QuoteURLChars(const string &src);	// Quotes special symbols
string	UnquoteURLChars(const string &src);	// Remove quotes
string 	QuoteGlobChars(const string &src);	// Quotes special symbols
string 	QuoteRegexChars(const string &src);	// Quotes special symbols

string::size_type	StrLast(const string &str, const string &last);

class k_wstring {
		wchar_t *buf;
		size_t	len;
	public:
		k_wstring() { buf = new wchar_t[1000]; len = 0; buf[0] = 0; }
		~k_wstring() { delete [] buf; }

		size_t		size() const { return len; }
		wchar_t&	operator[](size_t n) { return buf[n]; }
		const wchar_t&	operator[](size_t n) const { return buf[n]; }
		const wchar_t*	data() const { return buf; }

		void	operator=(const char *s);
		void	erase(size_t pos, size_t n);
		void	insert(size_t pos, size_t n, wchar_t c);
};

bool	IsUTF8Mode();
size_t	GetStringWidth(const string &s);
						// FIXME: Use wstring
size_t	GetWStringWidth(const k_wstring &s);
size_t	GetWStringWidth(const k_wstring &s, size_t n);
size_t	GetWStringLastCharAtPos(const k_wstring &s, size_t pos);
size_t	GetWStringWidthLastCharAtPos(const k_wstring &s, size_t pos);

bool	MatchStr(const string &str, const string &dir);
bool	MatchPathRegex(regex_t *reg, const string &dir);
regex_t	*MakePathRegex(const string &search_str);
string	MakeString(const string &search_str);

#endif	/* __K_STRMISC_H */
