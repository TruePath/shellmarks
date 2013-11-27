/*
	confobj.h	Configuration Class Header
	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2009 Kriang Lerdsuwanakij
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

#ifndef __K_CONFOBJ_H
#define __K_CONFOBJ_H

#include "config.h"
#include "setupcurses.h"
#include "list.h"
#include "gzfileio.h"

#ifdef	CLIB_HAVE_REGEX
# include <sys/types.h>
# include <regex.h>
int k_regcomp(regex_t *r, const string &s1, int flags);
#endif

#include CXX__HEADER_climits
#include CXX__HEADER_map
#include CXX__HEADER_vector

USING_NAMESPACE_STD;

enum MountActionType {
	mountActionSkip = 0,
	mountActionAll,
	mountActionTree,
	mountActionFileList
};

typedef vector<MountActionType> MountActionOrderType;

enum ShowNewDirType {
	showNewDirNo = 0,
	showNewDirYes,
	showNewDirMulti
};

struct DirConfig {
	string	dir;
	bool	regex;

	bool	globDot;
	bool	globPath;

#ifdef	CLIB_HAVE_REGEX
	regex_t	regex_buffer;
#endif

	DirConfig(string dir_, bool regex_, bool globDot_, bool globPath_) : 
		dir(dir_), regex(regex_), globDot(globDot_), globPath(globPath_) {

#ifdef	CLIB_HAVE_REGEX
		k_regcomp(&regex_buffer, dir, REG_EXTENDED);
#endif
	}

	virtual bool	operator ==(const DirConfig& d) const {
		return dir == d.dir && regex == d.regex
				    && globDot == d.globDot
				    && globPath == d.globPath;
	}
	virtual bool	operator !=(const DirConfig& d) const {
		return !(*this == d);
	}
	virtual ~DirConfig() {}
};

struct MountDirConfig : public DirConfig {
	MountActionOrderType mountAction;

	MountDirConfig(string dir_, bool regex_, bool globDot_,
		       bool globPath_, const MountActionOrderType &mountAction_) :
		DirConfig(dir_, regex_, globDot_, globPath_), mountAction(mountAction_) {}
	virtual bool	operator ==(const MountDirConfig& d) const {
		return this->DirConfig::operator ==(d)
			&& mountAction == d.mountAction;
	}
};

typedef sptr_list<DirConfig>	DirList;
typedef sptr<DirConfig>		DirItem;
typedef sptr_list<MountDirConfig>	MountDirList;
typedef sptr<MountDirConfig>		MountDirItem;

#define SCAN_MODE_FULL		0
#define SCAN_MODE_SMART		1
#define SCAN_MODE_PARTIAL	2

enum KeyBindingType {
	keyBindingDefault = 0,
	keyBindingVi,
	keyBindingEmacs
};

extern string	confLocalFile;

#define DEFAULT_COLOR		INT_MAX		/* Must be the same type as
						   in AttrConfig */
struct AttrConfig {
	COLOR_TYPE	colorForeground;
	COLOR_TYPE	colorBackground;
	ATTR_TYPE	colorAttr;
	ATTR_TYPE	bwAttr;
};

struct KcdConfig {
	bool	cfgQuietFull;
	bool	cfgQuietSmart;
	bool	cfgQuietPartial;

	bool	cfgAutoScan;
	bool	cfgSpaceSelect;
	bool	cfgSortTree;
	bool	cfgCaseSensitiveSort;
	bool	cfgGraphicChar;
	bool	cfgScrollBar;

	bool	cfgGlobDot;
	bool	cfgGlobPath;

	KeyBindingType	cfgKey;

	unsigned	cfgShowListThreshold;
	unsigned	cfgFuzzySize;
	unsigned	cfgMouseScrollRate;

	ShowNewDirType	cfgShowNewDir;

	AttrConfig	cfgAttrNormal;
	AttrConfig	cfgAttrLink;
	AttrConfig	cfgAttrHighlight;
	AttrConfig	cfgAttrURL;
	AttrConfig	cfgAttrHeader;
	AttrConfig	cfgAttrBold;
	AttrConfig	cfgAttrItalic;
	AttrConfig	cfgAttrScrollArrow;
	AttrConfig	cfgAttrScrollBlock;
	AttrConfig	cfgAttrScrollBar;
	AttrConfig	cfgAttrTitle;
	AttrConfig	cfgAttrMore;
	AttrConfig	cfgAttrLinkBold;
	AttrConfig	cfgAttrLinkItalic;
	AttrConfig	cfgAttrHighlightBold;
	AttrConfig	cfgAttrHighlightItalic;

	COLOR_TYPE	cfgDefaultBackground;

	DirList	*cfgSkipDir;
	DirList	*cfgStartDir;

	DirList *cfgDefaultTree;

	MountDirList *cfgMountDir;

	KcdConfig();
	~KcdConfig();

	KcdConfig(const KcdConfig &c);
	KcdConfig& operator=(const KcdConfig &c);

	void ForceQuiet(int forceQuiet);	

	void SetAttr(AttrConfig &cfg, int colorFg, int colorBg, 
		     ATTR_TYPE colorAttr, ATTR_TYPE bwAttr) {
		cfg.colorForeground = colorFg;
		cfg.colorBackground = colorBg;
		cfg.colorAttr = colorAttr;
		cfg.bwAttr = bwAttr;
	}
};

typedef map<string, KcdConfig> configMap;
extern KcdConfig kcdConfig;
extern KcdConfig defConfig;
extern configMap allConfig;

bool	FindSkipDir(const string &str);
bool	FindStartDir(const string &str);
bool	FindMountDir(const string &str);

int	k_fnmatch(const string &pattern, const string &str);

#endif	/* __K_CONFOBJ_H */
