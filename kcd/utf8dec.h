/*
	utf8dec.h	UTF-8 to wchar_t Decoder Header
	Copyright (c) 2000,2001,2007 Kriang Lerdsuwanakij
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

#ifndef	__K_UTF8DEC_H
#define	__K_UTF8DEC_H

#include "config.h"

#ifdef USE_UTF8_MODE
# ifdef HAVE_WCHAR_H
#  ifndef __USE_XOPEN
#   define __USE_XOPEN
#  endif
#  include CXX__HEADER_cwchar
#  include CXX__HEADER_cwctype
# endif
#endif

class	utf8_decode {
		bool	extra_char;
		bool	need_char;
		bool	need_update;
		bool	is_validated;

		int	char_count;
		int	char_left;
	public:
		utf8_decode() { new_seq(); }
		
		void	new_seq() { need_char = false; need_update = false; extra_char = false; char_count = 0; char_left = 0; }
		void	decode(wchar_t &wc, unsigned char c);

					// Return codes
					// need_char	need_update	extra_char
					//     N             N	            N		Bad sequence
					//     N             N              Y		Bad sequence, last
					//						char is a separate
					//						sequence
					//     N             Y              N		Complete sequence
					//     Y             N              N		Incomplete sequence
		bool	is_need_char() { return need_char; }
		bool	is_need_update() { return need_update; }
		bool	is_extra_char() { return extra_char; }

		void	set_need_update() { need_update = true; }
};

#endif	/* __K_UTF8DEC_H */
