/*
	strmisc.cc	String Functions
	Copyright (c) 1997-9,2000-1,2003,2004,2007,2009 Kriang Lerdsuwanakij
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

#include "strmisc.h"
#include "list.h"

#ifdef USE_UTF8_MODE
# ifdef HAVE_WCHAR_H
#  ifndef __USE_XOPEN
#   define __USE_XOPEN
#  endif
#  include CXX__HEADER_cwchar
# endif
#endif

#include CXX__HEADER_cstdlib

/*************************************************************************
	String to number conversion
*************************************************************************/

unsigned	StringToUnsigned(const char *str, char term)
{
	char		c;
	unsigned	val = 0;
	bool		empty = true;
	for ( ; ; ) {
		c = *str;
		if (c == term) {
			if (empty)		// No number here
				throw ErrorRange();
			else
				return val;
		}
		empty = false;
		if (c > '9' || c < '0')		// This includes '\0' too
			throw ErrorRange();
		unsigned old_val = val;
		val = val*10+c-'0';
		if (val < old_val)		// Overflow
			throw ErrorRange();
		
		str++;				// Next char
	}
}

/*************************************************************************
	Case insensitive string comparison
*************************************************************************/

#ifndef CLIB_HAVE_STRCASECMP
int	CompareStringCase(const char *str1, const char *str2)
{
	size_t	l1 = strlen(str1);
	
	for (size_t i = 0; i < l1; i++) {
		int	c1 = toupper(str1[i]);
		int	c2 = toupper(str2[i]);
		if (c1 != c2)
			return c1-c2;
	}
	if (l1 == strlen(str2))
		return 0;		// str2 == str1
	else				// str2 > str1
		return -1;		// Any negative value is OK
}
#endif	/* CLIB_HAVE_STRCASECMP */

#ifndef CLIB_HAVE_STRNCASECMP
int	CompareStringCase(const char *str1, const char *str2, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		int	c1 = toupper(str1[i]);
		int	c2 = toupper(str2[i]);
		if (c1 != c2)
			return c1-c2;
		if (c1 == 0)
			return 0;
	}
	return 0;			// str2 == str1
}
#endif	/* CLIB_HAVE_STRNCASECMP */

/*************************************************************************
	Quote/unquote support
*************************************************************************/

char	charNeedGlobQuote[] = "*?[]\\";
char	charNeedRegexQuote[] = "^.[]$()|*+?{}\\";
char	charNeedShellQuote[] = "\'\"`\\";
char	charNoNeedURLQuote[] = "/$-_@.&+-";	// Slash + safe chars
char	charHex[] = "0123456789abcdef";

int	HexToBin(char hex)
{
	if (hex >= '0' && hex <= '9')
		return hex-'0';
	if (hex >= 'a' && hex <= 'f')
		return hex-'a'+10;
	if (hex >= 'A' && hex <= 'F')
		return hex-'A'+10;
	throw ErrorRange();
}

string	QuoteShellChars(const string &src)	// Remove quotes
{
	string::size_type	len = src.size();
	string	dest;
	for (string::size_type i = 0; i < len; i++) {
		unsigned char c = static_cast<unsigned char>(src[i]);
		if (strchr(charNeedShellQuote, c)) {
			dest += '\\';
			dest += c;
		}
		else if (isgraph(c) || c >= 0x80)	// Printable (except
							// space) and UTF-8
							// chars
			dest += c;
		else {
			dest += '\\';
			dest += 'x';
			dest += charHex[c/16];
			dest += charHex[c%16];
		}
	}
	return dest;
}

string	UnquoteShellChars(const string &src, bool quoteGlob)	// Remove quotes
{
	string::size_type	len = src.size();
	string	dest;
	bool	quoted = false;
	for (string::size_type i = 0; i < len; i++) {
		if (quoted) {
			switch (src[i]) {
				case 'x':
					if (len < i+3)
						throw ErrorRange();
					dest += HexToBin(static_cast<unsigned char>(src[i+1]))*16
							   +HexToBin(static_cast<unsigned char>(src[i+2]));
					i += 2;
					break;
				default:
					if (quoteGlob) {
						if (strchr(charNeedGlobQuote, src[i]))
							dest += '\\';
						dest += src[i];
					}
					else
						dest += src[i];
			}
			quoted = false;
		}
		else if (src[i] == '\\') {
			quoted = true;
		}
		else {
			dest += src[i];
		}
	}
	return dest;
}

string	UnquoteConfigChars(const string &src)		// Remove quotes
{
	string::size_type	len = src.size();
	string	dest;
	bool	quoted = false;
	for (string::size_type i = 0; i < len; i++) {
		if (quoted) {
			switch (src[i]) {
				case 'x':
					if (len < i+3)
						throw ErrorRange();
					dest += HexToBin(static_cast<unsigned char>(src[i+1]))*16
							   +HexToBin(static_cast<unsigned char>(src[i+2]));
					i += 2;
					break;
				case '\"':
					dest += src[i];
				default:
					dest += '\\';
					dest += src[i];
			}
			quoted = false;
		}
		else if (src[i] == '\\') {
			quoted = true;
		}
		else {
			dest += src[i];
		}
	}
	return dest;
}

string 	QuoteURLChars(const string &src)	// Quotes special symbols
{
	string::size_type	len = src.size();
	string	dest;
	for (string::size_type i = 0; i < len; i++) {
		if (isalnum(src[i]) || strchr(charNoNeedURLQuote, src[i])) {
			dest += src[i];
		}
		else {
			dest += '%';
			dest += charHex[static_cast<unsigned char>(src[i])/16];
			dest += charHex[static_cast<unsigned char>(src[i])%16];
		}
	}
	return dest;
}

string	UnquoteURLChars(const string &src)	// Remove quotes
{
	string::size_type	len = src.size();
	string	dest;
	for (string::size_type i = 0; i < len; ) {
		if (src[i] == '%') {
			i+=3;			// Skip %XX
			
			dest += HexToBin(static_cast<unsigned char>(src[i-2]))*16
					   +HexToBin(static_cast<unsigned char>(src[i-1]));
		}
		else {
			dest += src[i++];
		}
	}
	return dest;
}

string 	QuoteGlobChars(const string &src)	// Quotes special symbols
{
	string::size_type	len = src.size();
	string	dest;
	for (string::size_type i = 0; i < len; i++) {
		if (strchr(charNeedGlobQuote, src[i])) {
			dest += '\\';
			dest += src[i];
		}
		else {
			dest += src[i];
		}
	}
	return dest;
}

string 	QuoteRegexChars(const string &src)	// Quotes special symbols
{
	string::size_type	len = src.size();
	string	dest;
	for (string::size_type i = 0; i < len; i++) {
		if (strchr(charNeedRegexQuote, src[i])) {
			dest += '\\';
			dest += src[i];
		}
		else {
			dest += src[i];
		}
	}
	return dest;
}

/*************************************************************************
	Check last part of string
*************************************************************************/

string::size_type	StrLast(const string &str, const string &last)
{
	string::size_type	lastLen = last.size();
	string::size_type	strLen = str.size();

				// str too small, match is impossible
	if (strLen < lastLen)
		return string::npos;
	
	string::size_type	i, j;
	for (i = strLen-lastLen, j = 0; i < strLen; i++, j++) {
		if (str[i] != last[j])
			return string::npos;	// Not match
	}
	return strLen-lastLen;			// Match, return position
}

/*************************************************************************
	Unicode support
*************************************************************************/

void	k_wstring::operator=(const char *s)
{
	len = 0;
	while(*s) {
		buf[len] = *s;
		++s;
		++len;
	}
	buf[len] = 0;
}

void	k_wstring::erase(size_t pos, size_t n)
{
	size_t	i;
	for (i = pos; i+n < len; ++i) {
		buf[i] = buf[i+n];
	}
	buf[i] = 0;
	len = i;
}

void	k_wstring::insert(size_t pos, size_t n, wchar_t c)
{
	if (len > 0)			// String not empty
		for (size_t i = len-1; i >= pos; --i) {
					// Shift the remainder of the 
					// string right
			buf[i+n] = buf[i];

			if (i == 0)	// Avoid overflow problem for --i
				break;
		}
					// Fill character
	for (size_t i = pos; i < pos+n; ++i) {
		buf[i] = c;
	}
	len += n;
	buf[len] = 0;			// Terminate string
}


bool	IsUTF8Mode()
{
#ifdef USE_UTF8_MODE
	static bool	utf8_mode_init = false;
	static bool	utf8_mode = false;
	if (!utf8_mode_init) {
		char *s;
		if ((s = getenv("LC_ALL")) != 0
		    || (s = getenv("LC_CTYPE")) != 0
		    || (s = getenv("LANG")) != 0) {
			if (strstr(s, "UTF-8"))
				utf8_mode = true;
		}
		utf8_mode_init = true;
	}
	return utf8_mode;
#else
	return false;
#endif
}


size_t	GetStringWidth(const string &s)
{
#ifdef USE_UTF8_MODE
	static mbstate_t state;

	if (IsUTF8Mode()) {
				// Space enough to hold converted string
				// and the string termination.
		saptr<wchar_t> buf(new wchar_t[s.size()+1]);
		const char *p = s.c_str();

		memset(&state, 0, sizeof(mbstate_t));
		size_t	num = mbsrtowcs(buf(), &p, s.size()+1, &state);
		if (num == static_cast<size_t>(-1))
			throw ErrorGenericSyntax(_("bad multibyte sequence %$"), s);

		size_t size = 0;
		for (size_t i = 0; i < num; ++i)
			size += wcwidth(buf[i]);
		return size;
	}
	else
#endif
		return s.size();
}

size_t	GetWStringWidth(const k_wstring &s)
{
#ifdef USE_UTF8_MODE
	if (IsUTF8Mode()) {
		size_t size = 0;
		size_t ssize = s.size();
		for (size_t i = 0; i < ssize; ++i) {
			size += wcwidth(s[i]);
		}
		return size;
	}
	else
#endif
		return s.size();
}

size_t	GetWStringWidth(const k_wstring &s, size_t n)
{
	size_t ssize = s.size();
#ifdef USE_UTF8_MODE
	if (IsUTF8Mode()) {
		size_t size = 0;
		for (size_t i = 0; i < ssize && i < n; ++i) {
			size += wcwidth(s[i]);
		}
		return size;
	}
	else
#endif
		return (n > ssize) ? ssize : n;
}

size_t	GetWStringLastCharAtPos(const k_wstring &s, size_t pos)
{
	size_t ssize = s.size();
#ifdef USE_UTF8_MODE
	if (IsUTF8Mode()) {
		size_t i = 0;
		for (size_t size = 0; i < ssize; ++i) {
			if (size >= pos) {
				while (i < ssize && wcwidth(s[i]) == 0)
					++i;
				return i;
			}
			size += wcwidth(s[i]);
		}
		return i;
	}
	else
#endif
		return (pos > ssize) ? ssize : pos;
}

size_t	GetWStringWidthLastCharAtPos(const k_wstring &s, size_t pos)
{
	return GetWStringWidth(s, GetWStringLastCharAtPos(s, pos));
}

/*************************************************************************
	/, // Regex Directory Match
*************************************************************************/

bool	MatchPathRegex(regex_t *reg, const string &dir)
{
#ifdef	CLIB_HAVE_REGEX
	if (!reg)
		return true;

	regmatch_t m;
	return !regexec(reg, dir.c_str(), 1, &m, 0);
#else
	return true;
#endif
}

bool	MatchStr(const string &str, const string &dir)
{
	size_t slash_pos = dir.rfind('/');
	size_t link_pos;
	if (slash_pos != string::npos)
		link_pos = dir.find(str, slash_pos+1);
	else
		link_pos = dir.find(str);

	return link_pos != string::npos;
}

regex_t	*MakePathRegex(const string &search_str)
{
#ifdef	CLIB_HAVE_REGEX
	size_t orig_pos = 0;
	size_t slash_pos = search_str.find('/');
	if (slash_pos == string::npos)
		return 0;

	string regex = ".*";

	do {
		regex.append(QuoteRegexChars(search_str.substr(orig_pos, slash_pos-orig_pos)));
		if (search_str[slash_pos+1] == '/') {
			regex.append(".*/.*");
			slash_pos++;
		}
		else
			regex.append("[^/]*/[^/]*");

		orig_pos = slash_pos+1;
		slash_pos = search_str.find('/', orig_pos);
	} while (slash_pos != string::npos);

	regex.append(1, '$');
	regex_t *reg = new regex_t;
	if (regcomp(reg, regex.c_str(), REG_EXTENDED))
		return 0;		// Error
	return reg;
#else
	return 0;
#endif
}

string	MakeString(const string &search_str)
{
#ifdef	CLIB_HAVE_REGEX
	size_t slash_pos = search_str.rfind('/');
	if (slash_pos == string::npos)
		return search_str;
	else
		return string(search_str, slash_pos+1);
#else
	return search_str;
#endif
}
