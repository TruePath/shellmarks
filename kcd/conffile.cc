/*
	conffile.cc	Configuration File Functions
	Copyright (c) 2000,2001,2002,2003,2004,2005,2007 Kriang Lerdsuwanakij
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

#include "conffile.h"

#include CXX__HEADER_cstdio
#include CXX__HEADER_cctype
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/stat.h>

#include "cstrlib.h"
#include "cxxlib.h"
#include "miscobj.h"
#include "khwin.h"
#include "seek.h"
#include "dirutil.h"

bool	operator== (const DirConfig &dir1, const DirConfig &dir2)
{
	return dir1.dir == dir2.dir && dir1.globDot == dir2.globDot
	       && dir1.globPath == dir2.globPath;
}

/*************************************************************************
	File I/O functions.
*************************************************************************/

// ReadLine
// Return	0 - OK, more lines to read
//		EOF - End-of-file encountered

int	ReadLine(FILE *file, Buffer &s)
{
	int	ret = 0;
	bool	inDoubleQuote = false, inBackSlash = false;
	
	s.Clear();			// Empty buffer

	for ( ; ; ) {			// Arbitray config column width
					// supported
	
		int	c = getc(file);
		if (c == EOF) {
			ret = EOF;
			break;
		}
		else if (c == '\n')
			break;		// ret = 0
		else if (c == '\r')
			continue;	// Ignore
		else if (c == '#' && !inDoubleQuote) {
			do {		// Read till CR or EOF
				c = getc(file);
			} while(c != EOF && c != '\n');
			if (c == EOF)
				ret = EOF;
			break;
		}
		else if (c == '\"') {
			if (!inBackSlash)
				inDoubleQuote = !inDoubleQuote;
		}
		else if (c == '\\') {
			inBackSlash = !inBackSlash;
			if (inBackSlash) {	// Begin escape
				s.Putc(c);	// Write the backslash
						// as is.  LoadConfig(...)
						// will unquote it later.
				continue;	// Don't reset inBackSlash
			}
		}
		else if (c == ' ' || c == '\t') {
					// We don't have to ignore contiguous
					// spaces/tabs here since it may be
					// inside "...".   LoadConfig(...)
					// below are now instructed to ignore
					// them wherever applicable

			c = ' ';	// Store as space
		}
		s.Putc(c);
		inBackSlash = false;
	}
	
	s.TermStr();			// Mark end of string
	return ret;
}

/*************************************************************************
	Error reporting functions.
*************************************************************************/

class ErrorSyntaxIgnored : public ErrorBase {
	public:
		const char *what() { return _("bad file format - command ignored"); }
		int ret() { return RET_SYNERR; }
};

void	PrintErrorCommand(const char *buf)
{
	gtout(cerr, _("        %$     Command ignored\n"))
	     << dump_bytes(buf, 0, strlen(buf), strlen(buf));
}

void	PrintErrorMessage(const char *msg, const char *cmd, const char *buf)
{
	cout << flush;
	gtout(cerr, _("%$: %$ in `%$\' command:\n"))
	     << progName << msg << cmd;
	PrintErrorCommand(buf);
	throw ErrorSyntaxIgnored();
}

void	ExtraChar(const char *cmd, const char *buf)
{
	PrintErrorMessage(_("extra characters"), cmd, buf);
}

void	CommaExpected(const char *cmd, const char *buf)
{
	PrintErrorMessage(_("comma expected"), cmd, buf);
}

void	InvalidValue(const char *cmd, const char *buf)
{
	PrintErrorMessage(_("invalid value"), cmd, buf);
}

void	InvalidAttr(const char *cmd, const char *buf)
{
	PrintErrorMessage(_("invalid attribute"), cmd, buf);
}

void	InvalidColor(const char *cmd, const char *buf)
{
	PrintErrorMessage(_("invalid color"), cmd, buf);
}

void	InvalidOption(const char *cmd, const char *buf)
{
	PrintErrorMessage(_("invalid option"), cmd, buf);
}

void	InvalidRegexOption(const char *cmd, const char *buf)
{
	PrintErrorMessage(_("invalid regex option"), cmd, buf);
}

void	NoRegex(const char *cmd, const char *buf)
{
	PrintErrorMessage(_("regular expression not supported"), cmd, buf);
}

void	MissingOn(const char *cmd, const char *buf)
{
	PrintErrorMessage(_("ON required"), cmd, buf);
}

void	EqualNotFound(const char *cmd, const char *buf)
{
	PrintErrorMessage(_("`=\' required"), cmd, buf);
}

void	QuoteNotFound(const char *cmd, const char *buf)
{
	PrintErrorMessage(_("`\"\' required"), cmd, buf);
}

void	YesNoNotFound(const char *cmd, const char *buf)
{
	PrintErrorMessage(_("only YES or NO is allowed"), cmd, buf);
}

void	NumberRequired(const char *cmd, const char *buf)
{
	PrintErrorMessage(_("non-negative integer required"), cmd, buf);
}

void	BlankEntry(const char *cmd, const char *buf)
{
	PrintErrorMessage(_("blank entry encountered"), cmd, buf);
}

/*************************************************************************
	Command helper functions.
*************************************************************************/

// Return true - Yes, false - No
bool	IsYesNo(const char *cmd, char *buf, size_t from, size_t len)
{
	if (CompareStringCase(buf+from, "yes", 3) == 0) {
		if (!IsEmpty(buf, from+3, len))
			ExtraChar(cmd, buf);
		return true;
	}
	else if (CompareStringCase(buf+from, "no", 2) == 0) {
		if (!IsEmpty(buf, from+2, len))
			ExtraChar(cmd, buf);
		return false;
	}
	else {
		YesNoNotFound(cmd, buf);
		return false;			// Dummy return to avoid
						// compiler warning
	}
}

// Read unsigned number till end of line
// Return >=0 - OK, -1 Error
unsigned	ReadUnsigned(const char *cmd, char *buf, size_t from, size_t len)
{
	unsigned	value = 0;

	if (from >= len) {
		NumberRequired(cmd, buf);
	}

	while (from < len) {
		if (!isdigit(buf[from]))
			NumberRequired(cmd, buf);

		unsigned old_value = value;

		value = value*10 + buf[from]-'0';
		if (value < old_value)		// Overflow
			PrintErrorMessage(_("number out of range"), cmd, buf);

		from++;
	}
	return value;
}

size_t	SeekPastEqualSignOrThrow(const char *cmd, const char *buf, size_t from, size_t len)
{
	try {
		from = SeekPastEqualSign(buf, from, len);
	}
	catch (...) {
		EqualNotFound(cmd, buf);
	}
	return from;
}

size_t	SeekEndQuoteOrThrow(const char *cmd, const char *buf, size_t from, size_t len)
{
	try {
		from = SeekChar('\"', buf, from, len, 2);
	}
	catch (...) {
		QuoteNotFound(cmd, buf);
	}
	return from;
}

/*************************************************************************
	Find string from linked-list
*************************************************************************/

class	IsPatternMatch {
		string	pattern;
	public:
		IsPatternMatch(const string &pattern_) : pattern(pattern_) {}
		int	operator ()(DirItem &str)
		{
					// fnmatch returns 0 if strings match
			return ! k_fnmatch(pattern, str->dir);
		}
		int	operator ()(MountDirItem &str)
		{
					// fnmatch returns 0 if strings match
			return ! k_fnmatch(pattern, str->dir);
		}
};

#ifdef	CLIB_HAVE_REGEX
class	IsRegPatternMatch {
		string	pattern;
		regex_t	regex_buffer;
	public:
		IsRegPatternMatch(const string &pattern_) : pattern(pattern_) {
			k_regcomp(&regex_buffer, pattern, REG_EXTENDED);
		}
		int	operator ()(DirItem &str)
		{
			regmatch_t m;
					// regexec returns 0 if strings match
			return !regexec(&regex_buffer, str->dir.c_str(), 1, &m, 0);
		}
		int	operator ()(MountDirItem &str)
		{
			regmatch_t m;
					// regexec returns 0 if strings match
			return !regexec(&regex_buffer, str->dir.c_str(), 1, &m, 0);
		}
};
#endif

/*************************************************************************
	Command processing/printing value structure.
*************************************************************************/

struct	ConfigValue {
	unsigned KcdConfig::*cfgUIntPtr;
	COLOR_TYPE KcdConfig::*cfgColorPtr;
	bool	KcdConfig::*cfgBoolPtr;
	DirList	* KcdConfig::*cfgStringListPtr;
	MountDirList *KcdConfig::*cfgMountDirListPtr;
	AttrConfig KcdConfig::*cfgAttrPtr;
	KeyBindingType KcdConfig::*cfgKeyPtr;
	ShowNewDirType KcdConfig::*cfgShowNewDirPtr;
};

/*************************************************************************
	Profiles.
*************************************************************************/

string	currentProfile;		// Profile from invoked command line
string	configProfile;		// Current processing profile in
				// configuration file

bool	IsProfileValid(const string &s, bool allowGlobal)
{
	if (s.size() == 0)
		return false;
	if (allowGlobal && s == "*")
		return true;

	if (s[0] != '_' && !isalpha(s[0]))
		return false;
	for (size_t i = 1; i < s.size(); ++i) {
		if (s[i] != '_' && !isalnum(s[i]))
			return false;
	}
	return true;
}

void	SetCurrentProfile(const string &s)
{
	currentProfile = s;
}

string	GetCurrentProfile()
{
	return currentProfile;
}

void	SetConfigProfile(const string &s)
{
	configProfile = s;
}

bool	IsConfigProfileGlobal()
{
	if (configProfile == "*")
		return true;
	return false;
}

/*************************************************************************
	Command processing/printing functions - Yes/No.
*************************************************************************/

void	SetYesNo(const ConfigValue &cfg, bool b)
{
	allConfig[configProfile].*cfg.cfgBoolPtr = b;
}

void	ConfigYesNo(const char *cmd, char *buf, size_t from, size_t len,
		    const ConfigValue &cfg)
{
	from = SeekPastEqualSignOrThrow(cmd, buf, from, len);

	bool	b = IsYesNo(cmd, buf, from, len);
			// Note: error has already been reported by IsYesNo

	SetYesNo(cfg, b);
}

void	PrintYesNo(const char *cmd, const ConfigValue &cfg)
{
	cout << cmd << ": ";
	if (allConfig[configProfile].*cfg.cfgBoolPtr)
		cout << _("Yes\n");
	else
		cout << _("No\n");
}

/*************************************************************************
	Command processing/printing functions - Unsigned integer.
*************************************************************************/

void	SetUnsigned(const ConfigValue &cfg, unsigned u)
{
	allConfig[configProfile].*cfg.cfgUIntPtr = u;
}

void	ConfigUnsigned(const char *cmd, char *buf, size_t from, size_t len,
		       const ConfigValue &cfg)
{
	from = SeekPastEqualSignOrThrow(cmd, buf, from, len);

	unsigned u = ReadUnsigned(cmd, buf, from, len);
			// Note: error has already been reported

	SetUnsigned(cfg, u);
}

void	PrintUnsigned(const char *cmd, const ConfigValue &cfg)
{
	cout << cmd << ": " << allConfig[configProfile].*cfg.cfgUIntPtr << '\n';
}

/*************************************************************************
	Command processing/printing functions - Key binding.
*************************************************************************/

struct	KeyBindingMap {
	const char	*name;
	KeyBindingType	value;
};

#define NUM_KEY_BINDING	4

KeyBindingMap	keyBindingMap[NUM_KEY_BINDING] = {
	{ "default", keyBindingDefault },
	{ "kcd", keyBindingDefault },
	{ "vi", keyBindingVi },
	{ "emacs", keyBindingEmacs }
};


void	SetKeyBinding(const ConfigValue &cfg, KeyBindingType k)
{
	allConfig[configProfile].*cfg.cfgKeyPtr = k;
}

void	ConfigKeyBinding(const char *cmd, char *buf, size_t from, size_t len,
		  const ConfigValue &cfg)
{
	from = SeekPastEqualSignOrThrow(cmd, buf, from, len);

	size_t	i;
	size_t	tok_end = SeekConfigTokenEnd(buf, from, len);

	for (i = 0; i < NUM_KEY_BINDING; ++i) {
		size_t key_len = strlen(keyBindingMap[i].name);
		if (tok_end-from == key_len 
		    && CompareStringCase(keyBindingMap[i].name, buf+from, key_len) == 0)
			break;
	}
	if (i == NUM_KEY_BINDING)
			// Throw exception
		PrintErrorMessage(_("invalid attribute"), cmd, buf);

	from = tok_end;
	from = SeekNonSpace(buf, from, len);
	if (from != len)
		ExtraChar(cmd, buf);

	KeyBindingType k = keyBindingMap[i].value;
		
			// Note: error has already been reported

	SetKeyBinding(cfg, k);
}

void	PrintKeyBinding(const char *cmd, const ConfigValue &cfg)
{
	cout << cmd << ": ";
	switch (allConfig[configProfile].*cfg.cfgKeyPtr) {
		case keyBindingDefault:
			cout << "Default";
			break;
		case keyBindingVi:
			cout << "vi";
			break;
		case keyBindingEmacs:
			cout << "emacs";
			break;
	}
	cout << '\n';
}

/*************************************************************************
	Command processing/printing functions - Show New Directory.
*************************************************************************/

struct	ShowNewDirMap {
	const char	*name;
	ShowNewDirType	value;
};

#define NUM_SHOW_NEW_DIR	4

ShowNewDirMap	showNewDirMap[NUM_SHOW_NEW_DIR] = {
	{ "default", showNewDirMulti },
	{ "no", showNewDirNo },
	{ "yes", showNewDirYes },
	{ "multi", showNewDirMulti }
};


void	SetShowNewDir(const ConfigValue &cfg, ShowNewDirType k)
{
	allConfig[configProfile].*cfg.cfgShowNewDirPtr = k;
}

void	ConfigShowNewDir(const char *cmd, char *buf, size_t from, size_t len,
		  const ConfigValue &cfg)
{
	from = SeekPastEqualSignOrThrow(cmd, buf, from, len);

	size_t	i;
	size_t	tok_end = SeekConfigTokenEnd(buf, from, len);

	for (i = 0; i < NUM_SHOW_NEW_DIR; ++i) {
		size_t key_len = strlen(showNewDirMap[i].name);
		if (tok_end-from == key_len 
		    && CompareStringCase(showNewDirMap[i].name, buf+from, key_len) == 0)
			break;
	}
	if (i == NUM_SHOW_NEW_DIR)
			// Throw exception
		PrintErrorMessage(_("invalid attribute"), cmd, buf);

	from = tok_end;
	from = SeekNonSpace(buf, from, len);
	if (from != len)
		ExtraChar(cmd, buf);

	ShowNewDirType k = showNewDirMap[i].value;
		
			// Note: error has already been reported

	SetShowNewDir(cfg, k);
}

void	PrintShowNewDir(const char *cmd, const ConfigValue &cfg)
{
	cout << cmd << ": ";
	switch (allConfig[configProfile].*cfg.cfgShowNewDirPtr) {
		case showNewDirNo:
			cout << "No";
			break;
		case showNewDirYes:
			cout << "Yes";
			break;
		case showNewDirMulti:
			cout << "Multi";
			break;
	}
	cout << '\n';
}

/*************************************************************************
	Command processing/printing functions - String list.
*************************************************************************/

void	SetStringList(const ConfigValue &cfg, const string &s, bool regex,
		      bool globDot, bool globPath)
{
						// Reference counted
	DirItem	str(new DirConfig(s, regex, globDot, globPath));

	DirList::iterator iter;
	for (iter = (allConfig[configProfile].*cfg.cfgStringListPtr)->begin(); 
	     iter != (allConfig[configProfile].*cfg.cfgStringListPtr)->end(); ++iter) {

						// Same directory name
		if ((*iter)->dir == str->dir) {
			if (**iter != *str) {
						// Different glob options
						// Use new options
				(allConfig[configProfile].*cfg.cfgStringListPtr)->erase(iter);
				(allConfig[configProfile].*cfg.cfgStringListPtr)->push_back(str);
			}

						// Silently ignore
						// duplicate entry
			return;
		}
	} 
						// New entry
	(allConfig[configProfile].*cfg.cfgStringListPtr)->push_back(str);
}

void	ConfigStringList(const char *cmd, char *buf, size_t from, size_t len, 
			 const ConfigValue &cfg)
{
	from = SeekPastEqualSignOrThrow(cmd, buf, from, len);

	bool regex = false;
	if (buf[from] == 'r' || buf[from] == 'R') {
#ifdef	CLIB_HAVE_REGEX
		regex = true;
		from = SeekNonSpace(buf, from+1, len);
#else
		NoRegex(cmd, buf);
#endif
	}

	if (buf[from] != '\"')
		QuoteNotFound(cmd, buf);
	from++;

					// Shell-style quote seek
	size_t end_quote = SeekEndQuoteOrThrow(cmd, buf, from, len);

					// Command argument is "" ?	
	if (from == end_quote)
		BlankEntry(cmd, buf);

					// Attempt to skip "/" ?
	if (cfg.cfgStringListPtr == &KcdConfig::cfgSkipDir
	    && strncmp("/", buf+from, end_quote-from) == 0) {
		PrintErrorMessage(_("cannot skip root directory"), cmd, buf);
	}

					// Process directory
	string	dir(buf+from, end_quote-from);
	string	expanded_dir;
	try {
		expanded_dir = ExpandDirectory(dir, true, regex);

		if (cfg.cfgStringListPtr == &KcdConfig::cfgSkipDir)
			expanded_dir = ResolveSymlink(expanded_dir);
	}
	catch (ErrorGenericSyntax &e) {
		PrintErrorMessage(e.what(), cmd, buf);
	}

	bool globDot = allConfig[configProfile].cfgGlobDot;
	bool globPath = allConfig[configProfile].cfgGlobPath;

	from = SeekNonSpace(buf, end_quote+1, len);
	while (!IsEmpty(buf, from, len)) {
		if (buf[from] != ',')
			CommaExpected(cmd, buf);

		from = SeekNonSpace(buf, from+1, len);
		size_t tok_end = SeekConfigTokenEnd(buf, from, len);
		bool *b = 0;
		if (tok_end-from == 7
		    && CompareStringCase("GlobDot", buf+from, 7) == 0) {
			if (regex)
				InvalidRegexOption(cmd, buf);
			b = &globDot;
		}
		else if (tok_end-from == 8
		    && CompareStringCase("GlobPath", buf+from, 8) == 0) {
			if (regex)
				InvalidRegexOption(cmd, buf);
			b = &globPath;
		}
		else
			InvalidOption(cmd, buf);

		from = SeekPastEqualSignOrThrow(cmd, buf, tok_end, len);

		if (CompareStringCase(buf+from, "yes", 3) == 0) {
			*b = true;
			from += 3;
		}
		else if (CompareStringCase(buf+from, "no", 2) == 0) {
			*b = false;
			from += 2;
		}
		else
			YesNoNotFound(cmd, buf);
	}

	SetStringList(cfg, expanded_dir, regex, globDot, globPath);
}

void	ConfigStringListNoGlob(const char *cmd, char *buf, size_t from, size_t len, 
			       const ConfigValue &cfg)
{
	from = SeekPastEqualSignOrThrow(cmd, buf, from, len);

	if (buf[from] != '\"')
		QuoteNotFound(cmd, buf);
	from++;

					// Shell-style quote seek
	size_t end_quote = SeekEndQuoteOrThrow(cmd, buf, from, len);

					// Command argument is "" ?	
	if (from == end_quote)
		BlankEntry(cmd, buf);

					// Attempt to skip "/" ?
	if (cfg.cfgStringListPtr == &KcdConfig::cfgSkipDir
	    && strncmp("/", buf+from, end_quote-from) == 0) {
		PrintErrorMessage(_("cannot skip root directory"), cmd, buf);
	}

	if (!IsEmpty(buf, end_quote+1, len))
		ExtraChar(cmd, buf);

					// Process directory
	string	dir(buf+from, end_quote-from);
	string	expanded_dir;
	try {
		expanded_dir = ExpandDirectory(dir, true);
	}
	catch (ErrorGenericSyntax &e) {
		PrintErrorMessage(e.what(), cmd, buf);
	}

	SetStringList(cfg, expanded_dir, false,
		      allConfig[configProfile].cfgGlobDot, 
		      allConfig[configProfile].cfgGlobPath);
}

void	PrintStringList(const char *cmd, const ConfigValue &cfg)
{
	cout << cmd << ": ";

	bool	indent = false;
	size_t	num_indent = strlen(cmd)+2;

	for (DirList::iterator iter = (allConfig[configProfile].*cfg.cfgStringListPtr)->begin(); 
	     iter != (allConfig[configProfile].*cfg.cfgStringListPtr)->end(); ++iter) {
		if (indent) {
			for (size_t i = 0; i < num_indent; ++i)
				cout << ' ';
		}
		else
			indent = true; 
		cout << (*iter)->dir;
		if ((*iter)->regex)
			cout << "   [ Regex ]";
		else if ((*iter)->globDot || (*iter)->globPath) {
			cout << "   [";
			if ((*iter)->globDot)
				cout << " GlobDot";
			if ((*iter)->globPath)
				cout << " GlobPath";
			cout << " ]";
		}
		cout << '\n';
	}

				// List is empty
	if (! indent)
		cout << '\n';
}

void	PrintStringListNoGlob(const char *cmd, const ConfigValue &cfg)
{
	cout << cmd << ": ";

	bool	indent = false;
	size_t	num_indent = strlen(cmd)+2;

	for (DirList::iterator iter = (allConfig[configProfile].*cfg.cfgStringListPtr)->begin(); 
	     iter != (allConfig[configProfile].*cfg.cfgStringListPtr)->end(); ++iter) {
		if (indent) {
			for (size_t i = 0; i < num_indent; ++i)
				cout << ' ';
		}
		else
			indent = true; 
		cout << (*iter)->dir << '\n';
	}

				// List is empty
	if (! indent)
		cout << '\n';
}

/*************************************************************************
	Command processing/printing functions - Clear string list.
*************************************************************************/

void	SetClearStringList(const ConfigValue &cfg)
{
	(allConfig[configProfile].*cfg.cfgStringListPtr)->clear();
}

void	SetClearStringList(const ConfigValue &cfg, const string &s, bool regex)
{
#ifdef	CLIB_HAVE_REGEX
	if (regex)
		(allConfig[configProfile].*cfg.cfgStringListPtr)->remove_if(IsRegPatternMatch(s));
	else
#endif
		(allConfig[configProfile].*cfg.cfgStringListPtr)->remove_if(IsPatternMatch(s));
}

void	ConfigClearStringList(const char *cmd, char *buf, size_t from, size_t len, 
			      const ConfigValue &cfg)
{
	from = SeekPastEqualSignOrThrow(cmd, buf, from, len);

	if (CompareStringCase("all", buf+from, 3)) {

					// Expect directory name

		bool regex = false;
		if (buf[from] == 'r' || buf[from] == 'R') {
#ifdef	CLIB_HAVE_REGEX
			regex = true;
			from = SeekNonSpace(buf, from+1, len);
#else
			NoRegex(cmd, buf);
#endif
		}

		if (buf[from] != '\"')
			QuoteNotFound(cmd, buf);
		from++;

					// Shell-style quote seek
		size_t end_quote = SeekEndQuoteOrThrow(cmd, buf, from, len);

		if (!IsEmpty(buf, end_quote+1, len))
			ExtraChar(cmd, buf);

					// Command argument is "" ?	
		if (from == end_quote)
			BlankEntry(cmd, buf);

					// Process directory
		string	dir(buf+from, end_quote-from);
		string	expanded_dir;
		try {
			expanded_dir = ExpandDirectory(dir, true, regex);
		}
		catch (ErrorGenericSyntax &e) {
			PrintErrorMessage(e.what(), cmd, buf);
		}

		SetClearStringList(cfg, expanded_dir, regex);
	}
	else {		
					// Clear everything
	
		if (!IsEmpty(buf, from+3, len))
			ExtraChar(cmd, buf);

		SetClearStringList(cfg);
	}
}

/*************************************************************************
	Command processing/printing functions - MountDir list.
*************************************************************************/

void	SetMountDirList(const ConfigValue &cfg, const string &s, 
			const MountActionOrderType &mountAction)
{
						// Reference counted
	MountDirItem	str(new MountDirConfig(s, false, false, false,
					       mountAction));

	MountDirList::iterator iter;
	for (iter = (allConfig[configProfile].*cfg.cfgMountDirListPtr)->begin(); 
	     iter != (allConfig[configProfile].*cfg.cfgMountDirListPtr)->end(); ++iter) {

						// Same directory name
		if ((*iter)->dir == str->dir) {
			if (**iter != *str) {
						// Different glob options
						// Use new options
				(allConfig[configProfile].*cfg.cfgMountDirListPtr)->erase(iter);
				(allConfig[configProfile].*cfg.cfgMountDirListPtr)->push_back(str);
			}

						// Silently ignore
						// duplicate entry
			return;
		}
	} 
						// New entry
	(allConfig[configProfile].*cfg.cfgMountDirListPtr)->push_back(str);
}

void	ConfigMountDirList(const char *cmd, char *buf, size_t from, size_t len, 
			   const ConfigValue &cfg)
{
	from = SeekPastEqualSignOrThrow(cmd, buf, from, len);

	if (buf[from] != '\"')
		QuoteNotFound(cmd, buf);
	from++;

					// Shell-style quote seek
	size_t end_quote = SeekEndQuoteOrThrow(cmd, buf, from, len);

					// Command argument is "" ?	
	if (from == end_quote)
		BlankEntry(cmd, buf);

					// Process directory
	string	dir(buf+from, end_quote-from);
	string	expanded_dir;
	try {
		expanded_dir = ExpandDirectory(dir, true);

		expanded_dir = ResolveSymlink(expanded_dir);
	}
	catch (ErrorGenericSyntax &e) {
		PrintErrorMessage(e.what(), cmd, buf);
	}

	MountActionOrderType mountAction;

	from = SeekNonSpace(buf, end_quote+1, len);
	while (!IsEmpty(buf, from, len)) {
		if (buf[from] != ',')
			CommaExpected(cmd, buf);

		from = SeekNonSpace(buf, from+1, len);
		size_t tok_end = SeekConfigTokenEnd(buf, from, len);
		if (tok_end-from == 4
			 && CompareStringCase("Skip", buf+from, 4) == 0) {
			mountAction.push_back(mountActionSkip);
			from += 4;
		}
/* 		else if (tok_end-from == 8
			 && CompareStringCase("FileList", buf+from, 8) == 0) {
			mountAction.push_back(mountActionFileList);
			from += 8;
		}*/
		else if (tok_end-from == 3
			 && CompareStringCase("All", buf+from, 3) == 0) {
			mountAction.push_back(mountActionAll);
			from += 7;
		}
		else if (tok_end-from == 4
			 && CompareStringCase("Tree", buf+from, 4) == 0) {
			mountAction.push_back(mountActionTree);
			from += 8;
		}
		else
			InvalidOption(cmd, buf);
	}

	SetMountDirList(cfg, expanded_dir, mountAction);
}

void	PrintMountDirList(const char *cmd, const ConfigValue &cfg)
{
	cout << cmd << ": ";

	bool	indent = false;
	size_t	num_indent = strlen(cmd)+2;

	for (MountDirList::iterator iter
	       = (allConfig[configProfile].*cfg.cfgMountDirListPtr)->begin(); 
	     iter != (allConfig[configProfile].*cfg.cfgMountDirListPtr)->end(); ++iter) {
		if (indent) {
			for (size_t i = 0; i < num_indent; ++i)
				cout << ' ';
		}
		else
			indent = true; 
		cout << (*iter)->dir;

		bool first = true;
		const MountActionOrderType &mountAction = (*iter)->mountAction;

		for (MountActionOrderType::const_iterator iter2 = mountAction.begin();
		     iter2 != mountAction.end(); ++iter2) {

			if (first)
				cout << ' ';

			switch (*iter2) {
				case mountActionSkip:
					cout << " Skip";
					break;
				case mountActionAll:
					cout << " All";
					break;
				case mountActionTree:
					cout << " Tree";
					break;
				case mountActionFileList:
					cout << " FileList";
					break;
			}

			first = false;
		}

		cout << '\n';
	}

				// List is empty
	if (! indent)
		cout << '\n';
}

/*************************************************************************
	Command processing/printing functions - Clear MountDir list.
*************************************************************************/

void	SetClearMountDirList(const ConfigValue &cfg)
{
	(allConfig[configProfile].*cfg.cfgMountDirListPtr)->clear();
}

void	SetClearMountDirList(const ConfigValue &cfg, const string &s, bool regex)
{
#ifdef	CLIB_HAVE_REGEX
	if (regex)
		(allConfig[configProfile].*cfg.cfgMountDirListPtr)->remove_if(IsRegPatternMatch(s));
	else
#endif
		(allConfig[configProfile].*cfg.cfgMountDirListPtr)->remove_if(IsPatternMatch(s));
}

void	ConfigClearMountDirList(const char *cmd, char *buf, size_t from, size_t len, 
			      const ConfigValue &cfg)
{
	from = SeekPastEqualSignOrThrow(cmd, buf, from, len);

	if (CompareStringCase("all", buf+from, 3)) {

					// Expect directory name

		bool regex = false;
		if (buf[from] == 'r' || buf[from] == 'R') {
#ifdef	CLIB_HAVE_REGEX
			regex = true;
			from = SeekNonSpace(buf, from+1, len);
#else
			NoRegex(cmd, buf);
#endif
		}

		if (buf[from] != '\"')
			QuoteNotFound(cmd, buf);
		from++;

					// Shell-style quote seek
		size_t end_quote = SeekEndQuoteOrThrow(cmd, buf, from, len);

		if (!IsEmpty(buf, end_quote+1, len))
			ExtraChar(cmd, buf);

					// Command argument is "" ?	
		if (from == end_quote)
			BlankEntry(cmd, buf);

					// Process directory
		string	dir(buf+from, end_quote-from);
		string	expanded_dir;
		try {
			expanded_dir = ExpandDirectory(dir, true, regex);
		}
		catch (ErrorGenericSyntax &e) {
			PrintErrorMessage(e.what(), cmd, buf);
		}

		SetClearMountDirList(cfg, expanded_dir, regex);
	}
	else {		
					// Clear everything
	
		if (!IsEmpty(buf, from+3, len))
			ExtraChar(cmd, buf);

		SetClearMountDirList(cfg);
	}
}

/*************************************************************************
	Command processing/printing functions - Attributes
*************************************************************************/

struct	AttrColorMap {
	const char	*name;
	ATTR_TYPE	value;
};

#define NUM_ATTR	7
#define NUM_COLOR	9
AttrColorMap attrMap[NUM_ATTR] = {
	{ "normal", A_NORMAL },		// First entry must be A_NORMAL
	{ "standout", A_STANDOUT },
	{ "underline", A_UNDERLINE },
	{ "reverse", A_REVERSE },
	{ "blink", A_BLINK },
	{ "dim", A_DIM },
	{ "bold", A_BOLD }
};
AttrColorMap colorMap[NUM_COLOR] = {
	{ "black", COLOR_BLACK },
	{ "red", COLOR_RED },
	{ "green", COLOR_GREEN },
	{ "yellow", COLOR_YELLOW },
	{ "blue", COLOR_BLUE },
	{ "magenta", COLOR_MAGENTA },
	{ "cyan", COLOR_CYAN },
	{ "white", COLOR_WHITE },
	{ "default", DEFAULT_COLOR }
};

/*************************************************************************
	Command processing/printing functions - Color attributes
*************************************************************************/

void	SetColorAttr(const ConfigValue &cfg, ATTR_TYPE colorAttr,
		     COLOR_TYPE colorForeground, COLOR_TYPE colorBackground)
{
	(allConfig[configProfile].*cfg.cfgAttrPtr).colorAttr = colorAttr;
	(allConfig[configProfile].*cfg.cfgAttrPtr).colorForeground = colorForeground;
	(allConfig[configProfile].*cfg.cfgAttrPtr).colorBackground = colorBackground;
}

void	ConfigColorAttr(const char *cmd, char *buf, size_t from, size_t len, 
			const ConfigValue &cfg)
{
	from = SeekPastEqualSignOrThrow(cmd, buf, from, len);

	bool first_attr = true;
	bool found_normal = false;
	ATTR_TYPE colorAttr = A_NORMAL;
	COLOR_TYPE colorForeground = COLOR_WHITE;
	COLOR_TYPE colorBackground = COLOR_BLACK;
	size_t i, tok_end;
	for ( ; ; ) {
		tok_end = SeekConfigTokenEnd(buf, from, len);
		for (i = 0; i < NUM_ATTR; ++i) {
			size_t attr_len = strlen(attrMap[i].name);
			if (tok_end-from == attr_len 
			    && CompareStringCase(attrMap[i].name, buf+from, attr_len) == 0) {

				if (i == 0) {
					if (!first_attr)
						// normal not the first value
						InvalidValue(cmd, buf);
					found_normal = true;
				}
				else {
					if (found_normal)
						// normal not the last value
						InvalidValue(cmd, buf);
					colorAttr |= attrMap[i].value;
				}
				break;
			}
		}
		if (i == NUM_ATTR) {
						// Unknown attribute, assume it is a color
			break;
		}

		from = tok_end;
		if (IsEmpty(buf, from, len))
						// Color missing
			InvalidAttr(cmd, buf);

		from = SeekNonSpace(buf, from, len);
		first_attr = false;
	}

	tok_end = SeekConfigTokenEnd(buf, from, len);
	for (i = 0; i < NUM_COLOR; ++i) {
		size_t color_len = strlen(colorMap[i].name);
		if (tok_end-from == color_len 
		    && CompareStringCase(colorMap[i].name, buf+from, color_len) == 0) {

			colorForeground = colorMap[i].value;
			break;
		}
	}
	if (i == NUM_COLOR || i == (NUM_COLOR-1))	// Don't accept default here
						// Unknown color
		InvalidColor(cmd, buf);

	from = tok_end;
	from = SeekNonSpace(buf, from, len);
	if (from == len)
		InvalidValue(cmd, buf);

	if (CompareStringCase("on", buf+from, 2))
		MissingOn(cmd, buf);
	from += 2;

	from = SeekNonSpace(buf, from, len);
	if (from == len)
		InvalidValue(cmd, buf);

	tok_end = SeekConfigTokenEnd(buf, from, len);
	for (i = 0; i < NUM_COLOR; ++i) {
		size_t color_len = strlen(colorMap[i].name);
		if (tok_end-from == color_len 
		    && CompareStringCase(colorMap[i].name, buf+from, color_len) == 0) {

			colorBackground = colorMap[i].value;
			break;
		}
	}
	if (i == NUM_COLOR)		// Unknown color
		InvalidColor(cmd, buf);
	from = tok_end;
	from = SeekNonSpace(buf, from, len);
	if (from != len)
		ExtraChar(cmd, buf);

	SetColorAttr(cfg, colorAttr, colorForeground, colorBackground);
}

void	PrintColorAttr(const char *cmd, const ConfigValue &cfg)
{
	cout << cmd << ": ";
	if ((allConfig[configProfile].*cfg.cfgAttrPtr).colorAttr) {
		for (int i = 1; i < NUM_ATTR; ++i) {
			if ((allConfig[configProfile].*cfg.cfgAttrPtr).colorAttr & attrMap[i].value)
				cout << attrMap[i].name << ' ';
		}
	}

	for (size_t i = 0; i < NUM_COLOR; ++i) {
		if ((allConfig[configProfile].*cfg.cfgAttrPtr).colorForeground == colorMap[i].value) {
			cout << colorMap[i].name << ' ';
			break;
		}
	}

	cout << "on ";

	for (size_t i = 0; i < NUM_COLOR; ++i) {
		if ((allConfig[configProfile].*cfg.cfgAttrPtr).colorBackground == colorMap[i].value) {
			cout << colorMap[i].name;
			break;
		}
	}
	cout << '\n';
}

/*************************************************************************
	Command processing/printing functions - Color
*************************************************************************/

void	SetColor(const ConfigValue &cfg, COLOR_TYPE c)
{
	allConfig[configProfile].*cfg.cfgColorPtr = c;
}

void	ConfigColor(const char *cmd, char *buf, size_t from, size_t len, 
		    const ConfigValue &cfg)
{
	from = SeekPastEqualSignOrThrow(cmd, buf, from, len);

	COLOR_TYPE color = COLOR_BLACK;		// Avoid compiler warning
	size_t i, tok_end;

	tok_end = SeekConfigTokenEnd(buf, from, len);
	for (i = 0; i < NUM_COLOR; ++i) {
		size_t color_len = strlen(colorMap[i].name);
		if (tok_end-from == color_len 
		    && CompareStringCase(colorMap[i].name, buf+from, color_len) == 0) {

			color = colorMap[i].value;
			break;
		}
	}
	if (i == NUM_COLOR || i == (NUM_COLOR-1))	// Don't accept default here
						// Unknown color
		InvalidColor(cmd, buf);

	from = tok_end;
	from = SeekNonSpace(buf, from, len);
	if (from != len)
		ExtraChar(cmd, buf);

	SetColor(cfg, color);
}

void	PrintColor(const char *cmd, const ConfigValue &cfg)
{
	cout << cmd << ": ";
	for (size_t i = 0; i < NUM_COLOR; ++i) {
		if (allConfig[configProfile].*cfg.cfgColorPtr == colorMap[i].value) {
			cout << colorMap[i].name << ' ';
			break;
		}
	}
	cout << '\n';
}

/*************************************************************************
	Command processing/printing functions - Black & white attributes
*************************************************************************/

void	SetBWAttr(const ConfigValue &cfg, ATTR_TYPE a)
{
	(allConfig[configProfile].*cfg.cfgAttrPtr).bwAttr = a;
}

void	ConfigBWAttr(const char *cmd, char *buf, size_t from, size_t len, 
		     const ConfigValue &cfg)
{
	from = SeekPastEqualSignOrThrow(cmd, buf, from, len);

	bool first_attr = true;
	bool found_normal = false;
	ATTR_TYPE bwAttr = A_NORMAL;
	for ( ; ; ) {
		size_t tok_end = SeekConfigTokenEnd(buf, from, len);
		size_t i;
		for (i = 0; i < NUM_ATTR; ++i) {
			size_t attr_len = strlen(attrMap[i].name);
			if (tok_end-from == attr_len 
			    && CompareStringCase(attrMap[i].name, buf+from, attr_len) == 0) {

				if (i == 0) {
					if (!first_attr)
						// normal not the first value
						InvalidValue(cmd, buf);
					found_normal = true;
				}
				else {
					if (found_normal)
						// normal not the last value
						InvalidValue(cmd, buf);
					bwAttr |= attrMap[i].value;
				}
				break;
			}
		}
		if (i == NUM_ATTR)		// Unknown attribute
			InvalidAttr(cmd, buf);

		from = tok_end;
		from = SeekNonSpace(buf, from, len);
		if (from == len) {		// No more attribute
			SetBWAttr(cfg, bwAttr);
			return;
		}

		first_attr = false;
	}
}

void	PrintBWAttr(const char *cmd, const ConfigValue &cfg)
{
	cout << cmd << ": ";
	if ((allConfig[configProfile].*cfg.cfgAttrPtr).bwAttr) {
		for (int i = 1; i < NUM_ATTR; ++i) {
			if ((allConfig[configProfile].*cfg.cfgAttrPtr).bwAttr & attrMap[i].value)
				cout << attrMap[i].name << ' ';
		}
	}
	else
		cout << attrMap[0].name;
	cout << '\n';
}

/*************************************************************************
	Command processing/printing table.
*************************************************************************/

struct	ConfigFunc {
	void	(*cfgReadFunc)(const char *cmd, char *buf, size_t from, size_t len, const ConfigValue &cfg);
	void	(*cfgPrintFunc)(const char *cmd, const ConfigValue &cfg);
};

struct	ConfigInfo {
	const char	*cfgName;
	ConfigFunc	*cfgFunc;
	ConfigValue	cfgValue;
};

ConfigFunc cfgFuncYesNo = { ConfigYesNo, PrintYesNo };
ConfigFunc cfgFuncUnsigned = { ConfigUnsigned, PrintUnsigned };
ConfigFunc cfgFuncKeyBinding = { ConfigKeyBinding, PrintKeyBinding };
ConfigFunc cfgFuncShowNewDir = { ConfigShowNewDir, PrintShowNewDir };
ConfigFunc cfgFuncStringList = { ConfigStringList, PrintStringList };
ConfigFunc cfgFuncStringListNoGlob = { ConfigStringListNoGlob, PrintStringListNoGlob };
ConfigFunc cfgFuncClearStringList = { ConfigClearStringList, 0 };
ConfigFunc cfgFuncMountDirList = { ConfigMountDirList, PrintMountDirList };
ConfigFunc cfgFuncClearMountDirList = { ConfigClearMountDirList, 0 };
ConfigFunc cfgFuncColorAttr = { ConfigColorAttr, PrintColorAttr };
ConfigFunc cfgFuncColor = { ConfigColor, PrintColor };
ConfigFunc cfgFuncBWAttr = { ConfigBWAttr, PrintBWAttr };

#define NUM_CONFIG_INFO	46
ConfigInfo configInfo[NUM_CONFIG_INFO] = {
	{ "QuietFullScan", &cfgFuncYesNo, { 0, 0, &KcdConfig::cfgQuietFull, 0, 0, 0, 0, 0 } },
	{ "QuietSmartScan", &cfgFuncYesNo, { 0, 0, &KcdConfig::cfgQuietSmart, 0, 0, 0, 0, 0 } },
	{ "QuietPartialScan", &cfgFuncYesNo, { 0, 0, &KcdConfig::cfgQuietPartial, 0, 0, 0, 0, 0 } },
	{ "AutoScan", &cfgFuncYesNo, { 0, 0, &KcdConfig::cfgAutoScan, 0, 0, 0, 0, 0 } },
	{ "SpaceSelect", &cfgFuncYesNo, { 0, 0, &KcdConfig::cfgSpaceSelect, 0, 0, 0, 0, 0 } },
	{ "SortTree", &cfgFuncYesNo, { 0, 0, &KcdConfig::cfgSortTree, 0, 0, 0, 0, 0 } },
	{ "CaseSensitiveSort", &cfgFuncYesNo, { 0, 0, &KcdConfig::cfgCaseSensitiveSort, 0, 0, 0, 0, 0 } },
	{ "GraphicChar", &cfgFuncYesNo, { 0, 0, &KcdConfig::cfgGraphicChar, 0, 0, 0, 0, 0 } },
	{ "ScrollBar", &cfgFuncYesNo, { 0, 0, &KcdConfig::cfgScrollBar, 0, 0, 0, 0, 0 } },
	{ "GlobDot", &cfgFuncYesNo, { 0, 0, &KcdConfig::cfgGlobDot, 0, 0, 0, 0, 0 } },
	{ "GlobPath", &cfgFuncYesNo, { 0, 0, &KcdConfig::cfgGlobPath, 0, 0, 0, 0, 0 } },

	{ "KeyBinding", &cfgFuncKeyBinding, { 0, 0, 0, 0, 0, 0, &KcdConfig::cfgKey, 0 } },
	{ "ShowNewDir", &cfgFuncShowNewDir, { 0, 0, 0, 0, 0, 0, 0, &KcdConfig::cfgShowNewDir } },

	{ "ShowListThreshold", &cfgFuncUnsigned, { &KcdConfig::cfgShowListThreshold, 0, 0, 0, 0, 0, 0, 0 } },
	{ "FuzzySize", &cfgFuncUnsigned, { &KcdConfig::cfgFuzzySize, 0, 0, 0, 0, 0, 0, 0 } },
	{ "MouseScrollRate", &cfgFuncUnsigned, { &KcdConfig::cfgMouseScrollRate, 0, 0, 0, 0, 0, 0, 0 } },

	{ "StartDir", &cfgFuncStringListNoGlob, { 0, 0, 0, &KcdConfig::cfgStartDir, 0, 0, 0, 0 } },
	{ "SkipDir", &cfgFuncStringList, { 0, 0, 0, &KcdConfig::cfgSkipDir, 0, 0, 0, 0 } },

	{ "DefaultTree", &cfgFuncStringListNoGlob, { 0, 0, 0, &KcdConfig::cfgDefaultTree, 0, 0, 0, 0 } },

	{ "ClearStartDir", &cfgFuncClearStringList, { 0, 0, 0, &KcdConfig::cfgStartDir, 0, 0, 0, 0 } },
	{ "ClearSkipDir", &cfgFuncClearStringList, { 0, 0, 0, &KcdConfig::cfgSkipDir, 0, 0, 0, 0 } },

	{ "MountDir", &cfgFuncMountDirList, { 0, 0, 0, 0, &KcdConfig::cfgMountDir, 0, 0, 0 } },
	{ "ClearMountDir", &cfgFuncClearMountDirList, { 0, 0, 0, 0, &KcdConfig::cfgMountDir, 0, 0, 0 } },

	{ "NormalColorAttr", &cfgFuncColorAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrNormal, 0, 0 } },
	{ "NormalBWAttr", &cfgFuncBWAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrNormal, 0, 0 } },
	{ "DirColorAttr", &cfgFuncColorAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrLink, 0, 0 } },
	{ "DirBWAttr", &cfgFuncBWAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrLink, 0, 0 } },
	{ "HighlightDirColorAttr", &cfgFuncColorAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrHighlight, 0, 0 } },
	{ "HighlightDirBWAttr", &cfgFuncBWAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrHighlight, 0, 0 } },
	{ "SymLinkColorAttr", &cfgFuncColorAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrLinkItalic, 0, 0 } },
	{ "SymLinkBWAttr", &cfgFuncBWAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrLinkItalic, 0, 0 } },
	{ "HighlightSymLinkColorAttr", &cfgFuncColorAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrHighlightItalic, 0, 0 } },
	{ "HighlightSymLinkAttr", &cfgFuncBWAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrHighlightItalic, 0, 0 } },

	{ "TitleColorAttr", &cfgFuncColorAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrTitle, 0, 0 } },
	{ "TitleBWAttr", &cfgFuncBWAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrTitle, 0, 0 } },
	{ "StatusColorAttr", &cfgFuncColorAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrURL, 0, 0 } },
	{ "StatusBWAttr", &cfgFuncBWAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrURL, 0, 0 } },
	{ "MoreColorAttr", &cfgFuncColorAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrMore, 0, 0 } },
	{ "MoreBWAttr", &cfgFuncBWAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrMore, 0, 0 } },

	{ "ScrollArrowColorAttr", &cfgFuncColorAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrScrollArrow, 0, 0 } },
	{ "ScrollArrowBWAttr", &cfgFuncBWAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrScrollArrow, 0, 0 } },
	{ "ScrollBlockColorAttr", &cfgFuncColorAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrScrollBlock, 0, 0 } },
	{ "ScrollBlockBWAttr", &cfgFuncBWAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrScrollBlock, 0, 0 } },
	{ "ScrollBarColorAttr", &cfgFuncColorAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrScrollBar, 0, 0 } },
	{ "ScrollBarBWAttr", &cfgFuncBWAttr, { 0, 0, 0, 0, 0, &KcdConfig::cfgAttrScrollBar, 0, 0 } },

	{ "DefaultBackground", &cfgFuncColor, { 0, &KcdConfig::cfgDefaultBackground, 0, 0, 0, 0, 0, 0 } }
};

/*************************************************************************
	Secure user configuration file
*************************************************************************/

static bool	checkSecureUserConfig = false;

void	NeedSecureUserConfig()
{
	checkSecureUserConfig = true;
}

/*************************************************************************
	Read configuration functions.
*************************************************************************/

void	LoadConfig(const string &confFile, bool quiet, bool checkSecure = false)
{
	FILE	*file;
	int	ret = 0;		// OK
	size_t	configLen;
	int	numError = 0;

	Buffer	configLineBuf(true, 100);	// 100 = "Good" initial size
	
	file = k_fopen(confFile, "rb");
	if (file == NULL)			// File not found
		return;

	SetConfigProfile("*");			// Default profile

	try {
		if (checkSecure) {
			struct stat	stat1, stat2;

			// No configuration file found, just use default value.
			if (k_lstat(confFile, &stat1) || fstat(fileno(file), &stat2))
				throw ErrorGenericFile(_("cannot obtain status for configuration file"));

			if (S_ISLNK(stat1.st_mode))
				throw ErrorGenericFile(_("configuration cannot be a symbolic link"));

			if (! S_ISREG(stat1.st_mode))
				throw ErrorGenericFile(_("configuration file must be a regular file"));

			if (stat1.st_nlink > 1)
				throw ErrorGenericFile(_("invalid link count for configuration file"));

						// Compare everything except st_atime
			if (stat1.st_dev != stat2.st_dev || stat1.st_ino != stat2.st_ino
			    || stat1.st_mode != stat2.st_mode || stat1.st_nlink != stat2.st_nlink
			    || stat1.st_uid != stat2.st_uid || stat1.st_gid != stat2.st_gid
			    || stat1.st_rdev != stat2.st_rdev || stat1.st_size != stat2.st_size
			    || stat1.st_blksize != stat2.st_blksize || stat1.st_blocks != stat2.st_blocks
			    || stat1.st_mtime != stat2.st_mtime || stat1.st_ctime != stat2.st_ctime)
				throw ErrorGenericFile(_("race condition detected"));
		}

		if (!quiet)
			gtout(cout, _("%$: loading configuration from %$\n"))
			     << progName << confFile;
	
		for ( ; ; ) {
			if (ret == EOF)
				break;
			
			ret = ReadLine(file, configLineBuf);
		
				// Need to obtain ptr after each ReadLine
				// since ptr may be changed when buffer
				// expands
			char *configLine = configLineBuf.GetPtr();
			configLen = strlen(configLine);
		
			if (configLen > 0) {	// Non empty/non comment line
				size_t i = SeekNonSpace(configLine, 0, configLen);
				if (i == configLen)	// Blank line
					continue;	// Check for EOF and 
							// read another line

							// Profile
				if (configLine[i] == '[') {
					size_t j = 0;
					try {
						j = SeekChar(']', configLine, i+1, configLen, 2);
					}
					catch (...) {
						cerr << progName
						     << _(": missing `]\'\n");
						PrintErrorCommand(configLine);
						numError++;
						continue;
					}

					string s(configLine+i+1, j-i-1);

					if (!IsEmpty(configLine, j+1, configLen)) {
						cerr << progName
						     << _(": extra characters after profile name\n");
						PrintErrorCommand(configLine);
						numError++;
						continue;
					}

					if (!IsProfileValid(s, true)) {
						cerr << progName
						     << _(": invalid characters in profile name\n");
						PrintErrorCommand(configLine);
						numError++;
						continue;
					}
					SetConfigProfile(s);

							// Initialize from [*] if not exist
					if (!IsConfigProfileGlobal() && allConfig.find(s) == allConfig.end())
						allConfig[s] = allConfig["*"];

					continue;
				}

				int	k;
				for (k = 0; k < NUM_CONFIG_INFO; k++) {
					const char	*cfgName = configInfo[k].cfgName;
					size_t	cfgNameLen = strlen(cfgName);
				
					if (CompareStringCase(cfgName, configLine+i, cfgNameLen) == 0) {
						i += cfgNameLen;
						try {
							configInfo[k].cfgFunc->cfgReadFunc(cfgName, configLine, i, 
									      configLen,
									      configInfo[k].cfgValue);
						}
						catch (ErrorSyntaxIgnored &) {
							numError++;
						}
						break;
					}
				}
					
				if (k == NUM_CONFIG_INFO) {
					cerr << progName
					     << _(": unrecognized command:\n");
					PrintErrorCommand(configLine);
					numError++;
							
					throw ErrorGenericSyntax(_("Error found, program aborted"));
				}
			}
		} 
	}
	catch (...) {
		fclose(file);
		throw;
	}

	fclose(file);
	
	if (numError > 0) {
		cerr << _("Error found, press Enter key to continue...\n");
		while (getchar() != '\n')
			;
	}
}

void	LoadAllConfig(bool quiet)
{
	static int	loaded = 0;
	
	if (loaded)
		return;
	
	loaded = 1;

	allConfig["*"] = defConfig;
	LoadConfig(KCD_ETCDIR "/kcd.conf", quiet);
	LoadConfig(confLocalFile, quiet, checkSecureUserConfig);

					// Initialize from [*] if not exist
	if (allConfig.find(GetCurrentProfile()) == allConfig.end()) {
		kcdConfig = allConfig["*"];
		allConfig[GetCurrentProfile()] = kcdConfig;
	}
	else
		kcdConfig = allConfig[GetCurrentProfile()];

			// Transfer all config to relevant variables
	isACSFallBack = !kcdConfig.cfgGraphicChar;
	
	khGeometryManager::disableScrollBar = !kcdConfig.cfgScrollBar;
	NCScreenManager::SetMouseScrollRate(kcdConfig.cfgMouseScrollRate);
}

/*************************************************************************
	Display configuration function.
*************************************************************************/

void	PrintProfile()
{
	gtout(cout, _("Profile: %$\n")) << GetCurrentProfile();
}

void	PrintConfig()
{
	PrintProfile();
	SetConfigProfile(GetCurrentProfile());
	for (int i = 0; i < NUM_CONFIG_INFO; ++i) {
		// Ignore attribute and color handled in PrintAttrConfig
		if (configInfo[i].cfgFunc->cfgPrintFunc && !configInfo[i].cfgValue.cfgAttrPtr
		    && configInfo[i].cfgFunc->cfgPrintFunc != PrintColor)
			configInfo[i].cfgFunc->cfgPrintFunc(configInfo[i].cfgName,
						   configInfo[i].cfgValue);
	}
}

void	PrintAttrConfig()
{
	PrintProfile();
	SetConfigProfile(GetCurrentProfile());
	for (int i = 0; i < NUM_CONFIG_INFO; ++i) {
		if (configInfo[i].cfgFunc->cfgPrintFunc && 
		    (configInfo[i].cfgValue.cfgAttrPtr || configInfo[i].cfgFunc->cfgPrintFunc == PrintColor))
			configInfo[i].cfgFunc->cfgPrintFunc(configInfo[i].cfgName,
						   configInfo[i].cfgValue);
	}
}

/*************************************************************************
	Read/write configuration in directory tree file.
*************************************************************************/

void	ReadDirFileConfig(gzFile /*file*/)
{
}

void	WriteDirFileConfig(gzFile /*file*/)
{
}
