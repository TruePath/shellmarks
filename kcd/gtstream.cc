/*
	gtstream.cc	C++ Stream Supporting gettext Usage
	Copyright (c) 2000, 2001, 2002, 2005 Kriang Lerdsuwanakij
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

#include "gtstream.h"

gtout::gtout(ostream &os_, const string &fmt) : os(os_), format(fmt), need_flush(false)
{
}

gtout::gtout(ostream &os_, const char *fmt) : os(os_), format(fmt), need_flush(false)
{
}

void gtout::finish()
{
	size_t	cur_idx = 0;
	size_t	size = format.size();
	for (size_t i = 0; i < size; ++i) {
		if (format[i] == '%' && i != size-1) {
			switch (format[i+1]) {
				case '%':		// Found "%%"
					os << '%';
					++i;		// Skip the first '%'
					break;
				case '$':		// Found "%$"
					if (cur_idx < str_vec.size()) {
						os << str_vec[cur_idx];
						str_flag[cur_idx] = true;
					}
					++cur_idx;
					++i;		// Skip the first '%'
					break;
				default:		// Should be in the form  "%n$"
					{
						size_t idx = 0;
						for (++i ; i < size && format[i] >= '0' && format[i] <= '9'; ++i) {
							idx = idx * 10 + format[i] - '0';
						}
						if (i < size && format[i] == '$') {
							if (idx-1 < str_vec.size()) {
								os << str_vec[idx-1];
								str_flag[idx-1] = true;
							}
							++cur_idx;
						}
						else {	// Error
							if (need_flush)
								os.flush();
							return;
						}
					}
			}
		}
		else {
			os << format[i];
		}
	}
	
//	size = str_vec.size();			// Print remaining strings
//	for (size_t i = 0; i < size; ++i) {
//		if (! str_flag[i])
//			os << str_vec[i];
//	}

	if (need_flush)
		os.flush();
}

