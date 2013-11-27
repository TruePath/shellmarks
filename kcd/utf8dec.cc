/*
	utf8dec.cc	UTF-8 to wchar_t Decoder
	Copyright (c) 2000 Kriang Lerdsuwanakij
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

#include "utf8dec.h"
#include "strmisc.h"

void	utf8_decode::decode(wchar_t &wc, unsigned char c)
{
#ifdef USE_UTF8_MODE
	if (IsUTF8Mode()) {			// Decode UTF-8
		if (char_left == 0) {		// No sequence in progress
			if (c < 0x80 || c > 0xFF) {	// Out of UTF-8 range
				wc = c;
				extra_char = 0;
				need_char = 0;		// End of sequence
				need_update = 1;	// Display should be updated
				return;
			}
			else if (c < 0xC0) {	// 10xxxxxx without 11xxxxxx
				extra_char = 0;
				need_char = 0;		// End of sequence
				need_update = 0;	// Display should be updated
				wc = 0;			// Ignore
				return;
			}
			else if (c < 0xE0) {
					// 110xxxxx 10yyyyyy
				wc = (c & 0x1F) << 6;
				if (c & 0x1E)		// Shouldn't be encoded like this
					is_validated = true;
				else
					is_validated = false;
				char_count = 1;
				char_left = 1;
				extra_char = 0;
				need_char = 1;
				need_update = 0;
				return;
			}
			else if (c < 0xF0) {
					// 1110xxxx 10yyyyyy 10yyyyyy
				wc = (c & 0xF) << 12;
				if (c & 0xF)
					is_validated = true;
				else
					is_validated = false;
				char_count = 2;
				char_left = 2;
				extra_char = 0;
				need_char = 1;
				need_update = 0;
				return;
			}
			else if (c < 0xF8) {
					// 11110xxx 10yyyyyy 10yyyyyy 10yyyyyy
				wc = (c & 0x7) << 18;
				if (c & 0x7)
					is_validated = true;
				else
					is_validated = false;
				char_count = 3;
				char_left = 3;
				extra_char = 0;
				need_char = 1;
				need_update = 0;
				return;
			}
			else if (c < 0xFC) {
					// 111110xx 10yyyyyy 10yyyyyy 10yyyyyy
					// 10yyyyyy
				wc = (c & 0x3) << 24;
				if (c & 0x3)
					is_validated = true;
				else
					is_validated = false;
				char_count = 4;
				char_left = 4;
				extra_char = 0;
				need_char = 1;
				need_update = 0;
				return;
			}
			else if (c < 0xFE) {
					// 1111110x 10yyyyyy 10yyyyyy 10yyyyyy
					// 10yyyyyy 10yyyyyy
				wc = (c & 0x1) << 30;
				if (c & 0x1)
					is_validated = true;
				else
					is_validated = false;
				char_count = 5;
				char_left = 5;
				extra_char = 0;
				need_char = 1;
				need_update = 0;
				return;
			}
			else {		// Bad sequence
				extra_char = 0;
				need_char = 0;
				need_update = 0;
				wc = 0;
				return;
			}
		}
		else {
			if (c < 0x80 || c >= 0xC0) {
				extra_char = 1;
				need_char = 0;
				need_update = 0;
				wc = 0;
				return;
			}

			switch (char_count) {
				case 1:
					break;
				case 2:
					if (char_left == 2 && (c & 0x3F) >= 0x20)
						is_validated = true;
					break;
				case 3:
					if (char_left == 3 && (c & 0x3F) >= 0x10)
						is_validated = true;
					break;
				case 4:
					if (char_left == 4 && (c & 0x3F) >= 0x8)
						is_validated = true;
					break;
				case 5:
					if (char_left == 5 && (c & 0x3F) >= 0x4)
						is_validated = true;
					break;
			}

			wc |= (c & 0x3F) << ((char_left-1)*6);
			--char_left;
			if (char_left) {	// Wait for more input
				extra_char = 0;
				need_char = 1;
				need_update = 0;
				return;
			}

			if (!is_validated) {	// Invalid sequence
				extra_char = 0;
				need_char = 0;
				need_update = 0;
				wc = 0;
				return;
			}
			if (wc >= 0xD800 && wc <= 0xDFFF) {
						// Invalid surrogate usage
				extra_char = 0;
				need_char = 0;
				need_update = 0;
				wc = 0;
				return;
			}
			if (wc == 0xFFFE || wc == 0xFFFF) {
						// Invalid character
				extra_char = 0;
				need_char = 0;
				need_update = 0;
				wc = 0;
				return;
			}

			extra_char = 0;
			need_char = 0;
			need_update = 1;
			return;
		}
	}
	else
#endif
	     {
		wc = c;
		need_char = 0;			// End of sequence
		need_update = 1;		// Display should be updated
		return;
	}
}
