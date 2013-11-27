/*
	gentree.h	Generate Directory Tree For Display Header
	Copyright (c) 2000, 2002, 2003, 2005 Kriang Lerdsuwanakij
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

#ifndef __K_GENTREE_H
#define __K_GENTREE_H

#include "config.h"

#include "dirobj.h"

extern string saveCwd;
string	my_strftime(const char *format, const struct tm *tm);

struct FindDirInfo {
	sptr<DirectoryEntry> dir_ptr;
	string		str;

	FindDirInfo(sptr<DirectoryEntry> dir_ptr_, const string &str_) : 
		dir_ptr(dir_ptr_), str(str_)
	{
	}
};

FindDirInfo *FindDir(sptr_list<DirectoryEntry> *dirTree, const string &str,
		     bool honor_cfg, bool change = true);
FindDirInfo *FindDir(sptr_list<DirectoryEntry> *dirTree, const string &str,
		     bool honor_cfg, bool change_top, 
		     bool change_sub);

void	DisplayTree(const string &root, bool use_mount_dir = true,
		    bool rescan = false);
void	DisplayFullTree(bool use_mount_dir = true, bool rescan = false);

void	CD(const string &linktext, bool entire_tree = false, 
	   bool use_mount_dir = true, size_t match_no = 0);
void	ShowBookmarks(const string &linktext);

void	AddBookmarks(int argc, char *argv[], int argcBegin);
void	RemoveBookmarks(int argc, char *argv[], int argcBegin);

void	ImportBookmarks(const string &file);
void	ExportBookmarks(const string &file);

#endif	/* __K_GENTREE_H */
