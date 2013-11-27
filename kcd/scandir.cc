/*
	scandir.cc	Scan Directory Functions
	Copyright (c) 2000,2001,2003,2005,2007 Kriang Lerdsuwanakij
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

#include "scandir.h"

#include CXX__HEADER_iomanip
#include CXX__HEADER_fstream
#include CXX__HEADER_algorithm
#include CXX__HEADER_cerrno

#include "conffile.h"
#include "gentree.h"
#include "cxxlib.h"
#include "strmisc.h"
#include "cstrlib.h"
#include "dirtree.h"

/*************************************************************************
	Log scan results to command line output
*************************************************************************/

void	CommandLineScanLog::PrintTime(long secdiff, const string &mode)
{
	struct tm elapse_time;
	elapse_time.tm_sec = secdiff % 60;
	int	total_min = secdiff / 60;
	elapse_time.tm_min = total_min % 60;
	elapse_time.tm_hour = total_min / 60;

	if (elapse_time.tm_hour)		// hour > 0
		gtout(cout, _("Time spent: %$ (%$)\n"))
		     << my_strftime(_("%H:%M:%S"), &elapse_time)
		     << mode;
	else
		gtout(cout, _("Time spent: %$ (%$)\n"))
		     << my_strftime(_("%M:%S"), &elapse_time)
		     << mode;
}

void	CommandLineScanLog::PrintChdirError(const string &dir, const string &str)
{
	cout << flush;		// Avoid out-of-order display
	gtout(cerr, _(": cannot change to directory %$ - %$\n"))
	     << dir << str;
}

void	ScanCommandScanLog::PrintStartDirNotDirectory(const string &str)
{
	cout << flush;		// Avoid out-of-order display
	gtout(cerr, _("%$: %$ specified in `StartDir\' command is not"
		      " a directory.  Command ignored\n"))
	     << progName << str;
}

void	ScanCommandScanLog::PrintStartDirNotFound(const string &str)
{
	cout << flush;		// Avoid out-of-order display
	gtout(cerr, _("%$: cannot find directory %$"
		      " as specified in `StartDir\' command.  Command ignored\n"))
	     << progName << str;
}

void	TreeCommandScanLog::PrintStartDirNotDirectory(const string &str)
{
	cout << flush;		// Avoid out-of-order display
	gtout(cerr, _("%$: %$ is not a directory.  Command ignored\n"))
	     << progName << str;
}

void	TreeCommandScanLog::PrintStartDirNotFound(const string &str)
{
	cout << flush;		// Avoid out-of-order display
	gtout(cerr, _("%$: cannot find directory %$.  Command ignored\n"))
	     << progName << str;
}

void	MountCommandScanLog::PrintStartDirNotDirectory(const string &str)
{
	cout << flush;		// Avoid out-of-order display
	gtout(cerr, _("%$: %$ specified in `MountDir\' command is not"
		      " a directory.  Command ignored\n"))
	     << progName << str;
}

void	MountCommandScanLog::PrintStartDirNotFound(const string &str)
{
	cout << flush;		// Avoid out-of-order display
	gtout(cerr, _("%$: cannot find directory %$"
		      " as specified in `MountDir\' command.  Command ignored\n"))
	     << progName << str;
}


void	HandleChdirError(ScanLog &scan_log, const string &dir, int e)
{
	if (e == EACCES)
		return;
	scan_log.PrintChdirError(dir, strerror(e));
}

/*************************************************************************
	Variables
*************************************************************************/

int	dirUpdated = 0;			// Equals 1 if recan in this session

void	ExpandSymLink(ScanLog &scan_log)
{
	scan_log.PrintWork(_("Resolving symbolic links"));
	RecursiveResolveSymLink(&dirTree, dirTree, string(""));
}

/*************************************************************************
	Full directory rescan
*************************************************************************/

void	RecursiveFullScanDir(ScanLog &scan_log, sptr<DirectoryEntry> &d, 
			     const string &current_dir)
{
	if (FindSkipDir(current_dir)
	    || FindMountDir(current_dir)) {	// Match skipped dir. 
		d->isSkip = 1;
		return;
	}

	DIR *dir = opendir(".");
	if (dir == NULL) {		// No read access
		d->isUnreadable = 2;
		return;			// Skip scanning this dir
	}

	try {
		struct dirent 	*entry;
		while((entry = readdir(dir)) != NULL) {
						// Skip "." and ".." dir 
			if (strcmp(entry->d_name, ".") != 0 && 
			    strcmp(entry->d_name, "..") != 0) {

						// Do not delete d2 if it is added to
						// dirTree!
				sptr<DirectoryEntry> d2(new DirectoryEntry(entry));

				if (d2->IsDir()) {

					d->subDir.push_back(d2);
				
					string new_dir = current_dir;
					if (new_dir[new_dir.size()-1] != '/')
						new_dir += '/';
					new_dir += entry->d_name;

						// Display current progress 
					scan_log.PrintDirectory(new_dir);
				
					if (!(d2->IsSymLink())) {
								// For dirs
						int ret = k_chdir(entry->d_name);
						if (ret == 0) {
								// Dir. can be accesed
			    		

								// Scan inside this dir
					    		RecursiveFullScanDir(scan_log, d2, new_dir);
				    			
					    			// Back to current dir.
					    		k_chdir("..");
					    	}
					    	else {	// No exec. permission
							int e = errno;
							HandleChdirError(scan_log, entry->d_name, e);

					    		d2->isUnreadable = 1;
					    	}
			    		}
				}
					// Else it is not a dir. entry.
					// It is automatically deleted.

			}
		}
	}
	catch (...) {
		closedir(dir);
		throw;
	}
	closedir(dir);
	
	if (kcdConfig.cfgSortTree)
		d->subDir.sort();
}

void	FullScanFromDir(ScanLog &scan_log, const string &dir, bool set_flags = false)
{
	if (k_access (dir, F_OK))
		scan_log.PrintStartDirNotFound(dir);
	else if (k_chdir(dir))
		scan_log.PrintStartDirNotDirectory(dir);
	else {
		sptr<DirectoryEntry> d(new DirectoryEntry(dir));
		dirTree.push_back(d);

		if (set_flags)		// Set flags if requested
			d->flags = 1;

		scan_log.PrintDirectory(dir);

		if (!d->IsSymLink()) {

						// Recursion...
			RecursiveFullScanDir(scan_log, d, dir);
		}
	}
}


void	FullScanDir(ScanLog &scan_log)
{
	time_t t1, t2;			// Get current time 
	time(&t1);
	SetScanTime(t1);

	scan_log.SetQuiet(kcdConfig.cfgQuietFull);

	dirTree.clear();

	if (kcdConfig.cfgStartDir->empty()) {
		FullScanFromDir(scan_log, "/");
	}
	else {
		for (DirList::iterator iter = kcdConfig.cfgStartDir->begin();
		     iter != kcdConfig.cfgStartDir->end(); ++iter) {

			try {
				FullScanFromDir(scan_log, UnquoteShellChars((*iter)->dir));
			}
			catch (ErrorRange &) {		// Ignore quoting error
			}
		}
		
		if (kcdConfig.cfgSortTree)
			dirTree.sort();
	}

	ExpandSymLink(scan_log);
	WriteDirFile();

	time(&t2);
	
	scan_log.PrintTime(t2-t1, _("full scan mode"));

	dirUpdated = 1;
}

/*************************************************************************
	Smart directory rescan
*************************************************************************/

int	hit = 0, miss = 0;

struct	IsDirFlagCleared {
	bool operator()(sptr<DirectoryEntry> &d) const {
		return d->flags == 0;
	}
};

struct	SetDirFlag {
	char	flags;
	SetDirFlag(char flags_) : flags(flags_) {}
	void operator()(sptr<DirectoryEntry> &d) {
		d->flags = flags;
	}
};

struct	AssertDirFlag {
	char	flags;
	AssertDirFlag(char flags_) : flags(flags_) {}
	void operator()(sptr<DirectoryEntry> &d) {
		if (d->flags != flags) {
			throw ErrorGeneric("Invalid directory tree state, program aborted");
		}
	}
};

void	RecursiveSmartScanDir(ScanLog &scan_log, sptr<DirectoryEntry> &d, 
			      const string &current_dir)
{
	int	ret;
	int	fullDirRead = 0;

	if (FindSkipDir(current_dir)
	    || FindMountDir(current_dir)) {	// Match skipped dir. 
		d->isSkip = 1;
		d->subDir.clear();
		return;
	}

	if (d->isSkip) {		// No longer skipped
		fullDirRead = 1;
		d->isSkip = 0;		// Clear skip flag

		DirectoryEntry d2(".");	// Update info
		d->UpdateEntry(&d2);
	}
	else {
					// Assumes "." is dir. files
					// This will read new stat
					// from disk
		DirectoryEntry d2(".");

						// Check date
		if (Max(d2.GetModTime(), d2.GetChangeTime()) 
		    > Max(d->GetModTime(), d->GetChangeTime())) {
			d->UpdateEntry(&d2);	// New info
			fullDirRead = 1;
		}
	}
	
	if (!fullDirRead) {		// Fast directory scanning

		if (d->isUnreadable) {
			d->subDir.clear();
			return;			// Unreadable
		}

		for (sptr_list<DirectoryEntry>::iterator iter = d->subDir.begin();
		     iter != d->subDir.end(); ++iter) {

			string new_dir = current_dir;
			if (new_dir[new_dir.size()-1] != '/')
				new_dir += '/';
		    	new_dir += (*iter)->GetNameStr();
					// Display current progress 
			scan_log.PrintDirectory(new_dir);

			if (!((*iter)->IsSymLink())) {
				    			// For dirs 
				ret = k_chdir((*iter)->GetNameStr());
				if (ret == 0) {
						// Dir. can be accesed

						// Scan inside this dir
					RecursiveSmartScanDir(scan_log, *iter, new_dir);
								
						// Back to current dir.
					k_chdir("..");
				}
				else {	// No exec. permission
					// Note: permissions of subdir may 
					//	be changed when the last
					//	modification time of the
					//	current dir is the same

					int e = errno;
					HandleChdirError(scan_log, (*iter)->GetNameStr(), e);

					(*iter)->isUnreadable = 1;
						// Remove all subdir
					(*iter)->subDir.clear();
				}
			}
		}
		hit++;
		if (kcdConfig.cfgSortTree)
			d->subDir.sort();
		return;
	}

					// Cases requiring slow scanning

	DIR *dir = opendir(".");
	if (dir == NULL) {		// No read access
		d->isUnreadable = 2;
		d->subDir.clear();
		return;			// Skip scanning this dir
	}

	if (d->isUnreadable) {		// Used to be unreadable
		d->isUnreadable = 0;	// Clear unreadable flag
	}

	for_each(d->subDir.begin(), d->subDir.end(), AssertDirFlag(0));

	try {
		struct dirent 	*entry;
		while((entry = readdir(dir)) != NULL) {

						// Skip "." and ".." dir
			if (strcmp(entry->d_name, ".") != 0 && 
			    strcmp(entry->d_name, "..") != 0) {

						// Do not delete d2 if it is added to
						// dirTree!
				sptr<DirectoryEntry> d2(new DirectoryEntry(entry));
	
				if (d2->IsDir()) {
				
					string new_dir = current_dir;
					if (new_dir[new_dir.size()-1] != '/')
						new_dir += '/';
				    	new_dir += entry->d_name;

						// Display current progress
					scan_log.PrintDirectory(new_dir);
				
					sptr_list<DirectoryEntry>::iterator iter = d->subDir.begin();
					for ( ; iter != d->subDir.end(); ++iter) {
						if ((*iter)->GetNameStr() == d2->GetNameStr()
						    && (*iter)->IsSymLink() == d2->IsSymLink()) {
							(*iter)->flags = 1;	// Mark it
							break;
						}
					}
					bool is_new_dir = false;
					if (iter == d->subDir.end()) {	// Newly created dir

						d2->flags = 1;		// Mark it
						d->subDir.push_back(d2);
						is_new_dir = true;
					}

					if (!(d2->IsSymLink())) {
					    			// For dirs
						ret = k_chdir(entry->d_name);
				    		if (ret == 0) {
				    				// Dir. can be accesed 
			    		
					    			// Scan inside this dir
							if (is_new_dir)
								RecursiveFullScanDir(scan_log, d2, new_dir);
							else
								// Use *iter instead of d2
				    				RecursiveSmartScanDir(scan_log, *iter, new_dir);
				    			
					    			// Back to current dir.
					    		k_chdir("..");
						}
					    	else {	// No exec. permission
							int e = errno;
							HandleChdirError(scan_log, entry->d_name, e);

							if (is_new_dir)
								// New directory, no need
								// to clear subdirectories.
						    		d2->isUnreadable = 1;
						    	else {
						    		(*iter)->isUnreadable = 1;
								(*iter)->subDir.clear();
							}
				    		}
				    	}
				    	else {		// For symbolic links
							
							// Check date
						if (!is_new_dir
						    && (Max(d2->GetModTime(), d2->GetChangeTime()) 
						      > Max((*iter)->GetModTime(), (*iter)->GetChangeTime()))) {
							(*iter)->UpdateEntry(d2());	// New info
						}
				    	}
				}
			}
		}
	}
	catch (...) {
		closedir(dir);
		throw;
	}
	closedir(dir);

	d->subDir.remove_if(IsDirFlagCleared());
	for_each(d->subDir.begin(), d->subDir.end(), SetDirFlag(0));

	miss++;
	
	if (kcdConfig.cfgSortTree)
		d->subDir.sort();
}

void	SmartScanDir(ScanLog &scan_log)
{
	time_t t1, t2;			// Get current time
	time(&t1);

	scan_log.SetQuiet(kcdConfig.cfgQuietSmart);

	LoadDirFile(true);	// Load saved tree for timestamp info

	SetScanTime(t1);	// New scan time

	if (dirUpdated)		// Full scan already enforced in 
				//	LoadDirFile(...)
		return;

	for_each(dirTree.begin(), dirTree.end(), AssertDirFlag(0));

				// Add and scan directory added to StartDir
				// We mark flags to 1 for newly added dir
				// and 2 for old dir that still exists

	if (kcdConfig.cfgStartDir->empty()) {
		bool	found_dir = false;

				// Remove all previous StartDir entry 
				// that is not '/'.  Duplicate '/' is
				// also removed.
		for (sptr_list<DirectoryEntry>::iterator iter = dirTree.begin();
		     iter != dirTree.end(); ++iter) {

			if ((*iter)->GetNameStr() == "/" && found_dir == false) {
					// This is the directory to keep
				(*iter)->flags = 2;
				found_dir = true;
			}
		}

				// If '/' wasn't in previous StartDir
				// start full scan
		if (found_dir == false) {
			FullScanFromDir(scan_log, "/", true);
		}
	}
	else {
		for (DirList::iterator iter = kcdConfig.cfgStartDir->begin();
		     iter != kcdConfig.cfgStartDir->end(); ++iter) {

			try {
				bool found_dir = false;
				string	dir_name = UnquoteShellChars((*iter)->dir);

				sptr<DirectoryEntry> d2(new DirectoryEntry(dir_name));

				for (sptr_list<DirectoryEntry>::iterator iter2 = dirTree.begin();
				     iter2 != dirTree.end(); ++iter2) {
					if ((*iter2)->GetNameStr() == dir_name
					    && found_dir == false
					    && d2->IsDir()
					    && (*iter2)->IsSymLink() == d2->IsSymLink()) {
							// This is the directory to keep
						(*iter2)->flags = 2;
						found_dir = true;
					}
				}

					// If it wasn't in previous StartDir
					// start full scan
				if (found_dir == false) {
					FullScanFromDir(scan_log, dir_name, true);
				}
			}
			catch (ErrorRange &) {		// Ignore quoting error
			}
		}
	}

				// Remove directory removed from StartDir
	dirTree.remove_if(IsDirFlagCleared());

				// Smart scan dir

	for (sptr_list<DirectoryEntry>::iterator iter = dirTree.begin();
	     iter != dirTree.end(); ++iter) {

				// Directory not just added
		if ((*iter)->IsDir()) {
			
			if ((*iter)->flags == 1)	// Already scanned
							// by FullScanFromDir
				continue;

			string	new_dir = (*iter)->GetNameStr();
			
			scan_log.PrintDirectory(new_dir);

						// Change dir. to StartDir
			if(k_chdir(new_dir) == 0) {
				if (!(*iter)->IsSymLink()) {

						// Recursion...
					RecursiveSmartScanDir(scan_log, *iter, new_dir);
				}
				else {
					DirectoryEntry d2((*iter)->GetNameStr());

						// Check date
					if (Max(d2.GetModTime(), d2.GetChangeTime()) 
					      > Max((*iter)->GetModTime(), (*iter)->GetChangeTime())) {
						(*iter)->UpdateEntry(&d2);	// New info
					}
				}
			}
			else {		// No exec. permission
				int e = errno;
				HandleChdirError(scan_log, new_dir, e);

		    		(*iter)->isUnreadable = 1;
				(*iter)->subDir.clear();
			}
		}
		else {
				// No longer a directory
			(*iter)->flags = 0;
		}
	}

				// Remove entries that are no longer directory
	dirTree.remove_if(IsDirFlagCleared());
	for_each(dirTree.begin(), dirTree.end(), SetDirFlag(0));

	if (kcdConfig.cfgSortTree)
		dirTree.sort();

	ExpandSymLink(scan_log);
	WriteDirFile();

	time(&t2);

	scan_log.PrintTime(t2-t1, _("smart scan mode"));

	dirUpdated = 1;
}

/*************************************************************************
	Partial directory rescan
*************************************************************************/

void	DoPartialScanDir(ScanLog &scan_log, const string &str_, bool full_scan)
{
	if (dirUpdated)		// Full scan already enforced in 
				//	LoadDirFile(...)
		return;

	if (k_chdir(str_)) {
		throw ErrorGenericCommandLine(_("cannot scan directory %$"),
					      str_);
	}

					// Save current working dir
	string str = k_getcwd();
	if (!str.size()) {
		throw ErrorGenericCommandLine(_("cannot obtain full directory name for %$"),
					      str_);
	}

	FindDirInfo *dir_info = FindDir(&dirTree, str, true, false, true);
	if (!dir_info) {
		throw ErrorGenericCommandLine(_("%$ is either a special directory, in `SkipDir\' or not in `StartDir\'"),
					      str_);
	}

	string current_dir = dir_info->str;
	sptr<DirectoryEntry> dir = dir_info->dir_ptr;	// We don't own this

	scan_log.PrintDirectory(current_dir);

						// Start scanning
	if (! k_chdir(current_dir)) {
		if (full_scan) {
			dir->subDir.clear();
			RecursiveFullScanDir(scan_log, dir, dir_info->str);
		}
		else
			RecursiveSmartScanDir(scan_log, dir, dir_info->str);
	}
	else {
		int e = errno;
		HandleChdirError(scan_log, current_dir, e);
	}

	delete dir_info;
}

void	PartialScanDir(ScanLog &scan_log, const vector<string> &str_, bool full_scan)
{
	time_t t1, t2;			// Get current time
	time(&t1);
					// Do not update scan time here

	scan_log.SetQuiet(kcdConfig.cfgQuietPartial);

	LoadDirFile(true);
	for (size_t i = 0; i < str_.size(); ++i) {
		DoPartialScanDir(scan_log, str_[i], full_scan);
		if (i != str_.size()-1 && saveCwd.size())
			k_chdir(saveCwd);
	}

	ExpandSymLink(scan_log);
	WriteDirFile();
	
	time(&t2);
	
	scan_log.PrintTime(t2-t1, _("partial scan mode"));
}

/*************************************************************************
	Helper functions
*************************************************************************/

void	SmartScanDir()
{
	ScanCommandScanLog	scan_log;
	SmartScanDir(scan_log);
}

void	FullScanDir()
{
	ScanCommandScanLog	scan_log;
	FullScanDir(scan_log);
}

void	ScanDir(bool full_scan)
{
	if (full_scan)
		FullScanDir();
	else
		SmartScanDir();
}

void	PartialScanDir(const vector<string> &str_, bool full_scan)
{
	ScanCommandScanLog	scan_log;
	PartialScanDir(scan_log, str_, full_scan);
}

void	ScanDirAndDisplay(const string &str_, bool full_scan)
{
	TreeCommandScanLog	scan_log;
	scan_log.SetQuiet(kcdConfig.cfgQuietPartial);

	FindDirInfo *find = FindDir(&dirTree, str_, false, false);
	if (find) {
		DoPartialScanDir(scan_log, str_, full_scan);
		ExpandSymLink(scan_log);
		WriteDirFile();			// Update saved tree too
	}
	else {
		dirTree.clear();
						// Allow scanning inside
						// skipped directory
		delete kcdConfig.cfgSkipDir;
		kcdConfig.cfgSkipDir = new DirList;

		FullScanFromDir(scan_log, str_);
		ExpandSymLink(scan_log);
	}
}

void	MountDirScanDir(const string &str_)
{
	MountCommandScanLog	scan_log;
	scan_log.SetQuiet(true);

	gtout(cout, _("scanning MountDir: %$\n")) << str_;

	FindDirInfo *find = FindDir(&dirTree, str_, false, false);
	if (find) {
		DoPartialScanDir(scan_log, str_, true);
	}
	delete find;
}
