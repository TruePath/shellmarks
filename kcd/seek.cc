/*
	seek.cc		Char Seek Functions
	Copyright (c) 1996-8,2000-2,2004 Kriang Lerdsuwanakij
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

#include "seek.h"
#include "cxxlib.h"
#include "strmisc.h"

/* 
	Case-sensitive search with escapes and quotes skipped
	Returns:	position (always >= 0) if found
*/
size_t	SeekCharIgnoreEscape(char c, const char *buffer, size_t from, size_t length)
{
	while (from < length) {
		if (buffer[from] == '\\') {
					// `\' Escape found
		
			from++;

					// Unexpected end of buffer
			if (from >= length)
				throw ErrorSeek();

					// Seek for a `\' and found `\'
			if (buffer[from] == '\\' && c == '\\')
				return from;		// Return position of
							// the last `\'
				
			from++;		// Skip it !
		}
		else if (buffer[from] == c) {
			return from;	// Return index where c is found
		}
		else {
					// Check for opening quotes
					
			switch(buffer[from]) {
				case '\"':
						// Get position of closing quote
				
					from = SeekCharIgnoreEscape('\"', buffer, from+1,
							length);
					break;
				case '\'':
						// Get position of closing quote
				
					from = SeekCharIgnoreEscape('\'', buffer, from+1,
							length);
					break;
				case '`':
						// Get position of closing quote
				
					from = SeekCharIgnoreEscape('`', buffer, from+1,
							length);
					break;
			}
			
			from++;		// Skip ordinary char and
					// closing quotes
		}
	}
					// Not found
	throw ErrorSeek();
}


/* 
	Case-sensitive search
	nest = 0 no escapes & quotes interpretation
	       1 with quotes skipped (quotes can only nested once)
	       2 with escapes and quotes skipped (quotes can only nested once)
	Returns:	position (always >= 0) if found
*/
size_t	SeekChar(char c, const char *buffer, size_t from, size_t length, int nest)
{
	if (nest == 2)
		return SeekCharIgnoreEscape(c, buffer, from, length);
		
	while (from < length) {
		if (buffer[from] == c) {
			return from;	// Return index where c is found
		}
		else if (nest) {
					// Check for openning quotes
					
			switch(buffer[from]) {
				case '\"':
						// Get position of closing quote
				
					from = SeekChar('\"', buffer, from+1,
							length);
					break;
				case '\'':
						// Get position of closing quote
				
					from = SeekChar('\'', buffer, from+1,
							length);
					break;
				case '`':
						// Get position of closing quote
				
					from = SeekChar('`', buffer, from+1,
							length);
					break;
			}
			from++;		// Skip ordinary char and
					// closing quotes
		}
		else
			from++;
	}
					// Not found
	throw ErrorSeek();
}

/* 
	Case-insensitive search with escapes and quotes skipped
	Returns:	position (always >= 0) if found
*/
size_t	SeekCaseCharIgnoreEscape(char c, const char *buffer, size_t from, size_t length)
{
	c = toupper(c);
	while (from < length) {
		if (buffer[from] == '\\') {
					// `\' Escape found
		
			from++;

					// Unexpected end of buffer
			if (from >= length)
				throw ErrorSeek();

					// Seek for a `\' and found `\'
			if (buffer[from] == '\\' && c == '\\')
				return from;		// Return position of
							// the last `\'
				
			from++;		// Skip it !
		}
		else if (toupper(buffer[from]) == c) {
			return from;	// Return index where c is found
		}
		else {
					// Check for opening quotes
					
			switch(buffer[from]) {
				case '\"':
						// Get position of closing quote
				
					from = SeekCaseCharIgnoreEscape('\"', buffer, from+1,
							length);
					break;
				case '\'':
						// Get position of closing quote
				
					from = SeekCaseCharIgnoreEscape('\'', buffer, from+1,
							length);
					break;
				case '`':
						// Get position of closing quote
				
					from = SeekCaseCharIgnoreEscape('`', buffer, from+1,
							length);
					break;
			}
			
			from++;		// Skip ordinary char and
					// closing quotes
		}
	}
					// Not found
	throw ErrorSeek();
}


/* 
	Case-insensitive search
	nest = 0 no escapes & quotes interpretation
	       1 with quotes skipped (quotes can only nested once)
	       2 with escapes and quotes skipped (quotes can only nested once)
	Returns:	position (always >= 0) if found
*/
size_t	SeekCaseChar(char c, const char *buffer, size_t from, size_t length, int nest)
{
	c = toupper(c);
	if (nest == 2)
		return SeekCaseCharIgnoreEscape(c, buffer, from, length);
		
	while (from < length) {
		if (toupper(buffer[from]) == c) {
			return from;	// Return index where c is found
		}
		else if (nest) {
					// Check for openning quotes
					
			switch(buffer[from]) {
				case '\"':
						// Get position of closing quote
				
					from = SeekCaseChar('\"', buffer, from+1,
							length);
					break;
				case '\'':
						// Get position of closing quote
				
					from = SeekCaseChar('\'', buffer, from+1,
							length);
					break;
				case '`':
						// Get position of closing quote
				
					from = SeekCaseChar('`', buffer, from+1,
							length);
					break;
			}
			from++;		// Skip ordinary char and
					// closing quotes
		}
		else
			from++;
	}
					// Not found
	throw ErrorSeek();
}

/*
	SeekString - Find str in buffer[from]...buffer[length-1]
	str cannot contain any quote or escape
	Returns:	position (always >= 0) if found
*/

size_t	SeekString(const char *str, const char *buffer, size_t from, size_t length, int nest)
{
	size_t	ret;
	size_t	len = strlen(str);
	for ( ; ; ) {
					// Seek for the first char of the
					// string

		ret = SeekChar(str[0], buffer, from, length, nest);
		if (ret+len <= length) {
			if (strncmp(buffer+ret, str, len) == 0)	// Found 
				return ret;
			from = ret+1;	// Seek from next byte 
		}
	}
}

/*
	SeekCaseString - Find str in buffer[from]...buffer[length-1] 
			(case-insensitive)
	str cannot contain any quote or escape
	Returns:	position (always >= 0) if found
*/

size_t	SeekCaseString(const char *str, const char *buffer, size_t from, size_t length, int nest)
{
	size_t	ret;
	size_t	len = strlen(str);
	for ( ; ; ) {
	
				// Chars does not have upper/lower case
				// representations
		if (toupper(str[0]) == tolower(str[0])) {
			ret = SeekChar(str[0], buffer, from, length, nest);
		}
		else {
			ret = SeekCaseChar(str[0], buffer, from, length, nest);
		}
		if (ret+len <= length) {
			if (CompareStringCase(buffer+ret, str, len) == 0)	// Found
				return ret;
			from = ret+1;	// Seek from next byte
		}
	}
}

/*
	SeekTokenEnd - Return the first position from buffer[from] that  
			separates/ends tokens inside HTML tags.
			<xxx aaa=bbb ccc=ddd>
	Returns:	position (always >= 0) if found
*/

size_t	SeekTokenEnd(const char *buffer, size_t from, size_t length)
{
	while (from < length) {
		if (buffer[from] == ' ' || buffer[from] == '=' ||
		    buffer[from] == '>' || buffer[from] == '\n' ||
		    buffer[from] == '\r' || buffer[from] == '\t')
			return from;
		from++;
	}
					// Not found
	throw ErrorSeek();
}

/*
	SeekConfigTokenEnd - Return the first position from buffer[from] that  
			     separates/ends tokens inside configuration file.
	Returns:	position (always >= 0)
*/

size_t	SeekConfigTokenEnd(const char *buffer, size_t from, size_t length)
{
	while (from < length) {
		if (buffer[from] == ' ' || buffer[from] == ',' ||
		    buffer[from] == '=' || buffer[from] == '\n' ||
		    buffer[from] == '\r' || buffer[from] == '\t')
			return from;
		from++;
	}
	return from;
}

/*
	SeekNonSpace - Return the first position from buffer[from] that is 
			not spaces.
	Returns:	position (always >= 0) if found
*/

size_t	SeekNonSpace(const char *buffer, size_t from, size_t length)
{
	while (from < length) {
		if (buffer[from] != ' ' && buffer[from] != '\t' &&
		    buffer[from] != '\n' && buffer[from] != '\r')
			return from;
		from++;
	}
					// Not found
	return from;
}

/*
	SeekPastEqualSign - Skip spaces, `=' and then spaces
	Returns:	position (always >= 0) if found
*/

size_t	SeekPastEqualSign(const char *buffer, size_t from, size_t length)
{
	from = SeekNonSpace(buffer, from, length);	// Skip spaces
		
	if (buffer[from] != '=')			// Check for a `='
		throw ErrorSeek();
	from++;						// Next char
	
	return SeekNonSpace(buffer, from, length);	// Skip spaces
}

/*
	IsEmpty - Check if buffer contains solely spaces and tabs
	Returns:	true  buffer empty
			false buffer not empty
*/

bool	IsEmpty(const char *buffer, size_t from, size_t length)
{
	for (size_t i = from; i < length; i++) {
		if (buffer[i] != ' ' && buffer[i] != '\t')
			return false;
	}
	return true;
}
