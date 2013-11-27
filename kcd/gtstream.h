/*
	gtstream.h	C++ Stream Supporting gettext Usage
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

#ifndef __K_GTSTREAM_H
#define __K_GTSTREAM_H

#include "config.h"

#include <string>
#include CXX__HEADER_vector
#include CXX__HEADER_iostream

USING_NAMESPACE_STD;

#if CXX__HAVE_HEADER_sstream
# include CXX__HEADER_sstream
	typedef ostringstream gtstream;
#else
# include <strstream.h>
	class gtstream : public ostrstream {
		public:
			const char *str() {
				(*this) << ends;
				const char *s = ostrstream::str();
				freeze(false);
				return s;
			}
	};
#endif

class gtout {
		gtout(const gtout &);
	private:
		vector<string>	str_vec;
		vector<bool>	str_flag;
		ostream		&os;
		string		format;
		bool		need_flush;

		void finish();
	public:
		gtout(ostream &os_, const string &fmt);
		gtout(ostream &os_, const char *fmt);

		~gtout() { finish(); }

		int opfx() { return 1; }
		void osfx() {}

		gtout& flush() { need_flush = true; return *this; }

		gtout& put(char c) { return *this << c; }
		gtout& write(const char *s, streamsize n) { return *this << string(s, n); }

		template <class T> gtout& operator<<(T t);

		gtout& operator<<(gtout& (*f)(gtout&)) {
			return (*f)(*this);
		}
		gtout& operator<<(ios& (*f)(ios&)) {
			(*f)(os); return *this;
		}
};

template <class T> gtout& gtout::operator<<(T t)
{
	gtstream	buffer;

					// Do not copy locale for speed reason
//	buffer.copyfmt(*this);		// Transfer states to buffer
//	buffer.clear(rdstate());
	buffer << t;

//	copyfmt(buffer);		// Transfer states from buffer
//	clear(buffer.rdstate());

	str_vec.push_back(buffer.str());
	str_flag.push_back(false);

	return *this;
}

#endif	/* __K_GTSTREAM_H */
