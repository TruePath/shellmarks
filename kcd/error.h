/*
	error.h		Error Report Functions Header
	Copyright (c) 1996,1997-9,2000,2002 Kriang Lerdsuwanakij
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

#ifndef __K_ERROR_H
#define __K_ERROR_H

#include "config.h"

#include <string>
#include CXX__HEADER_cstring
#include "buffer.h"

USING_NAMESPACE_STD;

/*************************************************************************
	Program return codes
*************************************************************************/

#define RET_OK		0
#define	RET_MEMERR	1	/* Not enough memory */
#define RET_FILEERR	2	/* File I/O error */
#define RET_SYNERR	3	/* File syntax error */
#define RET_ARGERR	4	/* Command line error */
#define RET_LIBERR	5	/* Error due to bad library */
#define RET_OUTERR	6	/* Output error */
#define RET_UNKERR	7	/* Unknown error */
#define RET_RUNERR	8	/* Misc runtime error */

extern	char	progName[];
extern	char	version[];

/*************************************************************************
	Generic exception classes
*************************************************************************/

class ErrorBase {
	public:
		virtual const char *what() { return _("unknown error"); }
		virtual int ret() { return RET_UNKERR; }
		virtual ~ErrorBase() {}
};

class ErrorGeneric : public ErrorBase {
		string	str;
	public:
		ErrorGeneric(const char *msg);
		ErrorGeneric(Buffer &buf);
		ErrorGeneric(gtstream &buf);
		template <class T1> ErrorGeneric(const char *msg, T1 t1);
		template <class T1, class T2> ErrorGeneric(const char *msg, T1 t1, T2 t2);

		const char *what() { return str.c_str(); }
};

template <class T1> ErrorGeneric::ErrorGeneric(const char *msg, T1 t1)
{
	gtstream buf;
	gtout(buf, msg) << t1;
	str = buf.str();
}

template <class T1, class T2> ErrorGeneric::ErrorGeneric(const char *msg, T1 t1, T2 t2)
{
	gtstream buf;
	gtout(buf, msg) << t1 << t2;
	str = buf.str();
}


class ErrorGenericSyntax : public ErrorGeneric {
	public:
		ErrorGenericSyntax(const char *msg) :  ErrorGeneric(msg) {}
		ErrorGenericSyntax(Buffer &buf) : ErrorGeneric(buf) {}
		ErrorGenericSyntax(gtstream &buf) : ErrorGeneric(buf) {}
		template <class T1> ErrorGenericSyntax(const char *msg, T1 t1)
						: ErrorGeneric(msg, t1) {}
		template <class T1, class T2> ErrorGenericSyntax(const char *msg, T1 t1, T2 t2)
						: ErrorGeneric(msg, t1, t2) {}

		int ret() { return RET_SYNERR; }
};

class ErrorGenericCommandLine : public ErrorGeneric {
	public:
		ErrorGenericCommandLine(const char *msg) :  ErrorGeneric(msg) {}
		ErrorGenericCommandLine(Buffer &buf) : ErrorGeneric(buf) {}
		ErrorGenericCommandLine(gtstream &buf) : ErrorGeneric(buf) {}
		template <class T1> ErrorGenericCommandLine(const char *msg, T1 t1)
						: ErrorGeneric(msg, t1) {}
		template <class T1, class T2> ErrorGenericCommandLine(const char *msg, T1 t1, T2 t2)
						: ErrorGeneric(msg, t1, t2) {}

		int ret() { return RET_ARGERR; }
};

class ErrorGenericFile : public ErrorGeneric {
	public:
		ErrorGenericFile(const char *msg) :  ErrorGeneric(msg) {}
		ErrorGenericFile(Buffer &buf) : ErrorGeneric(buf) {}
		ErrorGenericFile(gtstream &buf) : ErrorGeneric(buf) {}
		template <class T1> ErrorGenericFile(const char *msg, T1 t1)
						: ErrorGeneric(msg, t1) {}
		template <class T1, class T2> ErrorGenericFile(const char *msg, T1 t1, T2 t2)
						: ErrorGeneric(msg, t1, t2) {}

		int ret() { return RET_FILEERR; }
};

/*************************************************************************
	Specialized exception classes
*************************************************************************/

class ErrorBadFileFormat : public ErrorBase {
	public:
		const char *what() { return _("bad file format"); }
		int ret() { return RET_SYNERR; }
};

class ErrorIORedirect : public ErrorBase {
	public:
		const char *what() { return _("input/output is redirected or piped"); }
		int ret() { return RET_OUTERR; }
};

class ErrorBadNCurses : public ErrorBase {
	public:
		const char *what() { return _("your ncurses library is bad\n"
					      "Refer to kcd README file for solution"); }
		int ret() { return RET_LIBERR; }
};

class ErrorFileConfigChanged : public ErrorBase {
	public:
		const char *what() { return _("file configuration changed"); }
		int ret() { return RET_RUNERR; }
};

class ErrorScreenTooSmall : public ErrorBase {
	public:
		const char *what() { return _("screen too small"); }
		int ret() { return RET_RUNERR; }
};

class ErrorGZIO : public ErrorBase {
	public:
		const char *what() { return _("gz I/O error"); }
		int ret() { return RET_FILEERR; }
};

class ErrorRange : public ErrorBase {
	public:
		const char *what() { return _("input out of range"); }
		int ret() { return RET_SYNERR; }
};

class ErrorBadSequence : public ErrorBase {
	public:
		const char *what() { return _("bad multibyte sequence"); }
		int ret() { return RET_SYNERR; }
};

class ErrorSeek : public ErrorBase {
	public:
		const char *what() { return _("seek error"); }
		int ret() { return RET_SYNERR; }
};

/*************************************************************************
	Error message printing functions
*************************************************************************/

class dump_bytes {
		const char	*buffer;
		size_t	index, length, numbytes;
		friend	ostream& operator<<(ostream &os, const dump_bytes &);
	public:
		dump_bytes(const char *buffer_, size_t index_, size_t length_, size_t numbytes_) :
			buffer(buffer_), index(index_), length(length_), 
			numbytes(numbytes_) {}
};

ostream& operator<<(ostream &os, const dump_bytes &d);

#endif	/* __K_ERROR_H */
