/*
	khdoc.cc	Hypertext Document Classes
	Copyright (c) 1996-9,2000-2,2003,2007,2009 Kriang Lerdsuwanakij
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

/*
	Current supported HTML tags:
		<!--...-->
		<A HREF="...">...</A>
		<A HREF="#...">...</A>
		<A HREF="...#...">...</A>
		<A NAME="...">...</A>   (</A> optional but recommended)
		<B>...</B>
		<BLOCKQUOTE>...</BLOCKQUOTE>
		<BR>
		<CITE>...</CITE>
		<DL><DT><DD>...</DL>
		<EM>...</EM>
		<HR>
		<I>...</I>
		<LI>
		<OL>...</OL>
		<P>...</P>
		<PRE>...</PRE>
		<STRONG>...</STRONG>
		<TITLE>...</TITLE>	(Max 50 chars for 80 columns,
					 escapes not allowed)
		<UL>...</UL>
		&lt; &gt; &amp; &quot; and all ISO 8859-1 character set

		&#...;

	Current partially supported tags:
		<H?>...</H?>

	Current supported non-HTML tags:
		<A HREF=`...`>...</A>
		<A CALL="...">...</A>
		&boxur; &boxdr; &boxdl; &boxul;
		&boxvl; &boxvr; &boxhu; &boxhd;
		&boxh; &boxv; &boxvh;
		&rarr; &larr; &darr; &uarr;
*/

/*
	To do:
		- Various control chars (LF, CR, tab) in title not allowed.
		- Handle format & escape in title.
		- Does not support nested <A ...> tags.
		- Space part of link left at the end of line after wrap.
		- Line with over 1000 columns will not be properly wrap.  Make 
		  some arrays dynamically allocated.
		- Cannot handle large files > 1M, take too much RAM.
		- Make classes more suitable for inheritance.
		- Remove hardcoded NUM_COLUMN limit.
		- Replace NUM_INDENT limit with STL stack.
*/

#include "khdoc.h"
#include "cstrlib.h"
#include "gzfileio.h"
#include "seek.h"
#include "strmisc.h"
#include "gtstream.h"

#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/stat.h>

#ifdef USE_UTF8_MODE
# ifdef HAVE_WCHAR_H
#  ifndef __USE_XOPEN
#   define __USE_XOPEN
#  endif
#  include CXX__HEADER_cwchar
#  include CXX__HEADER_cwctype
# endif
#endif

#include CXX__HEADER_fstream

// #define DEBUG_FORMAT_TEXT

/*************************************************************************
	Hypertext functions
*************************************************************************/

#ifdef TRIM_NO_CHECK

inline void	ThrowErrorTagInside(const string &/*tag1*/, const string &/*tag2*/, char * /*buffer*/, int /*from*/, int /*length*/)
{
}

inline void	ThrowErrorNestedTag(const string &/*tag*/, char * /*buffer*/, int /*from*/, int /*length*/)
{
}

inline void	ThrowErrorTagWithout(const string &/*tag1*/, const string &/*tag2*/, char * /*buffer*/, int /*from*/, int /*length*/)
{
}

inline void	ThrowErrorSymbolNeeded(const string &/*tag1*/, const string &/*tag2*/, char * /*buffer*/, int /*from*/, int /*length*/)
{
}

inline void	ThrowErrorEndOfFile(const string &/*tag*/, char * /*buffer*/, int /*from*/, int /*length*/)
{
}

inline void	ThrowErrorCannotFind(const string &/*tag*/, char * /*buffer*/, int /*from*/, int /*length*/)
{
}

inline void	ThrowErrorMessage(const string &/*msg*/, char * /*buffer*/, int /*from*/, int /*length*/)
{
}

#else

// Find line number given index of buffer

size_t	GetSourceLine(char *buffer, size_t index)
{
	size_t	lineNo = 1;
	for (size_t i = 0; i <= index; i++)
		if (buffer[i] == '\n')		// End of line found
			lineNo++;
	return lineNo;
}

void	ThrowErrorMessage(const string &msg, char *buffer, size_t from, size_t length)
{
	gtstream bufstr;
	gtout(bufstr, msg) << GetSourceLine(buffer, from);
	bufstr << _("Content:\n")) << dump_bytes(buffer, from, length, 70);
	throw ErrorGenericSyntax(bufstr);
}

void	ThrowErrorTag1(const string &tag, const string &msg, char *buffer, size_t from, size_t length)
{
	gtstream bufstr;
	gtout(bufstr, msg) << tag << GetSourceLine(buffer, from);
	bufstr << _("Content:\n")) << dump_bytes(buffer, from, length, 70);
	throw ErrorGenericSyntax(bufstr);
}

void	ThrowErrorTag2(const string &tag1, const string &tag2, const string &msg, char *buffer, size_t from, size_t length)
{
	gtstream bufstr;
	gtout(bufstr, msg) << tag1 << tag2 << GetSourceLine(buffer, from);
	bufstr << _("Content:\n")) << dump_bytes(buffer, from, length, 70);
	throw ErrorGenericSyntax(bufstr);
}

void	ThrowErrorTagInside(const string &tag1, const string &tag2, char *buffer, size_t from, size_t length)
{
	ThrowErrorTag2(tag1, tag2, _("%$ tag inside %$ tag at line %$\n"), buffer, from, length);
}

void	ThrowErrorNestedTag(const string &tag, char *buffer, size_t from, size_t length)
{
	ThrowErrorTag1(tag, _("nested %$ tag at line %$\n"), buffer, from, length);
}

void	ThrowErrorTagWithout(const string &tag1, const string &tag2, char *buffer, size_t from, size_t length)
{
	ThrowErrorTag2(tag1, tag2, _("%$ without %$ at line %$\n"), buffer, from, length);
}

void	ThrowErrorSymbolNeeded(const string &tag1, const string &tag2, char *buffer, size_t from, size_t length)
{
	ThrowErrorTag2(tag1, tag2, _("%$ needed %$ at line %$\n"), buffer, from, length);
}

void	ThrowErrorEndOfFile(const string &tag, char *buffer, size_t from, size_t length)
{
	ThrowErrorTag1(tag, _("unexpected end of file inside %$ at line %$\n"), buffer, from, length);
}

void	ThrowErrorCannotFind(const string &tag, char *buffer, size_t from, size_t length)
{
	ThrowErrorTag1(tag, _("cannot find %$ at line %$\n"), buffer, from, length);
}

#endif	/* TRIM_NO_CHECK */

/*************************************************************************
	HyperDraw class
*************************************************************************/

HyperDraw::~HyperDraw()
{
}

/*************************************************************************
	Hypertext class
*************************************************************************/

// Static member
const size_t	HyperDocument::ALLOCSIZE;
const size_t	HyperDocument::ALLOCINC;

void	HyperDocument::Init()		// Used by constructors
{
	numChar = NULL;
	numIndent = 0;
}

void	HyperDocument::ReInit()		// Used by LoadDocument's
{
	if (isMyBuffer)			// Make sure buffer is allocated by
					// HyperDocument
		if (buffer)		// Make sure buffer is not NULL
			free(buffer);
	buffer = NULL;
	isMyBuffer = false;		// No longer own the buffer

	ReInitFormat();
}

void	HyperDocument::ReInitFormat()
{
	delete [] numChar;	// Deleting NULL is OK
	numChar = NULL;
	
	anchorList.erase(anchorList.begin(), anchorList.end());
	sectionList.erase(sectionList.begin(), sectionList.end());
	titleText.erase();

	// Document-dependent variables

	isInA = 0;
	isInPre = 0;
	procA = NULL;

	curRow = 0;
	curColumn = 0;
}

//------------------------------------------------------------------------
// Load document from buffer
//------------------------------------------------------------------------

HyperDocument::HyperDocument(char *buffer_, size_t bufferSize_)
{
	Init();
	InternalLoadDocument(buffer_, bufferSize_);
}

void	HyperDocument::LoadDocument(char *buffer_, size_t bufferSize_)
{
	ReInit();
	InternalLoadDocument(buffer_, bufferSize_);
}

void	HyperDocument::InternalLoadDocument(char *buffer_, size_t bufferSize_)
{
	buffer = buffer_;
	bufferSize = bufferSize_;
	isMyBuffer = false;
	drawObj = 0;
}

//------------------------------------------------------------------------
// Empty document for run-time generation
//------------------------------------------------------------------------

HyperDocument::HyperDocument(HyperDraw *drawObj_)
{
	Init();

	buffer = 0;
	bufferSize = 0;
	isMyBuffer = true;
	drawObj = drawObj_;
}

//------------------------------------------------------------------------
// Load document from the file given
//------------------------------------------------------------------------

#ifndef TRIM_NO_DOC_FILE
HyperDocument::HyperDocument(const string &fileName)
{
	Init();
	InternalLoadDocument(fileName);
}

void	HyperDocument::LoadDocument(const string &fileName)
{
	ReInit();
	InternalLoadDocument(fileName);
}

void	HyperDocument::InternalLoadDocument(const string &fileName)
{
	isMyBuffer = true;
	buffer = NULL;				// Not allocated yet
	drawObj = 0;

	gzFile	fileGZ1 = NULL, fileGZ2 = NULL;	// Avoid warning
	bool	isDir1 = true, isDir2 = true;	// Default to true when
						// file not found
	struct stat	fileStat;

	fileGZ1 = gzopen(fileName, "rb");
	try {
		if (fileGZ1) {
			k_stat(fileName, &fileStat);
			isDir1 = S_ISDIR(fileStat.st_mode);
		}

				// File not found or is a directory 
		if (fileGZ1 == NULL || isDir1) {
	
			string newFileName = fileName;
		
				// Try a name with `.gz' added or removed
			string::size_type pos = StrLast(fileName, ".gz");
			if (pos != string::npos)
		    					// Remove `.gz'
#ifdef _CXX_STRING_ERASE
				newFileName.erase(pos, 3);
#else
			    		// Same as newFileName.erase(pos, 3);
			    	newFileName.replace(pos, 3,
					static_cast<string::size_type>(0),
			    		0);
#endif				
			else
							// Add `.gz'
				newFileName += ".gz";

			fileGZ2 = gzopen(newFileName, "rb");
			if (fileGZ2) {
				k_stat(newFileName, &fileStat);
				isDir2 = S_ISDIR(fileStat.st_mode);
			}
		
					// Both are not found or are directories
			if (isDir1 && isDir2) {
				throw ErrorGenericFile(_("cannot read file %$ or %$"),
						       fileName, newFileName);
			}

			ReadFile(fileGZ2);
		}
		else
			ReadFile(fileGZ1);
	}
	catch (...) {
		if (fileGZ1)
			gzclose(fileGZ1);
		if (fileGZ2)
			gzclose(fileGZ2);
	}
	if (fileGZ1)
		gzclose(fileGZ1);
	if (fileGZ2)
		gzclose(fileGZ2);
}
#endif	/* TRIM_NO_DOC_FILE */

//------------------------------------------------------------------------
// Load document from opened file stream
//------------------------------------------------------------------------

#ifndef TRIM_NO_DOC_STREAM
HyperDocument::HyperDocument(FILE *file)
{
	Init();
	InternalLoadDocument(file);
}

void	HyperDocument::LoadDocument(FILE *file)
{
	ReInit();
	InternalLoadDocument(file);
}

void	HyperDocument::InternalLoadDocument(FILE *file)
{
	isMyBuffer = true;
	buffer = NULL;			// Not allocated yet
	drawObj = 0;

	gzFile	fileGZ;

	fileGZ = gzdopen(fileno(file), "rb");
	if (fileGZ == NULL)
		throw ErrorGenericFile(_("cannot read file stream"));
	try {
		ReadFile(fileGZ);
	}
	catch (...) {
		gzclose(fileGZ);
	}
	gzclose(fileGZ);
}
#endif	/* TRIM_NO_DOC_STREAM */

//------------------------------------------------------------------------
// File I/O
//------------------------------------------------------------------------

#if ! (defined(TRIM_NO_DOC_STREAM) && defined(TRIM_NO_DOC_FILE))
void	HyperDocument::ReadFile(gzFile file)
{
	size_t	bufferPtr = 0;
	int	byteRead;		// Must be int, gzread returns -1
					// upon failure
	size_t	readSize = ALLOCSIZE;

	if (buffer) {
		delete buffer;
	}

	buffer = static_cast<char *>(malloc(ALLOCSIZE));// Allocate buffer
							// Need malloc
							// because of realloc
							// below
	bufferSize = ALLOCSIZE;
	if (buffer == NULL)
		throw bad_alloc();

	while((byteRead = gzread(file, buffer+bufferPtr, readSize)) ==
	      static_cast<int>(readSize)) {
		char *newBuffer;
		bufferPtr += byteRead;		// Next read position
		bufferSize += ALLOCINC;		// New buffer size
		readSize = ALLOCINC;		// Number of bytes read
						// Expand memory block
		newBuffer = static_cast<char *>(realloc(buffer, bufferSize));
		if (newBuffer == NULL)
			throw bad_alloc();
			
		buffer = newBuffer;
	}
	if (byteRead == -1)
		throw ErrorGenericFile(_("error occurs while reading file"));

	bufferSize = bufferSize-readSize+byteRead;
}
#endif

//------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------

HyperDocument::~HyperDocument()
{
	delete [] numChar;		// Deleting NULL is OK
	
	if (isMyBuffer)
		if (buffer)
			free(buffer);
}

//------------------------------------------------------------------------
// Format document
//------------------------------------------------------------------------

attr_t	HyperDocument::GetNestedAttr()
{
	if (numNestA) {
		if (numNestB)
			return linkBoldA;
		if (numNestI)
			return linkItalicA;
		return linkA;
	}
	else {
		if (numNestB)
			return boldA;
		if (numNestI)
			return italicA;
		return normalA;
	}
}

void	HyperDocument::IndentLine(WINDOW *pad)
{
#ifdef TRIM_NO_INDENT
	if (numIndent > 0 && numIndent <= NUM_INDENT) {
		if (pad) {
			attr_t	oldAttr;
			short oldPair;
			my_wattr_get(pad, &oldAttr, &oldPair);
			wattrset(pad, normalA);
			wmove(pad, curRow, curColumn);
			for (int i = 0; i < indentPos[numIndent-1]; i++) {
				waddch(pad, ' ');
			}
			my_wattr_set(pad, oldAttr, oldPair);
			curColumn = indentPos[numIndent-1];
		}
		else
			curColumn = indentPos[numIndent-1];
	}
	else if (numIndent == 0)
#endif
		curColumn = 0;

	if (pad)				// Move cursor
		wmove(pad, curRow, curColumn);
}

int	HyperDocument::IndentColumn()
{
#ifdef TRIM_NO_INDENT
	if (numIndent > 0)
		return indentPos[numIndent-1];
#endif
	return 0;		// No indent
}

#ifdef TRIM_NO_INDENT
void	HyperDocument::ProcessLI(char * /*buffer*/, size_t /*length*/, WINDOW * /*pad*/, size_t /*from*/)
{
}
#else	/* TRIM_NO_INDENT */
void	HyperDocument::ProcessLI(char *buffer, size_t length, WINDOW *pad, size_t from)
{
	if (numIndent > 0 && numIndent <= NUM_INDENT) {
		if (!pad) {
			if (GetNestedAttr() != normalA) {
				ThrowErrorTagInside("<LI>", _("<A ...>, <B> or <I>"),
						    buffer, from-1, length);
			}
		}
		else {
			attr_t	oldAttr;
			short oldPair;
			my_wattr_get(pad, &oldAttr, &oldPair);
			wattrset(pad, normalA);
			wmove(pad, curRow, curColumn);
			for (int i = 0; i < indentPos[numIndent-1]; i++) {
				waddch(pad, ' ');
			}
					// Unordered list
			switch (indentType[numIndent-1]) {
				case IND_UL:
				case IND_DL:
					curColumn = indentPos[numIndent-1]-2;
					wmove(pad, curRow, curColumn);
					draw_entity(pad, EID_BULLET);
					break;
				case IND_OL:
					{		// Ordered list
						int	val = indentVal[numIndent-1];
						curColumn = indentPos[numIndent-1]-4;
						wmove(pad, curRow, curColumn);
						if (val > 999) {
								// Beyond program limit
								// Must have width 3
							my_waddstr(pad, _("***"));
						}
						else {
							int	printed = 0;
							if (val/100) {
								waddch(pad, val/100+'0');
								printed = 1;
							}
							else
								waddch(pad, ' ');
							val = val%100;
							if (val/10 || printed) {
								waddch(pad, val/10+'0');
								printed = 1;
							}
							else
								waddch(pad, ' ');
							val = val%10;
							if (val || printed) {
								waddch(pad, val+'0');
								printed = 1;
							}
							else
								waddch(pad, ' ');
						}
						indentVal[numIndent-1]++;
					}
					break;
				case IND_BLOCK:
					ThrowErrorTagInside("<LI>", "<BLOCKQUOTE>",
							    buffer, from-1, length);
			}
			my_wattr_set(pad, oldAttr, oldPair);
		}
		curColumn = indentPos[numIndent-1];
		if (pad)
			wmove(pad, curRow, curColumn);
	}
}
#endif	/* TRIM_NO_INDENT */

/* pad == NULL indicates that it is called from FindLink(...),
   curRow & curColumn are ignored in this case */

void	HyperDocument::ProcessA(char *buffer, size_t length, WINDOW *pad, size_t from,
		int curRow, int curColumn)
{
	size_t	i = SeekNonSpace(buffer, from, length);
	size_t	j = SeekTokenEnd(buffer, i, length);

	if (isInA == 1 && procA) {	// Must have </A> for <A HREF=...>
					// before another <A ...> tag
		ThrowErrorNestedTag(_("<A ...>"),
				    buffer, from-2, length);
	}
	else if (i == j)
		return;

	int	type = TYPE_NULL;
	string	name;
	if (CompareStringCase(buffer+i, "name", j-i) == 0) {
		type = TYPE_SECTION;
		name = _("<A NAME=\"...\">");
	}
	else if (CompareStringCase(buffer+i, "href", j-i) == 0) {
		type = TYPE_GOTO_OR_EXEC;
		name = _("<A HREF=...>");
	}
	else if (CompareStringCase(buffer+i, "call", j-i) == 0) {
		type = TYPE_CALL;
		name = _("<A CALL=\"...\">");
	}
	else {					// Unknown <A ????>
		isInA = 1;			// Wait for </A>
		return;
	}


	if (buffer[j] == '>') {
		ThrowErrorSymbolNeeded(_("`=\'"), name,
				       buffer, from-2, length);
	}

	j = SeekNonSpace(buffer, j, length);	// Skip spaces
	if (j == length) {
		ThrowErrorEndOfFile(name,
				    buffer, from-2, length);
	}
	if (buffer[j] != '=') {			// Check "="
		ThrowErrorSymbolNeeded(_("`=\'"), name,
				       buffer, from-2, length);
	}
	j++;					// Skip "="

	j = SeekNonSpace(buffer, j, length);	// Skip spaces
	if (j == length) {
		ThrowErrorEndOfFile(name,
				    buffer, from-2, length);
	}

	if (type == TYPE_GOTO_OR_EXEC) {
		if (buffer[j] != '\"' && buffer[j] != '`') {	/* Check " */
			ThrowErrorSymbolNeeded(_("`\"\' or ``\'"), name,
					       buffer, from-2, length);
		}
		type = (buffer[j] == '\"' ? TYPE_GOTO : TYPE_EXEC);
	}
	else {
		if (buffer[j] != '\"') {		// Check "
			ThrowErrorSymbolNeeded(_("`\"\'"), name,
					       buffer, from-2, length);
		}
	}

	isInA = 1;

	i = j+1;

	try {
		if (type == TYPE_EXEC)
					// Search closing `
					// Shell command, can be quoted
			j = SeekChar('`', buffer, i, length, 2);	// nest support
		else
			j = SeekChar('\"', buffer, i, length, 2);	// Search closing "
								// Quote support for kcd
								// since dir name may 
								// contain special chars
	}
	catch (...) {
		ThrowErrorEndOfFile(name,
				    buffer, from-2, length);
	}

	string	quoted(buffer+i, j-i);		
	procA = new Anchor(UnquoteURLChars(quoted));

	procA->type = type;
	procA->rowFrom = curRow;
	procA->colFrom = curColumn;

	if (type == TYPE_SECTION) {
		procA->rowTo = curRow;		// Don't care where </A> is
		procA->colTo = curColumn;

		sectionList.push_back(sptr<Anchor>(procA));

		procA = NULL;
	}
	else {
		procA->rowTo = -1;		// Still invalid
		procA->colTo = -1;

		anchorList.push_back(sptr<Anchor>(procA));
		numNestA++;
		if (pad)
			wattrset(pad, GetNestedAttr());
	}
}

void	HyperDocument::NextLine(WINDOW *pad)
{
	if (pad == NULL) {			// Update document width
		if (curColumn > numColumn)
			numColumn = curColumn;
	}
	else					// Record column width
		numChar[curRow] = curColumn;

	curRow++;				// Go to new line
	curColumn = 0;
	IndentLine(pad);			// Indent if necessary
}

void	HyperDocument::ProcessHypertext(char *buffer, size_t length, WINDOW *pad)
{
	init_entity_table();

	size_t	i = 0, j = 0;
	size_t	k = 0;
	int	lastCharColumn = 0, lastCharRow = 0;	/* Last char pos */
	int	lastPreColumn = 0, lastPreRow = 0;	/* Last preformatted pos */
	int	lastChar = 0;		/* Value ignored */

	numIndent = 0;			// Clear indent if this function is
					// run the second time
	numNestA = 0;
	numNestB = 0;
	numNestI = 0;

	numNestBTag = 0;
	numNestITag = 0;
	numNestStrongTag = 0;
	numNestEmTag = 0;
	numNestCiteTag = 0;

	if (pad) {
		numChar = new short[numRow];
	
		wattrset(pad, normalA);		/* Normal attribute */
	}

//#define DEBUG_FORMAT_TEXT
#ifdef DEBUG_FORMAT_TEXT
	FILE *f;
	if (pad) {
		f = k_fopen("debug.pad", "wb");
		if (f == NULL)
			cerr << "ERROR1";
	}
	else {
		f = k_fopen("debug.nopad", "wb");
		if (f == NULL)
			cerr << "ERROR2";
	}
#endif

	while (i < length) {

#ifdef DEBUG_FORMAT_TEXT
		fprintf(f, "[%d] %c (%d,%d)\n", i, buffer[i], curRow, curColumn);
#endif
#ifdef DEBUG_WRAP
		if (pad) {
			pnoutrefresh(pad, 0, 0, 0, 0, scrRow-1, scrColumn-1);
			wrefresh(stdscr);
			getch();
		}
#endif
	
		if (buffer[i] == '<') {		/* New tag found */
			i++;		/* Advance pointer past '<' */
			j = SeekTokenEnd(buffer, i, length);

				/* Look for tags */
			if (j-i == 2 && CompareStringCase(buffer+i, "BR", j-i) == 0) {
				NextLine(pad);
				lastPreColumn = curColumn;
				lastPreRow = curRow;
			}
			
#ifndef TRIM_NO_HEADER
			else if ((j-i == 2 && CompareStringCase(buffer+i, "H1", j-i) == 0) ||
			    (j-i == 2 && CompareStringCase(buffer+i, "H2", j-i) == 0) ||
			    (j-i == 2 && CompareStringCase(buffer+i, "H3", j-i) == 0) ||
			    (j-i == 2 && CompareStringCase(buffer+i, "H4", j-i) == 0) ||
			    (j-i == 2 && CompareStringCase(buffer+i, "H5", j-i) == 0) ||
			    (j-i == 2 && CompareStringCase(buffer+i, "H6", j-i) == 0)) {

				if (curColumn != IndentColumn()) {
					if (pad == NULL) {
						if (curColumn > numColumn)
							numColumn = curColumn;
					}
					else
						numChar[curRow] = curColumn;
					curRow++;
					curColumn = 0;
				}

				NextLine(pad);
				lastPreColumn = curColumn;
				lastPreRow = curRow;
				if (pad) {
					wattrset(pad, headerA);
				}
			}
			else if ((j-i == 3 && CompareStringCase(buffer+i, "/H1", j-i) == 0) ||
			    (j-i == 3 && CompareStringCase(buffer+i, "/H2", j-i) == 0) ||
			    (j-i == 3 && CompareStringCase(buffer+i, "/H3", j-i) == 0) ||
			    (j-i == 3 && CompareStringCase(buffer+i, "/H4", j-i) == 0) ||
			    (j-i == 3 && CompareStringCase(buffer+i, "/H5", j-i) == 0) ||
			    (j-i == 3 && CompareStringCase(buffer+i, "/H6", j-i) == 0) ||
			    (j-i == 2 && CompareStringCase(buffer+i, "DD", j-i) == 0) ||
			    (j-i == 1 && CompareStringCase(buffer+i, "P", j-i) == 0)) {

				if (curColumn != IndentColumn()) {
					if (pad == NULL) {
						if (curColumn > numColumn)
							numColumn = curColumn;
					}
					else
						numChar[curRow] = curColumn;
					curRow++;
					curColumn = 0;
				}

				NextLine(pad);
				lastPreColumn = curColumn;
				lastPreRow = curRow;
				if (pad) {
					wattrset(pad, normalA);
				}
			}
#endif	/* TRIM_NO_HEADER */

			else if (j-i == 1 && CompareStringCase(buffer+i, "A", j-i) == 0 && pad) {
				i++;	/* Skip this "A" */
				if (SeekNonSpace(buffer, i, length) > i) {
					/* More after "<A" */
					/* Process "HREF", "NAME" */
					ProcessA(buffer, length, pad, i,
						 curRow, curColumn);
				}
				/* else just empty <A> tag, ignore it ? */
			}
			else if (j-i == 2 && CompareStringCase(buffer+i, "/A", j-i) == 0 && pad) {
				if (isInA == 0) {
					ThrowErrorTagWithout("</A>", _("<A HREF=...>"),
							     buffer, i-1, length);
				}
				else if (procA) {	/* Inside <A HREF=...> */
					procA->rowTo = curRow;
					procA->colTo = curColumn-1;	/* Prev column */
					procA = NULL;

					numNestA--;
					if (pad)
						wattrset(pad, GetNestedAttr());
				}
				isInA = 0;
			}
			else if (j-i == 3 && CompareStringCase(buffer+i, "PRE", j-i) == 0) {
				if (isInPre) {
					ThrowErrorNestedTag("<PRE>",
							    buffer, i-1, length);
				}
				isInPre++;
			}
			else if (j-i == 4 && CompareStringCase(buffer+i, "/PRE", j-i) == 0) {
				if (!isInPre) {
					ThrowErrorTagWithout("</PRE>", "<PRE>",
							     buffer, i-1, length);
				}
				isInPre--;
			}

#ifndef TRIM_NO_STD_ATTRIB
			else if (j-i == 1 && CompareStringCase(buffer+i, "B", j-i) == 0) {
				numNestB++;
				numNestBTag++;
				if (pad)
					wattrset(pad, GetNestedAttr());
			}
			else if (j-i == 2 && CompareStringCase(buffer+i, "/B", j-i) == 0) {
				if (numNestBTag == 0) {
					ThrowErrorTagWithout("</B>", "<B>",
							     buffer, i-1, length);
				}
				numNestB--;
				numNestBTag--;
				if (pad)
					wattrset(pad, GetNestedAttr());
			}
			else if (j-i == 1 && CompareStringCase(buffer+i, "I", j-i) == 0) {
				numNestI++;
				numNestITag++;
				if (pad)
					wattrset(pad, GetNestedAttr());
			}
			else if (j-i == 2 && CompareStringCase(buffer+i, "/I", j-i) == 0) {
				if (numNestITag == 0) {
					ThrowErrorTagWithout("</I>", "<I>",
							     buffer, i-1, length);
				}
				numNestI--;
				numNestITag--;
				if (pad)
					wattrset(pad, GetNestedAttr());
			}
#endif	/* TRIM_NO_STD_ATTRIB */

#ifndef TRIM_NO_EXTRA_ATTRIB
			else if (j-i == 6 && CompareStringCase(buffer+i, "STRONG", j-i) == 0) {
				numNestB++;
				numNestStrongTag++;
				if (pad)
					wattrset(pad, GetNestedAttr());
			}
			else if (j-i == 7 && CompareStringCase(buffer+i, "/STRONG", j-i) == 0) {
				if (numNestStrongTag == 0) {
					ThrowErrorTagWithout("</STRONG>", "<STRONG>",
							     buffer, i-1, length);
				}
				numNestB--;
				numNestStrongTag--;
				if (pad)
					wattrset(pad, GetNestedAttr());
			}
			else if (j-i == 2 && CompareStringCase(buffer+i, "EM", j-i) == 0) {
				numNestI++;
				numNestEmTag++;
				if (pad)
					wattrset(pad, GetNestedAttr());
			}
			else if (j-i == 3 && CompareStringCase(buffer+i, "/EM", j-i) == 0) {
				if (numNestEmTag == 0) {
					ThrowErrorTagWithout("</EM>", "<EM>",
							     buffer, i-1, length);
				}
				numNestI--;
				numNestEmTag--;
				if (pad)
					wattrset(pad, GetNestedAttr());
			}
			else if (j-i == 4 && CompareStringCase(buffer+i, "CITE", j-i) == 0) {
				numNestI++;
				numNestCiteTag++;
				if (pad)
					wattrset(pad, GetNestedAttr());
			}
			else if (j-i == 5 && CompareStringCase(buffer+i, "/CITE", j-i) == 0) {
				if (numNestCiteTag == 0) {
					ThrowErrorTagWithout("</CITE>", "<CITE>",
							     buffer, i-1, length);
				}
				numNestI--;
				numNestCiteTag--;
				if (pad)
					wattrset(pad, GetNestedAttr());
			}
#endif	/* TRIM_EXTRA_ATTRIB */

#ifndef TRIM_NO_INDENT
			else if (j-i == 2 && CompareStringCase(buffer+i, "UL", j-i) == 0) {
				if (numIndent < NUM_INDENT) {
					indentType[numIndent] = IND_UL;
					if (numIndent)
						indentPos[numIndent] = 
							indentPos[numIndent-1]+4;
					else
						indentPos[numIndent] = 4;
				}
				numIndent++;
			}
			else if (j-i == 2 && CompareStringCase(buffer+i, "OL", j-i) == 0) {
				if (numIndent < NUM_INDENT) {
					indentType[numIndent] = IND_OL;
					if (numIndent)
						indentPos[numIndent] = 
							indentPos[numIndent-1]+4;
					else
						indentPos[numIndent] = 4;
					indentVal[numIndent] = 1;
				}
				numIndent++;
			}
			else if (j-i == 2 && CompareStringCase(buffer+i, "DL", j-i) == 0) {
				if (numIndent < NUM_INDENT) {
					indentType[numIndent] = IND_DL;
					if (numIndent)
						indentPos[numIndent] = 
							indentPos[numIndent-1]+4;
					else
						indentPos[numIndent] = 4;
				}
				numIndent++;
			}
			else if (j-i == 10 && CompareStringCase(buffer+i, "BLOCKQUOTE", j-i) == 0) {
					/* New line */				
				if (curColumn != IndentColumn())
					NextLine(pad);

				if (numIndent < NUM_INDENT) {
					indentType[numIndent] = IND_BLOCK;
					if (numIndent)
						indentPos[numIndent] = 
							indentPos[numIndent-1]+4;
					else
						indentPos[numIndent] = 4;
				}
				numIndent++;

				IndentLine(pad);
				lastPreColumn = curColumn;
				lastPreRow = curRow;
			}
			else if (j-i == 3 && CompareStringCase(buffer+i, "/UL", j-i) == 0) {
				if (curColumn != IndentColumn()) {
					NextLine();
				}

				if (numIndent > 0)
					numIndent--;

				NextLine(pad);
				lastPreColumn = curColumn;
				lastPreRow = curRow;
			}
			else if (j-i == 3 && CompareStringCase(buffer+i, "/OL", j-i) == 0) {
				if (curColumn != IndentColumn())
					NextLine(pad);

				if (numIndent > 0)
					numIndent--;

				NextLine(pad);
				lastPreColumn = curColumn;
				lastPreRow = curRow;
			}
			else if (j-i == 3 && CompareStringCase(buffer+i, "/DL", j-i) == 0) {
				if (curColumn != IndentColumn())
					NextLine(pad);

				if (numIndent > 0)
					numIndent--;

				NextLine(pad);
				lastPreColumn = curColumn;
				lastPreRow = curRow;
			}
			else if (j-i == 11 && CompareStringCase(buffer+i, "/BLOCKQUOTE", j-i) == 0) {
				if (curColumn != IndentColumn())
					NextLine(pad);

				if (numIndent > 0)
					numIndent--;

				NextLine(pad);
				lastPreColumn = curColumn;
				lastPreRow = curRow;
			}
			else if (j-i == 2 && CompareStringCase(buffer+i, "LI", j-i) == 0) {
				if (curColumn != IndentColumn())
					NextLine(pad);

				ProcessLI(buffer, length, pad, i);
				lastPreColumn = curColumn;
				lastPreRow = curRow;
			}
			else if (j-i == 2 && CompareStringCase(buffer+i, "DT", j-i) == 0) {
				if (curColumn != IndentColumn())
					NextLine(pad);

				NextLine(pad);

				ProcessLI(buffer, length, pad, i);
				lastPreColumn = curColumn;
				lastPreRow = curRow;
			}
#endif	/* TRIM_NO_INDENT */

#ifndef TRIM_NO_RULE
			else if (j-i == 2 && CompareStringCase(buffer+i, "HR", j-i) == 0) {
				if (curColumn != IndentColumn())
					NextLine(pad);

				NextLine(pad);
				
				if (pad) {
					for (int curColumn = numColumn/4; curColumn < numColumn*3/4;
					     curColumn++) {
						wmove(pad, curRow, curColumn);
						draw_entity(pad, EID_HLINE);
					}
				}
				NextLine(pad);
				NextLine(pad);
				lastPreColumn = curColumn;
				lastPreRow = curRow;
			}
#endif	/* TRIM_NO_RULE */

			else if (j-i == 5 && CompareStringCase(buffer+i, "TITLE", j-i) == 0) {
				try {
					j = SeekChar('>', buffer, i+5, length);
				}
				catch (...) {
					ThrowErrorCannotFind(_("`>\'"),
							     buffer, i-1, length);
				}
				j++;	/* Skip '>' */
				k = j;
				try {
					j = SeekCaseString("</TITLE>", buffer, j, length);
				}
				catch (...) {
						ThrowErrorCannotFind("</TITLE>",
								     buffer, i-1, length);
				}
				if (titleText.size()) {
					ThrowErrorMessage(_("cannot "
							    "specify title more "
							    "than once"),
							  buffer, i-1, length);
				}
						/* Empty <TITLE></TITLE> allowed */
				if (j-k)
					titleText = string(buffer+k, j-k);
			}
			else if (CompareStringCase(buffer+i, "!--", 3) == 0 && pad) {
						/* Seek for "-->" */
				try {
					j = SeekString("-->", buffer, i, length);
				}
				catch (...) {
					ThrowErrorCannotFind(_("`-->\'"),
							     buffer, i-1, length);
				}
			}
			/* else - ignore unregcognized tags */

					/* Seek for '>' */
					/* Can be quoted */
			try {
				i = SeekChar('>', buffer, j, length, 2);
							/* nest support */
			}
			catch (...) {
				ThrowErrorCannotFind(_("`>\'"),
						     buffer, i-1, length);
			}
			i++;
		}
		else {
			chtype	bufChar = '?';	// Enough to cover 0..255
#ifdef USE_UTF8_MODE
			bool	use_wide = false;
			wchar_t	bufWChar;
			int	charWidth = 1;
#endif

			switch(buffer[i]) {
				case '\t':
				case '\n':
				case '\r':
				case '\b':
				case ' ':
				case '\0':
					bufChar = ' ';
					break;
				case '&':
					i++;
						/* Seek for ';' */
					try {
						j = SeekChar(';', buffer, i, length);
					}
					catch (...) {
						ThrowErrorCannotFind(_("`;\'"),
								     buffer, i-1, length);
					}

					if (buffer[i] == '#') {
						unsigned bufChar_ = 0;
						try {
							bufChar_ = StringToUnsigned(buffer+i+1, ';');
						}
						catch (ErrorRange) {
							ThrowErrorMessage(_("error in escape"),
									  buffer, i-1, length);
						}
						if (bufChar_ > 255) {
							ThrowErrorMessage(_("character code too large"),
									  buffer, i-1, length);
						}
						bufChar = static_cast<chtype>(bufChar_);
					}
					else {
							// For unknown escape
						bufChar = '?';

						for (size_t k = 0; entity_table[k].name; ++k) {
							if (static_cast<size_t>(j-i) == entity_table[k].length
							    && strncmp(buffer+i, entity_table[k].name, j-i) == 0) {
								// FIXME: Handle wide/combining entity
#ifdef USE_UTF8_MODE
								if (IsUTF8Mode()) {
									use_wide = true;
									bufWChar = entity_table[k].wc;
									charWidth = wcwidth(bufWChar);
								}
								else
#endif
									bufChar = entity_table[k].code;
								break;
							}
						}

					}
					i = j;	/* i++ appears later... */
					break;
				default:	// ncurses only works with unsigned char
						// Assume no combining char
#ifdef USE_UTF8_MODE
					if (IsUTF8Mode()) {
						static mbstate_t state;
						memset(&state, 0, sizeof(mbstate_t));
						int ret = mbrtowc(&bufWChar, buffer+i, length-i, &state);
						if (ret <= 0)
							throw ErrorBadSequence();

						// FIXME: Check if printable logic correct?
						if (iswprint(bufWChar) || bufWChar == ' ') {
							charWidth = wcwidth(bufWChar);
							use_wide = true;
							i += ret-1;
						}
						else {
							bufChar = '?';
						}
					}
					else
#endif
					     if (isprint(buffer[i]) || buffer[i] == ' ') {
						bufChar = static_cast<unsigned char>(buffer[i]);
					}
					else {
						bufChar = static_cast<unsigned char>('?');
					}
			}

#ifdef USE_UTF8_MODE
			if (charWidth == 0) {
				if (pad)
					DrawWCharSub(pad, bufWChar);
				i++;
				continue;
			}
			else if (charWidth > 1) {
				if (curColumn > scrColumn-charWidth && isInPre == 0)
					NextLine(pad);

				if (pad)
					DrawWCharSub(pad, bufWChar);
				else		// Keep track of char for wrap
					for (int i = 0; i < charWidth; ++i)
						lineBuffer[curColumn+i] = '*';
				curColumn += charWidth;
				if (!isInPre) {	// this char pos
					lastCharRow = curRow;
					lastCharColumn = curColumn-1;
					lastChar = bufChar;
				}
				else {		// this preformatted char pos
					lastPreColumn = curColumn-1;
					lastPreRow = curRow;
				}
				i++;
				continue;
			}
#endif

				// Handle whitespaces
				// or line too long outside <PRE> tag

			if (bufChar == ' ' ||
			    (curColumn > scrColumn-1 && isInPre == 0)) {
				if (isInPre == 0) {

						// Skip space at the beginning of line
					if (curColumn == IndentColumn() && 
					    bufChar == ' ') {
						i++;	// Skip
					}

						// Ignore double spaces
					else if (curColumn > IndentColumn() && 
						 lastCharRow == curRow &&
						 lastCharColumn == curColumn-1 &&
						 lastChar == ' ' && bufChar == ' ') {
					    
						lastCharColumn = curColumn;
						i++;	// Skip
					}

						// Do wrap
						// FIXME: Update for Unicode
					else if (curColumn > scrColumn-1) {	
						chtype	ch;
						int	x = curColumn-1;
						if (pad)
							wmove(pad, curRow, x);
							
						// Search for the last space in the line
						while ((pad ? (ch = winch(pad) &
							(A_CHARTEXT|A_ALTCHARSET)):
							lineBuffer[x]) 
						       != ' ') {
							x--;
							if (x >= 0) {	// Move cursor back
								if (pad) 
									wmove(pad, curRow, x);
								if (curRow == lastPreRow &&
								    x == lastPreColumn)
								    	// Can no longer wrapped 
								    	// since it's preformatted
								    	break;
							}
							else		// x < 0, wrap fails
								break;
						}

							      	// Cannot break line
						if (x < 0 || (curRow == lastPreRow 
							      && x == lastPreColumn)) {	

								// We are printing a space
								// so break the line here
							if (bufChar == ' ') {
								NextLine(pad);
								lastPreColumn = curColumn;
								lastPreRow = curRow;
							}

								// Print character beyond screen width
							else {
								if (pad) {
									wmove(pad, curRow, curColumn);
									waddch(pad, bufChar);
								}
								else {			// This is OK
									lineBuffer[curColumn] = '*';
								}
								curColumn++;
							}
							i++;		// Skip current char
						}

							      	// Can break line
						else {
							x = curColumn;	/* Leave the space as-is 
									   as it may be inside
									   some link text */
							
							int	prevRow = curRow;
							int	prevCol = curColumn-1;
								/* Init it to avoid
								   compiler warning */
							attr_t	oldAttr = normalA;
							short oldPair = 0;
							
							NextLine(pad);
							lastPreColumn = curColumn;
							lastPreRow = curRow;
							
							if (pad) {
								/* Update links position */
								for (sptr_list<Anchor>::reverse_iterator iter = anchorList.rbegin();
								     iter != anchorList.rend()
								     && ((*iter)->rowFrom == prevRow || (*iter)->rowTo == prevRow); 
								     ++iter) {
									if ((*iter)->rowFrom == prevRow && (*iter)->colFrom >= x) {
										(*iter)->rowFrom = curRow;
										(*iter)->colFrom -= x;
									}
									if ((*iter)->rowTo == prevRow && (*iter)->colTo >= x) {
										(*iter)->rowTo = curRow;
										(*iter)->colTo -= x;
									}
								}

								/* Update section position */
								for (sptr_list<Anchor>::reverse_iterator iter = sectionList.rbegin();
								     iter != sectionList.rend()
								     && ((*iter)->rowFrom == prevRow || (*iter)->rowTo == prevRow); 
								     ++iter) {
									if ((*iter)->rowFrom == prevRow && (*iter)->colFrom >= x) {
										(*iter)->rowFrom = curRow;
										(*iter)->colFrom -= x;
									}
									if ((*iter)->rowTo == prevRow && (*iter)->colTo >= x) {
										(*iter)->rowTo = curRow;
										(*iter)->colTo -= x;
									}
								}
								my_wattr_get(pad, &oldAttr, &oldPair);
								wattrset(pad, A_NORMAL);
							}
							for (k = x; k <= static_cast<size_t>(prevCol); k++) {
								if (pad) {
									wmove(pad, prevRow, k);
									ch = winch(pad);
									waddch(pad, ' ');
									wmove(pad, curRow, curColumn);
									waddch(pad, ch);
#ifdef DEBUG_WRAP
	pnoutrefresh(pad, 0, 0, 0, 0, scrRow-1, scrColumn-1);
	wrefresh(stdscr);
	getch();
#endif
								}
								else {
									lineBuffer[curColumn] = lineBuffer[k];
								}
								curColumn++;
							}
							/* Display another space */
							if (pad) {
								my_wattr_set(pad, oldAttr, oldPair);
								wmove(pad, curRow, curColumn);
								waddch(pad, bufChar);
							}
							else
								lineBuffer[curColumn] = bufChar;
	
							/* Track & remove contiguous space */
							lastCharRow = curRow;
							lastCharColumn = curColumn;
							lastChar = bufChar;
							curColumn++;
							if (curColumn == NUM_COLUMN) {
								NextLine(pad);
								lastPreColumn = curColumn;
								lastPreRow = curRow;
							}
							i++;	
						}
					}

						// Not reach screen width yet
					else {
						// Display it
						if (pad) {
							// No need to deal with Unicode
							// for space char.
							waddch(pad, bufChar);
						}
						else
							lineBuffer[curColumn] = bufChar;
							
						// Track & remove contiguous space
						lastCharRow = curRow;
						lastCharColumn = curColumn;
						lastChar = bufChar;
						curColumn++;
						if (curColumn == NUM_COLUMN) {
							NextLine(pad);
							lastPreColumn = curColumn;
							lastPreRow = curRow;
						}
						i++;	
					}
				}
				else {			// For whitespaces 
							// inside <PRE> tag

					lastPreColumn = curColumn;
					lastPreRow = curRow;
					
					switch(buffer[i]) {
						case ' ':
						case '\0':
							if (pad)
								waddch(pad, ' ');
							else	// No wrap char
								lineBuffer[curColumn] = '*';
							curColumn++;
							break;
						case '\t':
							do {	// Add space to pad
								if (pad)
									waddch(pad, ' ');
								else	// No wrap char
									lineBuffer[curColumn] = '*';
								curColumn++;
							} while(curColumn % 8);
							break;
						case '\n':
							NextLine(pad);
							lastPreColumn = curColumn;
							lastPreRow = curRow;
							break;
					}
					if (curColumn == NUM_COLUMN) {
						NextLine(pad);
						lastPreColumn = curColumn;
						lastPreRow = curRow;
					}
					i++;
				}
			}

			else {			// Can be displayed directly
			
				if (pad) {	// Add char. to pad
#ifdef USE_UTF8_MODE
					if (use_wide)
						DrawWCharSub(pad, bufWChar);
					else
#endif
						waddch(pad, bufChar);
				}
				else	// Keep track of char for wrap
					lineBuffer[curColumn] = '*';
				if (!isInPre) {	// this char pos
					lastCharRow = curRow;
					lastCharColumn = curColumn;
					lastChar = bufChar;
				}
				else {		// this preformatted char pos
					lastPreColumn = curColumn;
					lastPreRow = curRow;
				}
				if (curColumn == NUM_COLUMN) {
					NextLine(pad);
					lastPreColumn = curColumn;
					lastPreRow = curRow;
				}
				curColumn++;
				i++;
			}
		}
	}
	if (isInA == 1) {
		throw ErrorGenericSyntax(_("unexpected end of file, missing </A>"));
	}

	if (isInPre == 1) {
		throw ErrorGenericSyntax(_("unexpected end of file, missing </PRE>"));
	}

#ifdef DEBUG_FORMAT_TEXT
	fclose(f);
#endif

	if (pad == NULL) {	// Update document size
		if (curColumn > numColumn)
			numColumn = curColumn;
		numRow = curRow+1;	// Off-by-1 different
	}
	else {				// *** Move check to loop above ??
		if (curRow != numRow-1 && curRow > scrRow) {
			throw ErrorGenericSyntax(_("hypertext size calculation not match actual hypertext parse\n"
						   "Number of row: %$ (calculatation) %$ (parse)"),
						 numRow, curRow+1);
		}

		for (i = curRow; i < static_cast<size_t>(numRow); i++) {
			numChar[i] = curColumn;
			curColumn = 0;
		}

// #define DEBUG_NUM_CHAR
#ifdef DEBUG_NUM_CHAR
		FILE *f2;
		f2 = k_fopen("debug.num", "wb");
		if (f2 == NULL)
			cerr << "ERROR1";
		else {
			for (i = 0; i < numRow; i++)
				fprintf(f2, "[%d] %d\n", i, static_cast<int>(numChar[i]));
			fclose(f2);
		}
#endif


	}
}

// pad == NULL when we want to find document dimension
// row, col = Screen size for hypertext display.  Minimum pad size used here
void	HyperDocument::FormatDocument(WINDOW *pad, int row, int col)
{
	ReInitFormat();	// Initialize document dependent variables
			// without freeing buffer
			
	scrRow = row;
	scrColumn = col;
	
	if (! drawObj) {
		if (pad) {	// We only need following when we output to pad
			;
		}
		else {		// We are going to find document dimension
			numRow = 1;	// Minimum limit - must contain an empty line
			numColumn = 0;
		}
		ProcessHypertext(buffer, bufferSize, pad);
	}
	else {
		drawObj->Draw(*this, pad);
	}
}

void	HyperDocument::SetDocumentSize(int docRow, int docCol, int row, int col)
{
	scrRow = row;		// Screen size for hypertext display
	scrColumn = col;
	
	numColumn = docCol;	// Document size
	numRow = docRow;
}

//------------------------------------------------------------------------
// Find link
//------------------------------------------------------------------------

int	HyperDocument::ReadATag()
{
	int	currentID = 0;	// ID used to replace rowFrom member of Anchor
				// It is used to sync between anchorList and
				// sectionList
	size_t	i = 0, j = 0;

	ReInitFormat();		// Initialize document dependent variables
				// without freeing buffer

	while (i < bufferSize) {
	
		if (buffer[i] == '<') {		/* New tag found */
			i++;		/* Advance pointer past '<' */
			j = SeekTokenEnd(buffer, i, bufferSize);

			if (j-i == 1 && CompareStringCase(buffer+i, "A", j-i) == 0) {
				i++;	/* Skip this "A" */
				if (SeekNonSpace(buffer, i, bufferSize) > i) {
					/* More after "<A" */
					/* Process "HREF", "NAME", "CALL" */
					ProcessA(buffer, bufferSize,
						 NULL, i,
						 currentID, -1);
					currentID++;
				}
				/* else just empty <A> tag, ignore it ? */
			}
			else if (j-i == 2 && CompareStringCase(buffer+i, "/A", j-i) == 0) {
				if (isInA == 0) {
					ThrowErrorTagWithout("</A>", _("<A HREF=...>"),
							     buffer, i-1, bufferSize);
				}
				if (procA) {
					procA = NULL;
					numNestA--;
				}
				isInA = 0;
			}

			/* else - ignore unregcognized tags */

					/* Seek for '>' */
					/* Can be quoted */
			try {
				i = SeekChar('>', buffer, j, bufferSize, 2);
						/* nest support */
			}
			catch (...) {
				ThrowErrorCannotFind(_("`>\'"),
						     buffer, i-1, bufferSize);
			}
			i++;
		}
		else
			i++;			// Check next char
	}
	return RET_OK;
}

int	CheckString(const string &s, const string &substr)
{
	string::size_type sub_idx = s.rfind(substr);
	if (sub_idx == string::npos)	// Cannot match, substring not found
					// in the anchor text
		return 0;

			// We must check if the matched string appears  
			// after the last `/' in anchor text

	string::size_type dir_idx = s.rfind('/');
	if (dir_idx == string::npos)	// Not found
		return 1;
	if (dir_idx < sub_idx)		// Match last portion of directory
		return 1;
	return 0;
}

Anchor	*HyperDocument::FindLink(const string &startSection, const string &findLink)
{
	if (ReadATag() != RET_OK)	// Read all <A HREF=...>, 
					// <A CALL="..."> and 
					// <A NAME="..."> tags from buffer
		return NULL;		// Error detected
	return FindNextLink(startSection, findLink);
}
		
Anchor	*HyperDocument::FindNextLink(const string &startSection, const string &findLink)
{
	if (anchorList.size() == 0) {		// No <A HREF=...> or <A CALL="...">
					// found
					
					// Report error
		throw ErrorGenericSyntax(_("no <A HREF=...> or <A CALL=\"...\"> tags present\n"
					   "Do not use the `-l\' option for this file"));
	}

	Anchor *curSection = 0;
	sptr_list<Anchor>::iterator curSectionIter;

	if (startSection.size()) {
					// Seek to the given startSection

		for (curSectionIter = sectionList.begin(); 
		     curSectionIter != sectionList.end(); ++curSectionIter) {
		
					// Found <A NAME="..."> tag that 
					// matches startSection
			if ((*curSectionIter)->name == startSection) {
				curSection = (*curSectionIter)();
				break;
			}
		}

					// Cannot find the desired tag
		if (!curSection) {
			throw ErrorGenericSyntax(_("cannot find <A NAME=\"%$\">"),
						 startSection);
		}
	}

					// Need to find the link after 
					// startSection if startSection 
					// given
	sptr_list<Anchor>::iterator curAnchorIter;

	if (curSection) {
				// Find anchor directly after curSection 

		for (curAnchorIter = anchorList.begin(); 
		     curAnchorIter != anchorList.end(); ++curAnchorIter) {
		
				// Skip a <A HREF=...> or <A CALL="...">
				// after curSection.
				// This is used to ensure that
				// kcd `dir' cycles through every matched 
				// `dir'

				// Found
			if ((*curAnchorIter)->rowFrom > (*curSectionIter)->rowFrom+1) {
				break;
			}
		}
			
				// Zero or one <A HREF=...> or <A CALL="...">
				// after curSection
		if (curAnchorIter == anchorList.end()) {
		
				// Let's use the first anchor of the document
				// instead
			curAnchorIter = anchorList.begin();
		}
	}

				// Find the first anchor after curAnchor 
				// that matches findLink
	Anchor *anchorSelected = 0;
	sptr_list<Anchor>::iterator anchorSelectedIter = curAnchorIter;

	do {
		if (CheckString((*anchorSelectedIter)->name, findLink)) {

							// Found !
			anchorSelected = (*anchorSelectedIter)();	// Set return value
			break;
		}
		next_loop(anchorList, anchorSelectedIter);
	} while (curAnchorIter != anchorSelectedIter);	// Make sure loop not repeated

	return anchorSelected;
}

//------------------------------------------------------------------------
// Draw functions
//------------------------------------------------------------------------

void	HyperDocument::DrawSetSize(int row, int col)
{
	numRow = row;
	numColumn = col;
}

void	HyperDocument::DrawBegin(WINDOW *pad)
{
	init_entity_table();

	curRow = 0;
	curColumn = 0;
	numChar = new short[numRow];
	for (size_t i = 0; i < static_cast<size_t>(numRow); ++i)
		numChar[i] = numColumn;

	numNestA = 0;
	numNestB = 0;
	numNestI = 0;
	numIndent = 0;

	wmove(pad, 0, 0);
}

void	HyperDocument::DrawSetTitle(const char *s)
{
	titleText = s;
}

void	HyperDocument::DrawSetTitle(const string &s)
{
	titleText = s;
}

void	HyperDocument::DrawSetItalic(WINDOW *pad)
{
	if (pad) {
		numNestI++;
		wattrset(pad, GetNestedAttr());
	}
}

void	HyperDocument::DrawClearItalic(WINDOW *pad)
{
	if (pad) {
		if (numNestI > 0)
			numNestI--;
		wattrset(pad, GetNestedAttr());
	}
}

void	HyperDocument::DrawSetBold(WINDOW *pad)
{
	if (pad) {
		numNestB++;
		wattrset(pad, GetNestedAttr());
	}
}

void	HyperDocument::DrawClearBold(WINDOW *pad)
{
	if (pad) {
		if (numNestB > 0)
			numNestB--;
		wattrset(pad, GetNestedAttr());
	}
}

void	HyperDocument::DrawCharSub(WINDOW *pad, chtype c)
{
	if (c == '\n') {
						// Record column width
		NextLine(pad);
	}
	else if (c == '\t' || c == '\r' || c == '\b' || c == '\0')
		;
	else {
		if (isprint(c))
			waddch(pad, static_cast<unsigned char>(c));
		else
			waddch(pad, '?');
		curColumn++;
	}
}

void	HyperDocument::DrawChar(WINDOW *pad, chtype c)
{
	if (pad) {
		if (procA)
			procA->linkText += static_cast<char>(c & 0xFF);

		DrawCharSub(pad, c);
	}
}

#ifdef USE_UTF8_MODE
void	HyperDocument::DrawWCharSub(WINDOW *pad, wchar_t wc)
{
	if (wc == '\n') {
						// Record column width
		NextLine(pad);
	}
	else if (wc == '\t' || wc == '\r' || wc == '\b' || wc == '\0')
		;
	else {
		if (iswprint(wc)) {
			int w = wcwidth(wc);
			if (w == 0 && curColumn) {
				cchar_t	cc;
				wmove(pad, curRow, curColumn-1);
				win_wch(pad, &cc);

				bool done = false;
				for (size_t i = 0; i < CCHARW_MAX; ++i) {
					if (!cc.chars[i]) {
						cc.chars[i] = wc;
						done = true;
						break;
					}
				}

				if (done) {
					wmove(pad, curRow, curColumn-1);
					wadd_wch(pad, &cc);
				}
				else {
					wmove(pad, curRow, curColumn);
					waddnwstr(pad, &wc, 1);
				}
			}
			else {
				waddnwstr(pad, &wc, 1);
			}
			curColumn += wcwidth(wc);
		}
		else {
			waddch(pad, '?');
			curColumn++;
		}
	}
}
#endif /* USE_UTF8_MODE */

void	HyperDocument::DrawEntityChar(WINDOW *pad, entity_id id)
{
	if (pad) {
		curColumn++;
		draw_entity(pad, id);
	}
}

void	HyperDocument::DrawString(WINDOW *pad, const char *s)
{
	if (pad) {
		if (procA)
			procA->linkText += s;

//ofstream ofs("xx", ios::app);
//ofs << curRow << ' ' << curColumn << ' ' << s << '\n';
#ifdef USE_UTF8_MODE
// FIXME: Handle wide/combining chars, UTF-8 encoding
//		if (0) {
		if (IsUTF8Mode()) {
			size_t length = strlen(s);

			for (size_t i = 0; s[i] != '\0'; ) {
				wchar_t wc;
				static mbstate_t state;
				memset(&state, 0, sizeof(mbstate_t));
				size_t ret = mbrtowc(&wc, s+i, length-i, &state);
				if (ret == 0 || ret == static_cast<size_t>(-1)
				    || ret == static_cast<size_t>(-2))
					throw ErrorBadSequence();

						// FIXME: Check if printable logic correct?
				if (iswprint(wc) || wc == ' ') {
					DrawWCharSub(pad, wc);
				}
				else {
					DrawCharSub(pad, '?');
				}
				i += ret;
			}
		}
		else
#endif
			for (size_t i = 0; s[i] != '\0'; ++i)
				DrawCharSub(pad, s[i]);
	}
}

void	HyperDocument::DrawString(WINDOW *pad, const string &s)
{
	DrawString(pad, s.c_str());
}

void	HyperDocument::DrawAName(WINDOW *pad, const char *s)
{
	if (pad) {
		Anchor *proc = new Anchor(s);

		proc->type = TYPE_SECTION;
		proc->rowFrom = curRow;
		proc->colFrom = curColumn;

		proc->rowTo = curRow;		// Don't care where </A> is
		proc->colTo = curColumn;

		sectionList.push_back(sptr<Anchor>(proc));
	}
}

void	HyperDocument::DrawAName(WINDOW *pad, const string &s)
{
	if (pad) {
		Anchor *proc = new Anchor(s);

		proc->type = TYPE_SECTION;
		proc->rowFrom = curRow;
		proc->colFrom = curColumn;

		proc->rowTo = curRow;		// Don't care where </A> is
		proc->colTo = curColumn;

		sectionList.push_back(sptr<Anchor>(proc));
	}
}

void	HyperDocument::DrawACallBegin(WINDOW *pad, const char *s)
{
	if (pad) {
		numNestA++;
		wattrset(pad, GetNestedAttr());

		procA = new Anchor(s);

		procA->type = TYPE_CALL;
		procA->rowFrom = curRow;
		procA->colFrom = curColumn;

		procA->rowTo = -1;
		procA->colTo = -1;

		anchorList.push_back(sptr<Anchor>(procA));
	}
}

void	HyperDocument::DrawACallBegin(WINDOW *pad, const string &s)
{
	if (pad) {
		numNestA++;
		wattrset(pad, GetNestedAttr());

		procA = new Anchor(s);

		procA->type = TYPE_CALL;
		procA->rowFrom = curRow;
		procA->colFrom = curColumn;

		procA->rowTo = -1;
		procA->colTo = -1;

		anchorList.push_back(sptr<Anchor>(procA));
	}
}

void	HyperDocument::DrawAHrefBegin(WINDOW *pad, const char *s)
{
	if (pad) {
		numNestA++;
		wattrset(pad, GetNestedAttr());

		procA = new Anchor(s);

		procA->type = TYPE_GOTO;
		procA->rowFrom = curRow;
		procA->colFrom = curColumn;

		procA->rowTo = -1;
		procA->colTo = -1;

		anchorList.push_back(sptr<Anchor>(procA));
	}
}

void	HyperDocument::DrawAHrefBegin(WINDOW *pad, const string &s)
{
	if (pad) {
		numNestA++;
		wattrset(pad, GetNestedAttr());

		procA = new Anchor(s);

		procA->type = TYPE_GOTO;
		procA->rowFrom = curRow;
		procA->colFrom = curColumn;

		procA->rowTo = -1;
		procA->colTo = -1;

		anchorList.push_back(sptr<Anchor>(procA));
	}
}

void	HyperDocument::DrawAEnd(WINDOW *pad)
{
	if (pad) {
		if (numNestA > 0) {
			procA->rowTo = curRow;
			procA->colTo = curColumn-1;	/* Prev column */

			numNestA--;
			wattrset(pad, GetNestedAttr());

			procA = 0;
		}
	}
}
