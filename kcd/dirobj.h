/*
	dirobj.h	Directory Object Header
	Copyright (c) 1997-8,2000,2002,2003,2005 Kriang Lerdsuwanakij
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

#ifndef __K_DIROBJ_H
#define __K_DIROBJ_H

#include "config.h"

#include <time.h>

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) ((dirent)->d_namlen)
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#include <sys/stat.h>

#include "gzfileio.h"
#include "list.h"

USING_NAMESPACE_STD;

/*************************************************************************
	Directory class.
*************************************************************************/

class	DirectoryEntry {
	protected:
		string	name;		// Directory name
		string	link;		// Link as what readlink(...) returns
		string	linkDest;	// Expanded link to real directory

					// Is POSIX umode_t or mode_t ?
		mode_t	linkType;	// Link mode
		mode_t	type;		// File mode
		time_t	modTime;	// Time of last modification
		time_t	changeTime;	// Time of last change

		nlink_t	numLink;	// Number of hard links
		bool	hasNumLink;	// Is numLink avaliable

		void	Init();
		void	gzWriteInfo(gzFile file);
	public:
		bool	bookmarked;

		char	isSkip;		// 0 = Not skipped
					// 1 = Skipped (MountDir included)
					// 2 = Temporary created (not in
					//     save file)
					// 3 = No filelist for MountDir
		char	isUnreadable;	// 0 = R-X, 1 = R--, 2 = --X

		char	isLinkDestSkip;	// 0 = Not skipped (exists and accessible)
					// 1 = Skipped (exists but link to outside
					//     save file)
					// 2 = Temporary created (not in
					//     save file)
					// 3 = Not found (not exists)
					// 4 = Uninitialized [used temporary]
		char	isLinkDestUnreadable;
		
		char	flags;		// Flags used internally, useful
					//	for functions outside
		sptr_list<DirectoryEntry> subDir;

		DirectoryEntry(const dirent *d);
		DirectoryEntry(const string &name_);
		DirectoryEntry(gzFile file, char id);
		~DirectoryEntry();

		bool	IsDir()	const		{ return S_ISDIR(type); }
		bool	IsSymLink() const	{ return S_ISLNK(linkType); }

		void	UpdateEntry(DirectoryEntry *d);
		void	UpdateLinkDest(const string &s) { linkDest = s; }

		const char	*GetName() const	{ return name.c_str(); }
		const char	*GetLink() const	{ return link.c_str(); }
		const char	*GetLinkDest() const	{ return linkDest.c_str(); }

		const string	&GetNameStr() const	{ return name; }
		const string	&GetLinkStr() const	{ return link; }
		const string	&GetLinkDestStr() const { return linkDest; }

		time_t	GetModTime() const		{ return modTime; }
		time_t	GetChangeTime()	const		{ return changeTime; }

		bool	HasNumLink() const		{ return hasNumLink; }
		nlink_t	GetNumLink() const		{ return numLink; }

		void	gzWrite(gzFile file);
};

#endif	/* __K_DIROBJ_H */
