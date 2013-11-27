/*
	utf8test.cc	UTF-8 Decoder Stress Test
	Copyright (c) 2000, 2001 Kriang Lerdsuwanakij
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

#define USE_UTF8_MODE
#include "utf8dec.cc"
#include CXX__HEADER_cstdio
#include <unistd.h>

//#define	HEX_DUMP

bool	IsUTF8Mode()
{
	return true;
}

int	main()
{
	mbstate_t	state;
	utf8_decode	decode;
	char		buffer[10];

	int	c;
	while ((c = getchar()) != -1) {
		wchar_t	wc;

		for ( ; ; ) {
			decode.decode(wc, static_cast<unsigned char>(c));
			if (decode.is_need_update()) {
				if (wc <= 0x7F)
					putchar(static_cast<unsigned char>(wc));
				else
#ifdef HEX_DUMP
					printf("[%X]", wc);
#else
					printf("X");
#endif
				break;
			}
			else if (decode.is_need_char())
				break;
			else {
				bool	is_extra_char = decode.is_extra_char();
				decode.new_seq();	// Reset errors
#ifdef HEX_DUMP
				printf("[?]");
#else
				printf("?");
#endif

				if (!is_extra_char)
					break;
			}
		}
	}
}
