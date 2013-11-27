/*
	gentree.cc	Generate Directory Tree For Display
	Copyright (c) 2000, 2002, 2003, 2004, 2005, 2009 Kriang Lerdsuwanakij
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

#include "gentree.h"

#include "confobj.h"
#include "buffer.h"
#include "miscobj.h"
#include "khwin.h"
#include "dirtree.h"
#include "dirutil.h"
#include "cstrlib.h"
#include "scandir.h"

//#define DUMP_HTML	
#ifdef DUMP_HTML
# include <fcntl.h>
# include <sys/stat.h>
# include <sys/types.h>
#endif

#include CXX__HEADER_algorithm
#include CXX__HEADER_fstream

string	saveCwd;

extern string	dirFile;

string	my_strftime(const char *format, const struct tm *tm)
{
	Buffer buf(1000);
	strftime(buf.GetPtr(), buf.GetAllocSize(), format, tm);
	return string(buf.GetPtr());
}

/*************************************************************************
	Generate commonly used strings
*************************************************************************/

string	html_unreadable;
string	html_skipped;
string	html_outside_tree;
string	html_not_in_file;
string	html_not_found;
string	html_no_filelist;

size_t	html_unreadable_width;
size_t	html_skipped_width;
size_t	html_outside_tree_width;
size_t	html_not_in_file_width;
size_t	html_not_found_width;
size_t	html_no_filelist_width;

void	GenerateHTMLStrings()
{
	html_unreadable = _(" [unreadable]");
	html_skipped = _(" [skipped]");
	html_outside_tree = _(" [outside tree]");
	html_not_in_file = _(" [*]");
	html_not_found = _(" [not found]");
	html_no_filelist = _(" [no filelist]");

	html_unreadable_width = GetStringWidth(html_unreadable);
	html_skipped_width = GetStringWidth(html_skipped);
	html_outside_tree_width = GetStringWidth(html_outside_tree);
	html_not_in_file_width = GetStringWidth(html_not_in_file);
	html_not_found_width = GetStringWidth(html_not_found);
	html_no_filelist_width = GetStringWidth(html_no_filelist);
}

/*************************************************************************
	Generate HTML-like codes from the directory tree
*************************************************************************/

size_t	htmlRow, htmlCol;

void	ReplaceControlChar(string &s)
{
	for (size_t i = 0; i < s.size(); ++i) {
						// Characters interpreted by
						// DrawString
		if (s[i] == '\t' || s[i] == '\r' || s[i] == '\b' || s[i] == '\0'
		    || s[i] == '\n')
			s[i] = '?';
	}
}

class HyperTreeDraw : public HyperDraw {
	private:
		sptr_list<DirectoryEntry> *dirTree;

		vector<size_t>	barStack;	// Positions of vertical bars
		size_t	curCol;			// curRow not needed

		void	NewLineUpdateRowCol();

		void	GenerateSymLinkHTML(HyperDocument &html, WINDOW *pad,
					    sptr<DirectoryEntry> &d,
					    const string &new_dir);

		void	GenerateDirHTML(HyperDocument &html, WINDOW *pad,
					sptr<DirectoryEntry> &d,
					const string &new_dir);

		void	RecursiveGenerateHTML(HyperDocument &html, WINDOW *pad,
					      sptr<DirectoryEntry> &d, 
					      size_t pos, const string &current_dir);
		void	DrawBegin(HyperDocument &html, WINDOW *pad);
		void	DrawEnd(HyperDocument &html, WINDOW *pad);
	public:
		void	Draw(HyperDocument &html, WINDOW *pad);
		HyperTreeDraw(sptr_list<DirectoryEntry> *dirTree_)
				: dirTree(dirTree_) {}
};

void	HyperTreeDraw::NewLineUpdateRowCol()
{
	if (curCol > htmlCol)
		htmlCol = curCol;
	htmlRow++;
	curCol = 0;
}

void	HyperTreeDraw::GenerateSymLinkHTML(HyperDocument &html, WINDOW *pad,
					   sptr<DirectoryEntry> &d,
					   const string &new_dir)
{
	bool	tagBegin = false;

	if (d->isLinkDestSkip == 0 
	    && d->isLinkDestUnreadable != 1) {

						// Write tags
		html.DrawAName(pad, new_dir);

		string s = "#";
		s += d->GetLinkDestStr();	// Anything after '#' can contain any
						// char, so quoting URL is not needed.
		html.DrawAHrefBegin(pad, s);

		html.DrawSetItalic(pad);

		tagBegin = true;
	}
	else if (d->isLinkDestSkip == 1) {
						// Write tags
		html.DrawAName(pad, new_dir);

		html.DrawACallBegin(pad, d->GetLinkDestStr());

		html.DrawSetItalic(pad);

		tagBegin = true;
	}

			    			// Write dir. name
	string s = d->GetNameStr();
	ReplaceControlChar(s);
	html.DrawString(pad, s);
	curCol += GetStringWidth(s);
			
			    			// Write graphical " -> "
	html.DrawChar(pad, ' ');
	html.DrawEntityChar(pad, EID_BOXH);
	html.DrawEntityChar(pad, EID_RARR);
	html.DrawChar(pad, ' ');
	curCol += 4;

			    			// Write linked dir.
	s = d->GetLinkStr();
	ReplaceControlChar(s);
	html.DrawString(pad, s);
	curCol += GetStringWidth(s);

	switch(d->isLinkDestSkip) {
		case 0:
			if(d->isLinkDestUnreadable) {
				html.DrawString(pad, html_unreadable);
				curCol += html_unreadable_width;
			}
			break;
		case 1:
			html.DrawString(pad, html_outside_tree);
			curCol += html_outside_tree_width;
			break;
		case 2:
			html.DrawString(pad, html_not_in_file);
			curCol += html_not_in_file_width;
			break;
		case 3:
			html.DrawString(pad, html_not_found);
			curCol += html_not_found_width;
			break;
	}
			
	if (tagBegin) {
			    			// End <A HREF=...> tag
		html.DrawClearItalic(pad);
		html.DrawAEnd(pad);
	}
}

void	HyperTreeDraw::GenerateDirHTML(HyperDocument &html, WINDOW *pad,
				       sptr<DirectoryEntry> &d,
				       const string &new_dir)
{
	if (d->isUnreadable != 1) {		// Exec permission
						// Write tags
		html.DrawAName(pad, new_dir);
		html.DrawACallBegin(pad, new_dir);
	}
//ofstream ofs("xx", ios::app);
//if (!pad)
//ofs << htmlRow << ' ' << htmlCol << ' ' << curCol << ' ' << d->GetNameStr() << '\n';
			    			// Write dir. name
	string s = d->GetNameStr();
	ReplaceControlChar(s);
	html.DrawString(pad, s);
	curCol += GetStringWidth(s);
			    		
	if (d->isUnreadable != 1) {		// Exec permission
			    			// Dir. can be accessed
			    				
			    			// End <A CALL=...> tag
		html.DrawAEnd(pad);
	}				    		
}

void	HyperTreeDraw::RecursiveGenerateHTML(HyperDocument &html, WINDOW *pad, 
					     sptr<DirectoryEntry> &d, 
					     size_t pos, const string &current_dir)
{
	if (d->isSkip) {
		if (d->isSkip == 3) {
			html.DrawString(pad, html_no_filelist);
			curCol += html_no_filelist_width;
			pos += html_no_filelist_width;
		}
		else if (d->isSkip == 2) {
			html.DrawString(pad, html_not_in_file);
			curCol += html_not_in_file_width;
			pos += html_not_in_file_width;
		}
		else {
			html.DrawString(pad, html_skipped);
			curCol += html_skipped_width;
			pos += html_skipped_width;
		}
	}

	if (d->isUnreadable) {
		html.DrawString(pad, html_unreadable);
		curCol += html_unreadable_width;
		pos += html_unreadable_width;
	}

	size_t	numSubDir = d->subDir.size();
	if (numSubDir == 0) {			// No more subdir
		return;				// Exit
	}

	size_t	curSubDir = 0;
	for (sptr_list<DirectoryEntry>::iterator iter = d->subDir.begin();
	     iter != d->subDir.end(); ++iter, ++curSubDir) {

		if (curSubDir == 0) {
			    			// First subdir entry
			    			// Draw a bar connected to 
			    			// parent dir
			html.DrawEntityChar(pad, EID_BOXH);
			curCol++;
			
			if (numSubDir == 1) {
			    			// Only one subdir present
			    			// Draw a bar
				html.DrawEntityChar(pad, EID_BOXH);
			    	curCol++;
			}
			else {
				html.DrawEntityChar(pad, EID_BOXHD);
			    	curCol++;
			    	
				barStack.push_back(pos+1);
			}
			html.DrawEntityChar(pad, EID_BOXH);
			curCol++;
		}
		else if (curSubDir == numSubDir-1) {
			    			// Last subdir entry
			html.DrawChar(pad, ' ');
			html.DrawEntityChar(pad, EID_BOXUR);
			html.DrawEntityChar(pad, EID_BOXH);
			curCol += 3;
			
			barStack.pop_back();
		}
		else {				// Middle entries
			html.DrawChar(pad, ' ');
			html.DrawEntityChar(pad, EID_BOXVR);
			html.DrawEntityChar(pad, EID_BOXH);
			curCol += 3;
		}
			    
		string new_dir = current_dir;
		if (new_dir.size() && new_dir[new_dir.size()-1] != '/')
			new_dir += '/';
		new_dir += (*iter)->GetNameStr();

			    			// For links
		if ((*iter)->IsSymLink()) {
			GenerateSymLinkHTML(html, pad, (*iter), new_dir);
		}
		else {
			GenerateDirHTML(html, pad, (*iter), new_dir);
			
				// Note: subdir. allowed even inside
				// unreadable or skipped dir
			
			    			// Scan inside this dir
		    	RecursiveGenerateHTML(html, pad, *iter,
					      pos+3+GetStringWidth((*iter)->GetNameStr()),
					      new_dir);
	    	}
		if (curSubDir != numSubDir-1) {	// Not the last one
						// New line
			html.DrawChar(pad, '\n');
			NewLineUpdateRowCol();
						// Add vertical bars
			for (size_t i = 0, j = 0; i < pos; i++) {
				if (j < barStack.size() && barStack[j] == i) {
					html.DrawEntityChar(pad, EID_BOXV);
					curCol++;
					
					j++;
				}
				else {
					html.DrawChar(pad, ' ');
					curCol++;
				}
			}
		}
	}
}

void	HyperTreeDraw::DrawBegin(HyperDocument &html, WINDOW *pad)
{
	htmlRow = 0;
	htmlCol = 0;
	curCol = 0;

	if (! pad) {
		;			// Nothing to do
	}
	else {
		html.DrawBegin(pad);

		time_t scanTime = GetScanTime();
		struct tm *scanTime2 = localtime(&scanTime);

		gtstream bufstr;
		gtout(bufstr, _("%$ %$   Last scan: %$"))
		     << progName << version
		     << my_strftime(_("%b %e  %H:%M"), scanTime2);
		html.DrawSetTitle(bufstr.str());
	}

}

void	HyperTreeDraw::DrawEnd(HyperDocument &html, WINDOW *pad)
{
	if (curCol != 0) {		// Not end with new line
		html.DrawChar(pad, '\n');
		NewLineUpdateRowCol();	// One more line needed
	}

	if (! pad) {
//ofstream ofs("xx", ios::app);
//if (!pad)
//ofs << htmlRow << ' ' << htmlCol << ' ' << '\n';
		html.DrawSetSize(htmlRow, htmlCol);
	}
	else {
		;			// Nothing to do
	}
}

void	HyperTreeDraw::Draw(HyperDocument &html, WINDOW *pad)
{
	DrawBegin(html, pad);

	bool	first = true;

	for (sptr_list<DirectoryEntry>::iterator iter = dirTree->begin();
	     iter != dirTree->end(); ++iter) {
	
		if (!first) {		// New line need ?
			html.DrawChar(pad, '\n');
			NewLineUpdateRowCol();
		}
		else
			first = false;

		string new_dir = (*iter)->GetNameStr();

		if ((*iter)->IsSymLink()) {		// Symbolic link
			GenerateSymLinkHTML(html, pad, (*iter), new_dir);
		}
		else {
			GenerateDirHTML(html, pad, (*iter), new_dir);

			curCol = GetStringWidth(new_dir);

				// Note: subdir. allowed even inside
				// unreadable or skipped dir
						// Recursion...
			RecursiveGenerateHTML(html, pad, *iter, curCol, new_dir);
		}
	}

	DrawEnd(html, pad);
}

/*************************************************************************
	Match Objects
*************************************************************************/

struct MatchInfo {
	string			name;
	sptr<DirectoryEntry>	dir;
	bool			is_curdir;
	unsigned		error;

	MatchInfo(const string &name_, sptr<DirectoryEntry> &dir_,
		  bool is_curdir_, unsigned error_ = 0) : 
		name(name_), dir(dir_), is_curdir(is_curdir_), error(error_)
	{
	}
};

struct MatchInfoIsCurDir {
	bool operator()(MatchInfo &m) const {
		return m.is_curdir;
	}
};

struct MatchInfoIsBookmarked {
	bool operator()(MatchInfo &m) const {
		return m.dir->bookmarked;
	}
};

struct MatchInfoIsNotBookmarkedNorCurDir {
	private:
		string curdir;
	public:
		MatchInfoIsNotBookmarkedNorCurDir(const string &s) : curdir(s) {
		}
		bool operator()(MatchInfo &m) const {
			return !m.dir->bookmarked && !m.is_curdir
				&& m.name != curdir;
		}
};

/*************************************************************************
	Find Match
*************************************************************************/

// The add_curdir feature isn't handled in fuzzy find because it
// doesn't work well with error sorting order.
void	FuzzyRecursiveFindMatchDir(list<MatchInfo> &match_list,
				   sptr<DirectoryEntry> &dir, 
				   const string &str,
				   regex_t *reg,
				   const string &current_dir)
{
	unsigned subStrLen;
	unsigned StrLen;
	unsigned cnt1, cnt2, TotCnt;
	unsigned TmpErr;
	int TmpVal;

	subStrLen = str.size();
	if (dir->IsDir() && dir->isUnreadable != 1) {
		string mstr = dir->GetNameStr();
		StrLen = mstr.size();
		for (size_t i = 0; i < StrLen; ++i)
			mstr[i] = tolower(mstr[i]);

		if (subStrLen < StrLen) {
			TotCnt = StrLen-subStrLen+1;
			unsigned err = static_cast<unsigned>(-1);
			for (cnt1 = 0; cnt1 < TotCnt; cnt1++) {
				TmpErr = 0;
				for (cnt2 = 0; cnt2 < subStrLen; cnt2++) {
					TmpVal = mstr[cnt1+cnt2-1]
						 - tolower(str[cnt2]);
					if (TmpVal < 0)
						TmpVal =- TmpVal;
					TmpErr += TmpVal * (subStrLen-cnt2+1);
				}
				TmpErr += cnt1*2;
				TmpErr += StrLen-subStrLen;
				for (cnt2 = 0; cnt2 < subStrLen; cnt2++) {
					if (mstr.find(tolower(str[cnt2])) == mstr.npos)
						TmpErr += 100;
				}

				if (TmpErr < err)
					err = TmpErr;
			}

			if (match_list.size()
			    < static_cast<size_t>(kcdConfig.cfgFuzzySize)) {
				list<MatchInfo>::iterator iter = match_list.begin();
				for ( ; iter != match_list.end(); ++iter) {
					if ((*iter).error > err)
						break;
				}
//FIXME: Add regex matching here
					// Insert before iter
				match_list.insert(iter,
						  MatchInfo(current_dir, dir, false, err));
			}
					// We have a fully populated list
			else if (match_list.back().error > err) {
				match_list.pop_back();

				list<MatchInfo>::iterator iter = match_list.begin();
				for ( ; iter != match_list.end(); ++iter) {
					if ((*iter).error > err)
						break;
				}
//FIXME: Add regex matching here
					// Insert before iter
				match_list.insert(iter,
						  MatchInfo(current_dir, dir, false, err));
			}
		}
	}

	for (sptr_list<DirectoryEntry>::iterator iter = dir->subDir.begin();
	     iter != dir->subDir.end(); ++iter) {      

						// For dirs
		if ((*iter)->IsDir()) {
			string new_dir = current_dir;
			if (new_dir[new_dir.size()-1] != '/')
				new_dir += '/';
			new_dir += (*iter)->GetNameStr();

			FuzzyRecursiveFindMatchDir(match_list, *iter,
						   str, reg, new_dir);
		}
	}
}

size_t	FuzzyFindMatchDir(list<MatchInfo> &match_list, const string &str,
			  regex_t *reg)
{
	match_list.clear();

	if (str.size() == 0)			// In case getcwd fails
		return 0;

	for (sptr_list<DirectoryEntry>::iterator iter = dirTree.begin();
	     iter!=dirTree.end(); ++iter) {
		string new_dir = (*iter)->GetNameStr();
		FuzzyRecursiveFindMatchDir(match_list, *iter, str, reg,
					   new_dir);
	}

	return match_list.size();
}

void	RecursiveFindMatchDir(list<MatchInfo> &match_list, sptr<DirectoryEntry> &dir, 
			      const string &str, regex_t *reg,
			      const string &current_dir,
			      bool add_curdir)
{
	if (dir->IsDir() && dir->isUnreadable != 1 &&
	    dir->GetNameStr().find(str) != string::npos) {	// Found

		if (MatchPathRegex(reg, current_dir))
			match_list.push_back(MatchInfo(current_dir, dir, false));
	}
	else if (add_curdir && saveCwd.size() && current_dir == saveCwd) {
							// Write current directory
							// as well
		match_list.push_back(MatchInfo(saveCwd, dir, true));
	}

	for (sptr_list<DirectoryEntry>::iterator iter = dir->subDir.begin();
	     iter != dir->subDir.end(); ++iter) {

			    			// For dirs
		if ((*iter)->IsDir()) {
			string new_dir = current_dir;
			if (new_dir[new_dir.size()-1] != '/')
				new_dir += '/';
			new_dir += (*iter)->GetNameStr();
			
			RecursiveFindMatchDir(match_list, *iter, str, reg,
					      new_dir, add_curdir);
		}
	}
}

size_t	FindMatchDir(list<MatchInfo> &match_list, const string &str,
		     regex_t *reg, bool add_curdir = false)
{
	match_list.clear();	
	
	if (str.size() == 0)		// In case getcwd fails
		return 0;
		
	for (sptr_list<DirectoryEntry>::iterator iter = dirTree.begin();
	     iter != dirTree.end(); ++iter) {
		string new_dir = (*iter)->GetNameStr();
		RecursiveFindMatchDir(match_list, *iter, str, reg, new_dir,
				      add_curdir);
	}
	
	if (kcdConfig.cfgFuzzySize &&
	    (match_list.size() == 0
	     || (match_list.size() == 1 && match_list.front().is_curdir == true)))
		return FuzzyFindMatchDir(match_list, str, reg);

	return match_list.size();
}

/*************************************************************************
	Generate HTML-like codes from the match list
*************************************************************************/

class HyperListDraw : public HyperDraw {
		size_t		numDigit, max_length;
		list<MatchInfo> &match_list;
	public:
		HyperListDraw(list<MatchInfo> &match_list_);
		void Draw(HyperDocument &html, WINDOW *pad);
};

HyperListDraw::HyperListDraw(list<MatchInfo> &match_list_) :
				match_list(match_list_)
{
	size_t	tempNumMatch = match_list.size();
	numDigit = 0;
	while (tempNumMatch) {
		tempNumMatch /= 10;
		numDigit++;
	}

	max_length = 0;
	for (list<MatchInfo>::iterator iter = match_list.begin();
	     iter != match_list.end(); iter++) {
		string s = iter->name;
		ReplaceControlChar(s);
		size_t size = GetStringWidth(s);
		if (size > max_length)
			max_length = size;
	}
}

void	HyperListDraw::Draw(HyperDocument &html, WINDOW *pad)
{

	if (!pad) {
				// Determine document size
//		gtstream bufstr;
//		gtout(bufstr, "echo %$ %$ > xx") << match_list.size() << 3+numDigit+max_length;
//		system(bufstr.str());

		html.DrawSetSize(match_list.size(), 3+numDigit+max_length);
	}
	else {
				// Draw document
		html.DrawBegin(pad);

		string	s = "";
		for (size_t i = 0; i < numDigit + 2; ++i)
			s += ' ';

		time_t scanTime = GetScanTime();
		struct tm *scanTime2 = localtime(&scanTime);

		gtstream bufstr;
		gtout(bufstr, _("%$ %$   Found: %$   Last scan: %$"))
		     << progName << version << match_list.size()
		     << my_strftime(_("%b %e  %H:%M"), scanTime2);
		html.DrawSetTitle(bufstr.str());

		size_t	index = 1;
		for (list<MatchInfo>::iterator iter = match_list.begin();
		     iter != match_list.end(); iter++) {
	
			if (index != 1) {		// New line need ?
				html.DrawChar(pad, '\n');
			}

					// Write tags
			html.DrawAName(pad, iter->name);
			html.DrawACallBegin(pad, iter->name);

			size_t index1 = index;
			for (size_t i = 0; i < numDigit && index1; ++i) {
				s[numDigit-i] = (index1 % 10) + '0';
				index1 /= 10;
			}
			html.DrawString(pad, s);

			string ss = iter->name;
			ReplaceControlChar(ss);
			html.DrawString(pad, ss);

			html.DrawChar(pad, ' ');
		
			for (size_t i = 0; i < max_length-GetStringWidth(iter->name); i++) {
				html.DrawChar(pad, ' ');
			}

		    			// End <A CALL=...> tag
			html.DrawAEnd(pad);

			index++;
		}
	}
}

/*************************************************************************
	Add current working directory to dirTree
	Size effect: chdir() called if dir not found
*************************************************************************/

void	ExtendTree(sptr<DirectoryEntry> &d, const string &str)
{
	string::size_type delim;
	
	if ((delim = str.find('/')) != string::npos) {	// Contains subdir
							// Need to recurse
	
						// Split str into 2 string
		string str_first(str, 0, delim);
		string str_last(str, delim+1);

		sptr<DirectoryEntry> d2(new DirectoryEntry(str_first));
		d2->isSkip = 2;			// Mark as [*]

		d->subDir.push_back(d2);	// Add our own

		if (kcdConfig.cfgSortTree)
			d->subDir.sort();

		if (str_last.size()) {
			k_chdir(str_first);	// Require recursion
			ExtendTree(d2, str_last);
		}
	}
	else {					// No more subdir
		sptr<DirectoryEntry> d2(new DirectoryEntry(str));
		d2->isSkip = 2;			// Mark as [*]

		d->subDir.push_back(d2);	// Add our own

		if (kcdConfig.cfgSortTree)
			d->subDir.sort();
	}
}

// change - when true, check honor_cfg if change is required
// honor_cfg - when true, don't modify tree if directory is in the list of SkipDir

FindDirInfo *RecursiveFindDir(sptr<DirectoryEntry> &d, const string &current_dir, 
			      const string &str, bool honor_cfg, bool change)
{
			// Check of subdir. of d
	for (sptr_list<DirectoryEntry>::iterator iter = d->subDir.begin();
	     iter != d->subDir.end(); ++iter) {
	     
		if ((*iter)->IsDir()) {		// Make sure it's not a
						// symbolic link

			string new_dir = current_dir;

						// Last char is not `/'
			if (new_dir[new_dir.size()-1] != '/') {
				new_dir += '/';		// Add one
			}
			new_dir += (*iter)->GetNameStr();	// Add dir. name

					// Name to long, impossible to match
					// against this dir.
			if (new_dir.size() > str.size())
				continue;	// Skip this
					
					// Same length
					// Match possible
			if (new_dir.size() == str.size()) {

				if (str != new_dir)
					continue;	// Not matched
							// Skip this
				return new FindDirInfo((*iter), str);	// Matched
			}

				// Now new_dir.size() < str.size()

				// Can match first part of dir.
			if (new_dir.size() != 1 && str[new_dir.size()] != '/')
				continue;


					// We're left with this case
					// str     -> /xxxx/yyyy
					// new_dir -> /xxxx
					
					// This case does not happen here
					// (new_dir is not top-level)
					// str     -> /xxxx
					// new_dir -> /

			if (str.substr(0, new_dir.size()) == new_dir) {

				// Note: subdir. allowed even inside
				// unreadable or skipped dir
					
						// Recursion...
				return RecursiveFindDir(*iter, new_dir, 
							str, honor_cfg, change);
			}
		}
	}

	if (change) {			// No match, new one added
					// if change == true

		if (honor_cfg && FindSkipDir(current_dir))
			return 0;

//		if (!d->isSkipped && kcdConfig.cfgAutoScan) {
//		}
		

		// No match, need to branch the current dir from here

			// Two cases:
			// Need to check whether parent dir. is root 
			// or not
		if (current_dir.size() == 1) {		// Branching from root dir

			k_chdir("/");			// Required to obtain permission
							// in ExtendTree(...)
		
							// Ignore the first `/'
			ExtendTree(d, str.substr(1));
		}
		else {			// str[current_dir.size()] == `/'
					// Branching from other dir
				
			k_chdir(current_dir);		// Required to obtain permission
							// in ExtendTree(...)

							// Ignore the `/'
			ExtendTree(d, str.substr(current_dir.size()+1));
		}
		return new FindDirInfo(d, current_dir);
	}
	else
		return 0;
}

FindDirInfo *FindDir(sptr_list<DirectoryEntry> *tree, const string &str,
		     bool honor_cfg, bool change_top, bool change_sub)
{
	if (str.size() == 0)
		return 0;
						// Look for top-level directories

	for (sptr_list<DirectoryEntry>::iterator iter = tree->begin();
	     iter != tree->end(); ++iter) {
	
		if ((*iter)->IsDir()) {		// Make sure it's not a
						// symbolic link

			string new_dir = (*iter)->GetNameStr();
				
					// Name to long, impossible to match
					// against this dir.
			if (new_dir.size() > str.size())
				continue;	// Skip this
					
					// Same length
					// Match possible
			if (new_dir.size() == str.size()) {

				if (str != new_dir)
					continue;	// Not matched
							// Skip this
				return new FindDirInfo(*iter, str);	// Matched
			}

				// Now new_dir.size() < str.size()

				// Can match first part of dir.
			if (new_dir.size() != 1 && str[new_dir.size()] != '/')
				continue;

				// We're left with two cases
				// str     -> /xxxx		(first case)
				// new_dir -> /
				// str     -> /xxxx/yyyy	(second case)
				// new_dir -> /xxxx

			if (str.substr(0, new_dir.size()) == new_dir) {

				// Note: subdir. allowed even inside
				// unreadable or skipped dir
					
						// Recursion...
				return RecursiveFindDir(*iter, new_dir, 
							str, honor_cfg, change_sub);
			}
		}
	}

				// Cannot find current dir. in top-level dir
	if (change_top) {
		if (honor_cfg && ! FindStartDir(str))
			return 0;

		sptr<DirectoryEntry> d(new DirectoryEntry(str));
		d->isSkip = 2;		// Mark as [*]
		tree->push_back(d);	// Add our own top-level entry

		if (kcdConfig.cfgSortTree)
			tree->sort();

		return new FindDirInfo(d, str);
	}
	else
		return 0;
}

FindDirInfo *FindDir(sptr_list<DirectoryEntry> *tree, const string &str,
		     bool honor_cfg, bool change)
{
	return FindDir(tree, str, honor_cfg, change, change);
}

/*************************************************************************
	Screen setup
*************************************************************************/

void FullScreenMode(HyperDraw *drawObj, const string &hyperInitPos)
{
	HyperDocument		*html = new HyperDocument(drawObj);

	{
						// Display
		NCurses::Init();

		try {

			if (COLS < 50 || LINES < 10) {
				throw ErrorScreenTooSmall();
			}
			else {
				init_entity_table();

				khGeometryManager	geoMan(modeHVScroll);
				khScreenManager		scrnMan(geoMan);

				khStatusWindowWithFind	findBar(scrnMan, idStatWindow, NULL);
				khURLWindowWithFind	urlBar(scrnMan, idURLWindow);
				khHScrollBar		hscroll(scrnMan, idHScrollBar);
				khVScrollBar		vscroll(scrnMan, idVScrollBar);
				khScrollBox		cursorRest(scrnMan, idScrollBox);
	
				urlBar.SetTitle(" ");		// Empty title
				scrnMan.SetCursor(&cursorRest);
				scrnMan.RequestUpdateAndRestCursor();

							// Use pre-computed 
							// document size
				khHyperWindowWithFind	hyper(scrnMan, idHyperWindow,
							      *html, findBar, urlBar,
							      hscroll, vscroll, hyperInitPos,
							      1, htmlRow, htmlCol);

				findBar.SetFindWindow(&hyper);
				hscroll.SetWin(&hyper);	// Handle scroll through mouse
				vscroll.SetWin(&hyper);
				
							// Set cfgSpaceSelect flag
				khStatusWindowWithFind::spaceSelect = kcdConfig.cfgSpaceSelect;

							// Send all keyboard input
							// to hyper
				scrnMan.SetFocus(&hyper);
			
							// Paint screen
				scrnMan.RequestUpdateAndRestCursor();
				hyper.KeyboardLoop();
			}
		}
		catch (...) {
			move(LINES-1, 0);
			printf("\n\n");		// Avoid printing error
						// message on top of drawn
						// screen
			NCurses::End();
			throw;
		}
		NCurses::End();
	}
}

/*************************************************************************
	MountDir handling
*************************************************************************/

void	ProcessMountDir(bool search, const string &root)
{
	for (MountDirList::iterator iter = kcdConfig.cfgMountDir->begin();
	     iter != kcdConfig.cfgMountDir->end(); ++iter) {
		FindDirInfo *f = FindDir(&dirTree, (*iter)->dir, false);
		if (f) {
			f->dir_ptr->subDir.clear();

			if (root.size()) {
				if ((*iter)->dir.size() < root.size()
				    || (*iter)->dir.substr(0, root.size())
					!= root)
					continue;
			}

			f->dir_ptr->isSkip = 0;

				// This make sure the MountDir
				// directory is actually scanned
				// and not skipped
			MountDirList* m = kcdConfig.cfgMountDir;
			kcdConfig.cfgMountDir = new MountDirList;

			const MountActionOrderType &mountAction = (*iter)->mountAction;

			for (MountActionOrderType::const_iterator iter2 = mountAction.begin();
			     iter2 != mountAction.end(); ++iter2) {

				bool result = false;
				switch (*iter2) {
					case mountActionSkip:
						result = true;
						break;
					case mountActionAll:
						MountDirScanDir((*iter)->dir);
						result = true;
						break;
					case mountActionTree:
						if (search)
							;
						else {
							MountDirScanDir((*iter)->dir);
							result = true;
						}
						break;
					case mountActionFileList:
						break;
				}

				if (result)
					break;
			}

			delete kcdConfig.cfgMountDir;
			kcdConfig.cfgMountDir = m;
		}
		delete f;
	}
}

/*************************************************************************
	Display tree functions
*************************************************************************/

void AddDisplayTree(const string &root, sptr_list<DirectoryEntry> *tree)
{
	string root_abs = RelativeToAbsolutePath(root, saveCwd);
	root_abs = ExpandDirectory(root_abs, false);

						// Allocate from heap since
						// its lifetime is longer than
						// this function
	DirectoryEntry *entry = new DirectoryEntry(root_abs);
	FindDirInfo *find = FindDir(&dirTree, root_abs, false, false);

	if (find)
		entry->subDir = find->dir_ptr->subDir;
	else
		entry->isSkip = 2;		// Not in file

	tree->push_back(entry);
}

HyperDraw *GetDisplayTree(const string &root)
{
							// Allocate from heap since
							// its lifetime is longer than
							// this function
	sptr_list<DirectoryEntry> *tree = new sptr_list<DirectoryEntry>;

	if (root.size()) {
		AddDisplayTree(root, tree);
							// Add current directory after
							// adding the entry above
		if (saveCwd.size())			// Current directory is available
			FindDir(tree, saveCwd, false);
	}
	else if (saveCwd.size()) {
							// Allocate from heap since
							// its lifetime is longer than
							// this function
		DirectoryEntry *entry = new DirectoryEntry(saveCwd);
		FindDirInfo *find = FindDir(&dirTree, saveCwd, false, false);
		if (find)
			entry->subDir = find->dir_ptr->subDir;
		else
			entry->isSkip = 2;		// Not in file
		tree->push_back(entry);
	}
	else
		throw ErrorGenericCommandLine(_("cannot determine current directory"));

	HyperDraw *drawObj;
	if (tree->size()) {
		RecursiveResolveSymLink(tree, *tree, string(""));
		drawObj = new HyperTreeDraw(tree);
	}
	else						// Display entire tree
		drawObj = new HyperTreeDraw(&dirTree);
	return drawObj;
}

void	DisplayTree(const string &root, bool use_mount_dir, bool rescan)
{
	string str;
	if (root.size() == 0) {
		if (saveCwd.size())
			str = saveCwd;
		else
			throw ErrorGenericCommandLine(_("cannot determine current directory"));
	}
	else {
		str = RelativeToAbsolutePath(root, saveCwd);
		str = ExpandDirectory(str, false);
	}

	FindDirInfo *find = FindDir(&dirTree, str, false, false);
	if (!find)
		rescan = true;

	if (rescan)
		ScanDirAndDisplay(str, false);

	if (use_mount_dir)
		ProcessMountDir(false, root);

	GenerateHTMLStrings();
	HyperDraw *drawObj = GetDisplayTree(str);
	FullScreenMode(drawObj, "");
}

void	DisplayFullTree(bool use_mount_dir, bool rescan)
{
	if (rescan)
		ScanDir(false);

	if (use_mount_dir)
		ProcessMountDir(false, "");

	GenerateHTMLStrings();
	HyperDraw *drawObj = new HyperTreeDraw(&dirTree);
	FullScreenMode(drawObj, "");
}

/*************************************************************************
	Change directory
*************************************************************************/

void	GetBookmarks(list<MatchInfo> &match_list, const string &str,
		     regex_t *reg);

void	CheckCDString(const string &str)
{
	if (str.size()) {
#ifdef	CLIB_HAVE_REGEX
		if (str[0] == '/')
			throw ErrorGenericCommandLine(_("invalid search string"));
		if (str[str.size()-1] == '/')
			throw ErrorGenericCommandLine(_("invalid search string"));
		if (str.find("///") != string::npos)
			throw ErrorGenericCommandLine(_("invalid search string"));
#else
		if (str.find('/'))
			throw ErrorGenericCommandLine(_("regular expression not supported"));
#endif
	}
}

// FIXME: init_pos and adjust look redundant

void	DirListCD(list<MatchInfo> &match_list, const string &init_pos,
		  const string &str, size_t adjust, size_t match_no)
{
	list<MatchInfo>::iterator iter;

	if (match_no) {
							// Current directory is not
							// need here.  Remove it.
		match_list.remove_if(MatchInfoIsCurDir());
		adjust = 0;

		iter = match_list.begin();
							// match_no starts at 1
		for (size_t i = 1; i < match_no; ++i, ++iter)
			;
	}
	else {
		list<MatchInfo>::iterator current;

		for (iter = match_list.begin();
		     iter != match_list.end(); ++iter) {
			if ((*iter).is_curdir || (*iter).name == init_pos) {
				current = iter;
				break;
			}
		}
		if (iter == match_list.end()) {
							// getcwd fails

							// This checks every entry
							// in the list

			iter = match_list.begin();	// Arbitrary pick the first
							// one
			current = iter;			// Set terminate condition
							// for the while loop below
		}
		else {
							// This checks every entry
							// except the current dir

			iter = current;
			next_loop(match_list, iter);	// Start from the next one
		}

							// Loop until we find a
							// directory that actually
							// exists
		while (k_access((*iter).name, F_OK)) {
			cout << flush;			// Avoid out-of-order display
			gtout(cerr, _("%$: skipping %$"
				      " - directory no longer exist\n"))
			     << progName << (*iter).name;

			next_loop(match_list, iter);
			if (iter == current) {
							// Can reach here when we
							// have matches but all
							// no longer exist
				if (!(*current).is_curdir) {
					cout << _("no directory change\n");
					return;
				}
				else
					throw ErrorGenericFile(_("cannot find directory "
								 "matching the string %$"),
							       str);
			}
		}
	}

	if ((*iter).is_curdir || (*iter).name == init_pos) {
		cout << _("no directory change\n");
		return;
	}

	FILE	*file = k_fopen(dirFile, "wb");
	if (file == NULL) {			// Cannot create or truncate
						// ~/.kcd.newdir
		throw ErrorGenericFile(_("cannot create file %$"),
				       dirFile);
	}
	
							// Output desired dir
	k_fputs((*iter).name, file);
	fclose(file);

	bool show_new = false;
	switch (kcdConfig.cfgShowNewDir) {
		case showNewDirNo:
			break;
		case showNewDirYes:
			show_new = true;
			break;
		case showNewDirMulti:
							// More than one matched dir
			if (match_list.size() > adjust+1)
				show_new = true;
			break;
	}
	if (show_new)
		cout << _("change to ") << (*iter).name << '\n';
}

void	CD(const string &linktext, bool entire_tree, 
	   bool use_mount_dir, size_t match_no)
{
	CheckCDString(linktext);

	LoadDirFile(true);

	if (use_mount_dir)
		ProcessMountDir(linktext.size(), "");

	HyperDraw *drawObj = 0;
	list<MatchInfo>	match_list;			// Must match lifetime
							// of drawObj!

	if (saveCwd.size())				// Current directory is available
		FindDir(&dirTree, saveCwd, false);

	string	hyperInitPos;
	
	GenerateHTMLStrings();

	if (linktext.size()) {				// kcd DIR
		string	str = MakeString(linktext);
		regex_t *reg = MakePathRegex(linktext);

		ImportBookmarks("");

		if (entire_tree)			// Ignore bookmarks
							// We add current
							// directory into the mix
							// to aid cycling through
							// all match directory
							// in the command line
			FindMatchDir(match_list, str, reg, true);

		else {
			GetBookmarks(match_list, str, reg);
			if (match_list.size()) {
				if (match_list.size() > 1 && saveCwd.size()) {
					list<MatchInfo>::iterator iter;
					for (iter = match_list.begin();
					     iter != match_list.end(); ++iter) {
						if ((*iter).name == saveCwd)
							break;
					}
					if (iter == match_list.end()) {
							// Add current directory to aid jumping
						FindDirInfo *f = FindDir(&dirTree, saveCwd, true, false);
						match_list.push_front(MatchInfo(saveCwd, f->dir_ptr, true));
					}
				}
			}
			else {
				FindMatchDir(match_list, str, reg, true);
			}
		}

		size_t	adjust = 0;			// In case getcwd fails
		for (list<MatchInfo>::iterator iter = match_list.begin();
		     iter != match_list.end(); ++iter) {
			if ((*iter).is_curdir) {
				adjust = 1;
				break;
			}
		}
							// +1 because of current
							// directory in match_list
		if ((match_list.size() > static_cast<size_t>(kcdConfig.cfgShowListThreshold)
					 +adjust
		     && match_no == 0)
		    || match_no+adjust > match_list.size()) {

							// Show whole list

							// Current directory is not
							// need here.  Remove it.
			match_list.remove_if(MatchInfoIsCurDir());
			drawObj = new HyperListDraw(match_list);
			
			hyperInitPos = "";
			
		}
		else {					// Jump immediately
			DirListCD(match_list, saveCwd, linktext, adjust, match_no);
			return;
		}
	}
	else {						// Browse dir tree
		if (kcdConfig.cfgDefaultTree->size()) {
							// Allocate from heap since
							// its lifetime is longer than
							// this function
			sptr_list<DirectoryEntry> *tree = new sptr_list<DirectoryEntry>;

			for (DirList::iterator iter = kcdConfig.cfgDefaultTree->begin();
			     iter != kcdConfig.cfgDefaultTree->end(); ++iter) {
				AddDisplayTree((*iter)->dir, tree);
			}
							// Add current directory after
							// adding the entry above
			if (saveCwd.size())		// Current directory is available
				FindDir(tree, saveCwd, false);
			drawObj = new HyperTreeDraw(tree);
		}
		else
			drawObj = new HyperTreeDraw(&dirTree);
		hyperInitPos = saveCwd;
	}

#ifdef DUMP_HTML
	{
		Buffer	buffer;
		int handle = open("/home/lerdsuwa/devel/kcd/debug", O_RDWR|O_CREAT, 0700);
		write(handle, buffer.GetPtr(), strlen(buffer.GetPtr()));
		close(handle);
	}
#endif

	FullScreenMode(drawObj, hyperInitPos);
}

/*************************************************************************
	Bookmark helper functions
*************************************************************************/

list<string> bookmark_list;

void	AddBookmark(const string &str, bool verify = true)
{
	string s = ExpandDirectory(str, false);
	if (verify) {
		FindDirInfo *f = FindDir(&dirTree, s, true, false);
		if (f) {
							// Prevent duplicate entries
			if (find(bookmark_list.begin(), bookmark_list.end(), s)
			    == bookmark_list.end())
				bookmark_list.push_back(s);
			else {
				cout << flush;		// Avoid out-of-order display
				gtout(cerr, _("%$: directory %$ already bookmarked\n"))
				     << progName << s;
			}
		}
		else {
			cout << flush;			// Avoid out-of-order display
			gtout(cerr, _("%$: cannot find %$ in saved directory tree\n"))
			     << progName << s;
		}
		delete f;
	}
	else
		bookmark_list.push_back(s);
}

void	RemoveBookmark(const string &str, bool verify = true)
{
	string s = ExpandDirectory(str, false);
	if (verify) {
		FindDirInfo *f = FindDir(&dirTree, s, true, false);
		if (!f) {
			cout << flush;			// Avoid out-of-order display
			gtout(cerr, _("%$: cannot find %$ in saved directory tree\n"
				      "        Directory ignored\n"))
			     << progName << s;
		}
		delete f;
	}
	remove(bookmark_list.begin(), bookmark_list.end(), s);
}

void	GetBookmarks(list<MatchInfo> &match_list, const string &str,
		     regex_t *reg)
{
	for (list<string>::const_iterator iter = bookmark_list.begin();
	     iter != bookmark_list.end(); iter++) {

		if (str.size()) {
			if (MatchStr(str, *iter)
			    && MatchPathRegex(reg, *iter)) {
				FindDirInfo *f = FindDir(&dirTree, *iter, true, false);
				if (f)
					match_list.push_back(MatchInfo(*iter, f->dir_ptr, false));
				delete f;
			}
		}
		else {
			FindDirInfo *f = FindDir(&dirTree, *iter, true, false);
			if (f)
				match_list.push_back(MatchInfo(*iter, f->dir_ptr, false));
			delete f;
		}
	}
}

void	OutputBookmarks(const string &filename)
{
	const char *	s;
	if (filename.size())
		s = filename.c_str();
	else
		s = bookmarkFile.c_str();

	ofstream ofs(s);
	if (!ofs) {
		throw ErrorGenericFile(_("cannot create file %$"), s);
	}

	for (list<string>::const_iterator iter = bookmark_list.begin();
	     iter != bookmark_list.end(); iter++) {
		ofs << QuoteShellChars(*iter) << '\n';
		if (!ofs)
			throw ErrorGenericFile(_("error writing file %$"), s);
	}
}

/*************************************************************************
	Bookmark functions
*************************************************************************/

void	ImportBookmarks(const string &filename)
{
	bookmark_list.clear();

	const char *	s;
	if (filename.size())
		s = filename.c_str();
	else
		s = bookmarkFile.c_str();

	ifstream ifs(s);
	if (!ifs)		// Don't throw exception here
		return;

	for ( ; ; ) {
		string	buffer;
		getline(ifs, buffer);
		if (ifs.eof())
			break;
		if (!ifs)
			throw ErrorGenericFile(_("error reading file %$"), s);
		try {
			AddBookmark(UnquoteShellChars(buffer), false);
		}
		catch (ErrorRange &) {
			// Continue next line without abort
			// FIXME: Detect UnquoteShellChars errors
		}
	}
}

void	ExportBookmarks(const string &filename)
{
	ImportBookmarks("");
	OutputBookmarks(filename);
}

void	ShowBookmarks(const string &linktext)
{
	LoadDirFile(true);
	ImportBookmarks("");

	list<MatchInfo>	match_list;

	if (linktext.size()) {				// kcd DIR
		string	str = MakeString(linktext);
		regex_t *reg = MakePathRegex(linktext);
		GetBookmarks(match_list, str, reg);
	}
	else
		GetBookmarks(match_list, linktext, 0);

	if (!match_list.size()) {
		cout << flush;			// Avoid out-of-order display
		gtout(cerr, _("%$: no bookmarks found\n"))
		     << progName;
		return;
	}

	GenerateHTMLStrings();

	HyperDraw *drawObj = new HyperListDraw(match_list);
			
	FullScreenMode(drawObj, "");
}

void	AddBookmarks(int argc, char *argv[], int argcBegin)
{
	LoadDirFile(true);
	ImportBookmarks("");

	if (argcBegin == argc) {
		if (saveCwd.size())
			AddBookmark(saveCwd, true);
		else {
			cout << flush;			// Avoid out-of-order display
			gtout(cerr, _("%$: cannot determine current directory\n"
				      "        Directory ignored"))
			     << progName;
			return;
		}
	}
	else {
		for (int i = argcBegin; i < argc; ++i) {
			if (!argv[i][0])
				continue;

			if (argv[i][0] != '/' && argv[i][0] != '~'
			    && !saveCwd.size()) {
				cout << flush;			// Avoid out-of-order display
				gtout(cerr, _("%$: cannot determine current directory for path %$\n"
	 				      "        Directory ignored"))
				     << progName << argv[i];
			}
			else {
				string dir = RelativeToAbsolutePath(argv[i], saveCwd);
				AddBookmark(dir, true);
			}
		}
	}

	OutputBookmarks("");
}

void	RemoveBookmarks(int argc, char *argv[], int argcBegin)
{
	LoadDirFile(true);
	ImportBookmarks("");

	if (argcBegin == argc) {
		if (saveCwd.size())
			RemoveBookmark(saveCwd, true);
		else {
			cout << flush;			// Avoid out-of-order display
			gtout(cerr, _("%$: cannot determine current directory\n"
				      "        Directory ignored"))
			     << progName;
			return;
		}
	}
	else {
		for (int i = argcBegin; i < argc; ++i) {
			if (!argv[i][0])
				continue;

			if (argv[i][0] != '/' && argv[i][0] != '~'
			    && !saveCwd.size()) {
				cout << flush;			// Avoid out-of-order display
				gtout(cerr, _("%$: cannot determine current directory for path %$\n"
					      "        Directory ignored"))
				     << progName << argv[i];
			}
			else {
				string dir = RelativeToAbsolutePath(argv[i], saveCwd);
				AddBookmark(dir, true);
			}
		}
	}

	OutputBookmarks("");
}
