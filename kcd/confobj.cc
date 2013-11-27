/*
	confobj.cc	Configuration Class
	Copyright (c) 2000, 2002, 2003, 2004, 2005 Kriang Lerdsuwanakij
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

#include "confobj.h"
#include "cstrlib.h"

// fnmatch.h that comes with Apache has different function name
#define ap_fnmatch fnmatch
#include <fnmatch.h>


#ifdef	CLIB_HAVE_REGEX
int k_regcomp(regex_t *r, const string &s1, int flags)
{
	string s2;
	if (s1.size() && s1[0] == '^')
		s2 = s1;
	else {
		s2 = '^';
		s2 += s1;
	}
	if (s1[s1.size()-1] != '$')
		s2 += '$';
	return regcomp(r, s2.c_str(), flags);
}
#endif


KcdConfig::KcdConfig() {
	cfgQuietFull = false;		// Equals true for quiet mode (full scan)
	cfgQuietSmart = false;		// Equals true for quiet mode (smart scan)
	cfgQuietPartial = false;	// Equals true for quiet mode (partial scan)
	cfgAutoScan = false;		// Equals true to rescan automatically
	cfgSpaceSelect = true;		// Default - space in find mode
					// select the highlighted dir
	cfgSortTree = true;		// Equals true for sorted dir tree
	cfgCaseSensitiveSort = false;	// Equals true for case-sensitive sort
	cfgShowListThreshold = 4;	// When match more than the specified
					// number, show matched dir as a list
	cfgFuzzySize = 10;		// Number of directory kept for
					// fuzzy searching
	cfgMouseScrollRate = 2;		// Rate of mouse scrolling
	cfgGraphicChar = true;		// Enable line-drawing chars, etc.

#ifdef KCD_NC_GOOD_SCROLL
	cfgScrollBar = true;		// Enable scroll bar
#else
	cfgScrollBar = false;		// Disable scroll bar
#endif

	cfgGlobDot = false;
	cfgGlobPath = false;

	cfgKey = keyBindingDefault;

	cfgShowNewDir = showNewDirMulti;

	SetAttr(cfgAttrNormal, COLOR_WHITE, DEFAULT_COLOR, A_NORMAL, A_NORMAL);
	SetAttr(cfgAttrLink, COLOR_GREEN, DEFAULT_COLOR, A_NORMAL, A_UNDERLINE);
	SetAttr(cfgAttrHighlight, COLOR_CYAN, COLOR_BLUE, A_NORMAL, A_REVERSE);
	SetAttr(cfgAttrURL, COLOR_BLACK, COLOR_WHITE, A_NORMAL, A_REVERSE);
	SetAttr(cfgAttrHeader, COLOR_CYAN, DEFAULT_COLOR, A_NORMAL, A_BOLD);
	SetAttr(cfgAttrBold, COLOR_WHITE, DEFAULT_COLOR, A_BOLD, A_BOLD);
	SetAttr(cfgAttrItalic, COLOR_YELLOW, DEFAULT_COLOR, A_BOLD, A_BOLD);

	SetAttr(cfgAttrScrollArrow, COLOR_YELLOW, COLOR_BLUE, A_BOLD, A_NORMAL);
	SetAttr(cfgAttrScrollBlock, COLOR_CYAN, COLOR_BLUE, A_NORMAL, A_REVERSE);
	SetAttr(cfgAttrScrollBar, COLOR_BLUE, COLOR_CYAN, A_NORMAL, A_NORMAL);
	SetAttr(cfgAttrTitle, COLOR_BLACK, COLOR_CYAN, A_NORMAL, A_REVERSE);
	SetAttr(cfgAttrMore, COLOR_RED, DEFAULT_COLOR, A_NORMAL, A_BOLD);

	SetAttr(cfgAttrLinkBold, COLOR_WHITE, DEFAULT_COLOR, A_BOLD, A_BOLD | A_UNDERLINE);
	SetAttr(cfgAttrLinkItalic, COLOR_YELLOW, DEFAULT_COLOR, A_BOLD, A_BOLD | A_UNDERLINE);
	SetAttr(cfgAttrHighlightBold, COLOR_WHITE, COLOR_BLUE, A_BOLD, A_REVERSE);
	SetAttr(cfgAttrHighlightItalic, COLOR_YELLOW, COLOR_BLUE, A_BOLD, A_REVERSE);

	cfgDefaultBackground = COLOR_BLACK;

				// Can throw exception here!
	cfgSkipDir = new DirList;
	cfgStartDir = new DirList;

	cfgDefaultTree = new DirList;

	cfgMountDir = new MountDirList;
}

KcdConfig::KcdConfig(const KcdConfig &c)
{
	cfgSkipDir = 0;
	cfgStartDir = 0;
	cfgDefaultTree = 0;
	cfgMountDir = 0;

	*this = c;
}

KcdConfig &KcdConfig::operator=(const KcdConfig &c)
{
	cfgQuietFull = c.cfgQuietFull;
	cfgQuietSmart = c.cfgQuietSmart;
	cfgQuietPartial = c.cfgQuietPartial;
	cfgAutoScan = c.cfgAutoScan;
	cfgSpaceSelect = c.cfgSpaceSelect;
	cfgSortTree = c.cfgSortTree;
	cfgCaseSensitiveSort = c.cfgCaseSensitiveSort;
	cfgShowListThreshold = c.cfgShowListThreshold;
	cfgFuzzySize = c.cfgFuzzySize;
	cfgMouseScrollRate = c.cfgMouseScrollRate;
	cfgGraphicChar = c.cfgGraphicChar;
	cfgScrollBar = c.cfgScrollBar;

	cfgGlobDot = c.cfgGlobDot;
	cfgGlobPath = c.cfgGlobPath;

	cfgKey = c.cfgKey;

	cfgShowNewDir = c.cfgShowNewDir;

	cfgAttrNormal = c.cfgAttrNormal;
	cfgAttrLink = c.cfgAttrLink;
	cfgAttrHighlight = c.cfgAttrHighlight;
	cfgAttrURL = c.cfgAttrURL;
	cfgAttrHeader = c.cfgAttrHeader;
	cfgAttrBold = c.cfgAttrBold;
	cfgAttrItalic = c.cfgAttrItalic;

	cfgAttrScrollArrow = c.cfgAttrScrollArrow;
	cfgAttrScrollBlock = c.cfgAttrScrollBlock;
	cfgAttrScrollBar = c.cfgAttrScrollBar;
	cfgAttrTitle = c.cfgAttrTitle;
	cfgAttrMore = c.cfgAttrMore;

	cfgAttrLinkBold = c.cfgAttrLinkBold;
	cfgAttrLinkItalic = c.cfgAttrLinkItalic;
	cfgAttrHighlightBold = c.cfgAttrHighlightBold;
	cfgAttrHighlightItalic = c.cfgAttrHighlightItalic;

	cfgDefaultBackground = c.cfgDefaultBackground;

	delete cfgSkipDir;
	delete cfgStartDir;
	delete cfgDefaultTree;
	delete cfgMountDir;

				// Can throw exception here!
	cfgSkipDir = new DirList;
	*cfgSkipDir = *c.cfgSkipDir;
	cfgStartDir = new DirList;
	*cfgStartDir = *c.cfgStartDir;

	cfgDefaultTree = new DirList;
	*cfgDefaultTree = *c.cfgDefaultTree;

	cfgMountDir = new MountDirList;
	*cfgMountDir = *c.cfgMountDir;

	return *this;
}

KcdConfig::~KcdConfig()
{
	delete cfgSkipDir;
	cfgSkipDir = 0;
	delete cfgStartDir;
	cfgStartDir = 0;

	delete cfgDefaultTree;
	cfgDefaultTree = 0;

	delete cfgMountDir;
	cfgMountDir = 0;
}

void	KcdConfig::ForceQuiet(int forceQuiet)
{
	kcdConfig.cfgQuietFull = forceQuiet;
	kcdConfig.cfgQuietSmart = forceQuiet;
	kcdConfig.cfgQuietPartial = forceQuiet;
}

KcdConfig kcdConfig;
KcdConfig defConfig;
configMap allConfig;

/*************************************************************************
	Find string from linked-list helper
*************************************************************************/

template <bool regex, class T> bool TemplateFindDirList(const string &str,
							T *list)
{
	for (typename T::iterator iter = list->begin();
	     iter != list->end(); ++iter) {

		if (regex) {
#ifdef	CLIB_HAVE_REGEX
			if ((*iter)->regex) {
				regmatch_t m;
					// regexec returns 0 if strings match
				if (! regexec(&((*iter)->regex_buffer), str.c_str(), 1, &m, 0))
					return true;
			}
			else
#endif
			     {
				int flags = 0;
				if (! (*iter)->globDot)
					flags |= FNM_PERIOD;
				if (! (*iter)->globPath)
					flags |= FNM_PATHNAME;

				if (! k_fnmatch((*iter)->dir, str, flags))
					return true;
			}
		}
		else {
			if ((*iter)->dir == str)
				return true;
		}
	}
	return false;
}

/*************************************************************************
	Find string from linked-list
*************************************************************************/

bool	FindSkipDir(const string &str)
{
	if (str == "/dev" || str == "/proc")
		return true;

	return TemplateFindDirList<true>(str, kcdConfig.cfgSkipDir);
}

bool	FindStartDir(const string &str)
{
	return TemplateFindDirList<false>(str, kcdConfig.cfgStartDir);
}

bool	FindMountDir(const string &str)
{
	return TemplateFindDirList<true>(str, kcdConfig.cfgMountDir);
}

int	k_fnmatch(const string &pattern, const string &str)
{
	int flags = 0;
	if (! kcdConfig.cfgGlobDot)
		flags |= FNM_PERIOD;
	if (! kcdConfig.cfgGlobPath)
		flags |= FNM_PATHNAME;

	return k_fnmatch(pattern, str, flags);
}
