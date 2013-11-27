/*
	dirobj.cc	Directory Object Class
	Copyright (c) 1996,1997-9,2000,2002,2003,2005 Kriang Lerdsuwanakij
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

#include "dirobj.h"
#include "cstrlib.h"
#include "error.h"

DirectoryEntry::DirectoryEntry(const dirent *d) : 
			name(d->d_name), link(""), linkDest(""),
			numLink(0), hasNumLink(false), bookmarked(false), 
			subDir()
{
	flags = 0;
	isLinkDestSkip = 4;		// Uninitialized
	isLinkDestUnreadable = 0;
#ifdef CLIB_HAVE_D_TYPE
	if (d->d_type != DT_UNKNOWN &&
	    d->d_type != DT_DIR && d->d_type != DT_LNK) {
		linkType = 0;		// Ignore
		type = 0;
	}
	else {				// Get actual type if DT_UNKNOWN
					// Get timestamp if DT_DIR
					// Get link destination if DT_LNK
	 	Init();
	}
#else
	Init();
#endif
}

DirectoryEntry::DirectoryEntry(const string &name_) : 
			name(name_), link(""), linkDest(""),
			numLink(0), hasNumLink(false), bookmarked(false), 
			subDir()
{
	flags = 0;
	isLinkDestSkip = 4;		// Uninitialized
	isLinkDestUnreadable = 0;
	Init();
}

DirectoryEntry::DirectoryEntry(gzFile file, char id) : 
			name(""), link(""), linkDest(""),
			numLink(0), hasNumLink(false), bookmarked(false),
			subDir()
{
	char	c;
	flags = 0;

	switch (id) {
		case 'L':
			link = k_gzgets(file);
			k_gzgetT(file, linkType);
	
			linkDest = k_gzgets(file);
			k_gzgetT(file, isLinkDestSkip);
			k_gzgetT(file, isLinkDestUnreadable);

			name = k_gzgets(file);

			k_gzgetT(file, type);
			k_gzgetT(file, modTime);
			k_gzgetT(file, changeTime);
			break;
			
		case 'D':
			k_gzgetc(file, c);
			isUnreadable = (c & 0xC) >> 2;
			isSkip = (c & 2) >> 1;

			name = k_gzgets(file);

			k_gzgetT(file, type);
			k_gzgetT(file, modTime);
			k_gzgetT(file, changeTime);

				// For dir. linkType == type and link is ""
			link = "";
			linkType = type;

			if (c & 1) {	// Contain subdirectory ?
				char	c2 = 0;
				k_gzgetc(file, c2);
				while (c2 != 'E') {
					if (c2 != 'D' && c2 != 'L') {
						throw ErrorGenericFile(_("error in saved directory file\n"
									"Please rebuild it using `kcd -rf\'"));
					}
					
					subDir.push_back(new DirectoryEntry(file, c2));
					
					c2 = 0;
					k_gzgetc(file, c2);
				}
			}
			break;
			
		default:
			throw ErrorGenericFile(_("error in saved directory file\n"
						"Please rebuild it using `kcd -rf\'"));
	}
}

void	DirectoryEntry::UpdateEntry(DirectoryEntry *d)
{
					// Do not change name
	link = d->link;
	linkType = d->linkType;	
	type = d->type;

	modTime = d->modTime;
	changeTime = d->changeTime;

	isSkip = d->isSkip;
	isUnreadable = d->isUnreadable;

	linkDest = d->linkDest;
	isLinkDestSkip = d->isLinkDestSkip;
	isLinkDestUnreadable = d->isLinkDestUnreadable;

	bookmarked = d->bookmarked;

	// No update flag

}

void	DirectoryEntry::Init()
{
	link = "";
	
	isSkip = 0;
	isUnreadable = 0;

	struct stat	s;
	if (k_lstat(name, &s)) {
		linkType = 0;		// Zero all mode fields
   					// It will be discarded in the
   					// calling routine
		type = 0;
		return;
	}
	linkType = s.st_mode;

	modTime = s.st_mtime;		// Modify time
	changeTime = s.st_ctime;	// Change time

	numLink = s.st_nlink;
	hasNumLink = true;
	
					// Only issue system call once
					// unless it is a symbolic link
	if (IsSymLink()) {
		if (k_stat(name, &s)) {	// Get stats for linked file/dir
   					// Cyclic symbolic links
   			linkType = 0;	// Zero all mode fields
   					// It will be discarded in the
   					// calling routine
   			type = 0;
			return;
  		}
  		
		link = k_readlink(name);
	}

	type = s.st_mode;		// Linked file/dir stats
}

DirectoryEntry::~DirectoryEntry()
{
}

void	DirectoryEntry::gzWriteInfo(gzFile file)
{
	k_gzputs(file, name);
	k_gzputT(file, type);
	k_gzputT(file, modTime);
	k_gzputT(file, changeTime);
}

void	DirectoryEntry::gzWrite(gzFile file)
{
	if (link.size() != 0) {
		k_gzputc(file, 'L');
		k_gzputs(file, link);
		k_gzputT(file, linkType);

		k_gzputs(file, linkDest);
		k_gzputT(file, isLinkDestSkip);
		k_gzputT(file, isLinkDestUnreadable);

		gzWriteInfo(file);
	}
	else {
		k_gzputc(file, 'D');
		char c = (subDir.size()?1:0);
		
		// Bit packed flags
		// XXXXUUSE
		// UU = unreadable
		// S = skipped (only 0 or 1)
		// E = empty
		
		k_gzputc(file, isUnreadable*4+isSkip*2+c);

		gzWriteInfo(file);

		if (c) {
			for (sptr_list<DirectoryEntry>::iterator iter = subDir.begin();
			     iter != subDir.end(); ++iter)
				(*iter)->gzWrite(file);
			k_gzputc(file, 'E');		// End of list
		}
				// For dir. linkType == type and link is ""
	}
}
