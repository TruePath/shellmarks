/*
	khdoc.h		Hypertext Document Classes Header
	Copyright (c) 1996-8,2000,2001,2002 Kriang Lerdsuwanakij
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

#ifndef __K_KHDOC_H
#define __K_KHDOC_H

#include "config.h"

#include "setupcurses.h"

#ifdef ZLIBAC_NEED_EXTERN_C
extern "C" {
#	include <zlib.h>
}
#else
#	include <zlib.h>
#endif

#include "list.h"
#include "miscobj.h"

const int	TYPE_GOTO = 0;		// Jump to other document
const int	TYPE_SECTION = 1;	// Jump to other section in with doc
const int	TYPE_EXEC = 2;		// Exec command
const int	TYPE_CALL = 3;		// Call ProcessCall(...)
const int	TYPE_EXIT = 4;		// Exit immediately
const int	TYPE_NULL = -1;		// Uninitialized type
const int	TYPE_GOTO_OR_EXEC = -2;	// Temporary undecided between GOTO, EXEC

extern ATTR_TYPE	normalA,	// Normal text
			linkA,		// Hypertext link (normal)
			highlightA,	// Highlighted linkA
			barA,		// Status bar
			urlA,		// URL bar
			headerA,	// <Hx>...</Hx> header
			boldA,		// Bold text
			italicA,	// Italic text
			markA,		// '+' bar
			linkBoldA,	// Hypertext link (bold)
			linkItalicA,	// Hypertext link (italic)
			highlightBoldA,	// Highlighted linkBoldA
			highlightItalicA;	// Highlighted linkItalicA

size_t	GetSourceLine(char *buffer, size_t index);

#define	NUM_COLUMN	1000
#define	NUM_INDENT	10
const int	IND_UL = 0;		/* UL */
const int	IND_OL = 1;		/* OL */
const int	IND_BLOCK = 2;		/* BLOCKQUOTE */
const int	IND_DL = 3;		/* DL */

class	HyperDocument;
class	HyperDraw {
	public:
		virtual void	Draw(HyperDocument &, WINDOW *) = 0;
		virtual ~HyperDraw();
};

class	HyperDocument {
	private:
		static const size_t	ALLOCSIZE = 40960;
		static const size_t	ALLOCINC = 40960;

		int	isInA;		/* Inside <A HREF=...>...</A> ? */
		int	isInPre;	/* Inside <PRE>...</PRE> ? (Possibly > 1) */
		Anchor	*procA;		/* Current processed <A HREF=...> */
		int	curRow, curColumn;	/* Current position */

		char	lineBuffer[NUM_COLUMN];	/* Should be enough */

				// For text indent
		
		int	indentPos[NUM_INDENT];	/* Indent positions */
		int	indentVal[NUM_INDENT];	/* Ordered-list counters */
		int	indentType[NUM_INDENT];	/* Indent type */
		int	numIndent;		/* Current indent level */

						// Handle nesting of <A ...>, <B>, <I>
		int	numNestA;		// Nested link
		int	numNestB;		// Nested tags with bold appearance
		int	numNestI;		// Nested tags with italic appearance

		int	numNestBTag;
		int	numNestITag;
		int	numNestStrongTag;
		int	numNestEmTag;
		int	numNestCiteTag;

				// Physical screen size

		int	scrRow, scrColumn;	/* Size of screen used by
						   HyperDocument */

				// HTML tags processing

		attr_t	GetNestedAttr();
		
		void	IndentLine(WINDOW *pad);
		int	IndentColumn();
		void	NextLine(WINDOW *pad);
		
		void	ProcessLI(char *buffer, size_t length, WINDOW *pad, size_t from);
		void	ProcessA(char *buffer, size_t length, WINDOW *pad, size_t from,
					int curRow, int curColumn);
		void	ProcessHypertext(char *buffer, size_t length,
					WINDOW *pad = NULL);

		int	ReadATag();

				// Buffer containing HTML document
				
		char	*buffer;
		size_t	bufferSize;
		bool	isMyBuffer;		// Determine whether buffer
						// should be freed by the
						// destructor
		HyperDraw	*drawObj;
						// Document created by
						// calling HyperDocument
						// Draw functions

		void	Init();
		void	ReInit();
		void	ReInitFormat();

				// Used by constructors
		void	InternalLoadDocument(char *buffer_, size_t bufferSize_);

#ifndef TRIM_NO_DOC_FILE
		void	InternalLoadDocument(const string &fileName);
#endif
#ifndef TRIM_NO_DOC_STREAM
		void	InternalLoadDocument(FILE *file);
#endif
#if ! (defined(TRIM_NO_DOC_STREAM) && defined(TRIM_NO_DOC_FILE))
		void	ReadFile(gzFile file);	// Used by LoadDocument(...)
#endif

		string	titleText;
		sptr_list<Anchor>	anchorList;
		sptr_list<Anchor>	sectionList;
		int	numRow, numColumn;

	public:
		short	*numChar;
		
		const string &GetTitle() const { return titleText; }
		const sptr_list<Anchor> &GetAnchorList() const { return anchorList; }
		const sptr_list<Anchor> &GetSectionList() const { return sectionList; }
		int	GetNumRow() const { return numRow; }
		int	GetNumColumn() const { return numColumn; }

		HyperDocument(char *buffer_, size_t bufferSize_);
		HyperDocument(HyperDraw *drawObj_);
#ifndef TRIM_NO_DOC_FILE
		explicit	HyperDocument(const string &fileName);
#endif
#ifndef TRIM_NO_DOC_STREAM
		explicit	HyperDocument(FILE *file);
#endif
		~HyperDocument();

		void	LoadDocument(char *buffer_, size_t bufferSize_);
#ifndef TRIM_NO_DOC_FILE
		void	LoadDocument(const string &fileName);
#endif
#ifndef TRIM_NO_DOC_STREAM
		void	LoadDocument(FILE *file);
#endif

		void	FormatDocument(WINDOW *pad, int row, int col);
		void	SetDocumentSize(int docRow, int docSize, int row, int col);

		Anchor	*FindLink(const string &startSection, const string &findLink);
		Anchor	*FindNextLink(const string &startSection, const string &findLink);

	private:
			// No definition for these

		HyperDocument(const HyperDocument &);
		HyperDocument& operator=(const HyperDocument &);

	protected:
		void DrawCharSub(WINDOW *pad, chtype c);
#ifdef USE_UTF8_MODE
		void DrawWCharSub(WINDOW *pad, wchar_t c);
#endif
	public:
		void DrawSetSize(int row, int col);
		void DrawBegin(WINDOW *pad);
		void DrawSetTitle(const char *s);
		void DrawSetTitle(const string &s);
		void DrawSetItalic(WINDOW *pad);
		void DrawClearItalic(WINDOW *pad);
		void DrawSetBold(WINDOW *pad);
		void DrawClearBold(WINDOW *pad);
		void DrawChar(WINDOW *pad, chtype c);
		void DrawEntityChar(WINDOW *pad, entity_id id);
		void DrawString(WINDOW *pad, const char *s);
		void DrawString(WINDOW *pad, const string &s);
		void DrawAName(WINDOW *pad, const char *s);
		void DrawAName(WINDOW *pad, const string &s);
		void DrawACallBegin(WINDOW *pad, const char *s);
		void DrawACallBegin(WINDOW *pad, const string &s);
		void DrawAHrefBegin(WINDOW *pad, const char *s);
		void DrawAHrefBegin(WINDOW *pad, const string &s);
		void DrawAEnd(WINDOW *pad);
};

#endif	/* __K_KHDOC_H */
