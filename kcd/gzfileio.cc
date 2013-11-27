/*
	gzfileio.cc	I/O Functions for zlib Interface
	Copyright (c) 1996,1997-9,2000-2 Kriang Lerdsuwanakij
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

#include "gzfileio.h"
#include "error.h"

/*************************************************************************
	Routines for stdio-style reading/writing gzip file
*************************************************************************/

gzFile	gzopen(const string &path, const char *mode)
{
	return gzopen(path.c_str(), mode);
}

void	k_gzread(gzFile file, char *str, size_t len)
{
	if (gzread(file, str, len) != static_cast<int>(len))
		throw ErrorGZIO();
}

void	k_gzwrite(gzFile file, const char *str, size_t len)
{
			// Need to cast away const-ness
			
	if (gzwrite(file, const_cast<char *>(str), len)
	    != static_cast<int>(len))
		throw ErrorGZIO();
}

void	k_gzputc(gzFile file, char c)
{
	k_gzwrite(file, &c, 1);
}

void	k_gzgetc(gzFile file, char &c)
{
	k_gzread(file, &c, 1);
}

void	k_gzputsu(gzFile file, const char *str)
{
	k_gzwrite(file, str, strlen(str));
}

#if 0
void	k_gzgetsu(gzFile file, char *str, size_t n)
{
	k_gzread(file, str, n);
}

void	k_gzgetsu(gzFile file, Buffer &str, size_t n)
{
	for (size_t i = 0; i < n; i++) {	// Always read firstchar
		char	c;
		k_gzgetc(file, c);
		str.Putc(c);
	}
}

void	k_gzgetsu(gzFile file, Buffer &str)
{
	try {
		for (size_t i = 0; ; i++) {	// Always read firstchar
			char	c;
			k_gzgetc(file, c);
			str.Putc(c);
		}
	}
	catch (...) {
		if (gzeof(file))		// Detect EOF
			return;

		throw;				// Other error
						// Rethrow exception
	}
}
#endif

void	k_gzputs(gzFile file, const char *str)
{
						// With a trailing zero
	k_gzwrite(file, str, strlen(str)+1);
}

void	k_gzputs(gzFile file, const string &str)
{
	k_gzputs(file, str.c_str());
}

#if 0
void	k_gzgets(gzFile file, char *str)
{
	char	notTerm = 1;			// Always read first char
	for (size_t i = 0; notTerm; i++) {
		k_gzgetc(file, str[i]);
		notTerm = str[i];
	}
}

void	k_gzgets(gzFile file, Buffer &str)
{
	char	notTerm = 1;
	for (size_t i = 0; notTerm; i++) {		// Always read firstchar
		char	c;
		k_gzgetc(file, c);
		str.Putc(c);
		notTerm = c;
	}
}
#endif

string	k_gzgets(gzFile file)
{
	string	str;
	for ( ; ; ) {			// Always read firstchar
		char	c;
		k_gzgetc(file, c);
		if (!c)
			break;
		str += c;
	}
	return str;
}

string	k_gzgetsu(gzFile file, size_t n)
{
	string	str;
	for (size_t i = 0; i < n; i++) {	// Always read firstchar
		char	c;
		k_gzgetc(file, c);
		str += c;
	}
	return str;
}

