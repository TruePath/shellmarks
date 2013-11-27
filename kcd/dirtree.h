/*
	dirtree.h	Directory Tree Functions Header
	Copyright (c) 2000,2002,2003 Kriang Lerdsuwanakij
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

#ifndef __K_DIRTREE_H
#define __K_DIRTREE_H

#include "config.h"

#include "list.h"
#include "dirobj.h"

extern sptr_list<DirectoryEntry> dirTree;
extern string	saveFile;
extern string	bookmarkFile;

time_t	GetScanTime();
void	SetScanTime(time_t t);

void	LoadDirFile(bool quiet);
void	WriteDirFile();

int	operator < (const DirectoryEntry &a, const DirectoryEntry &b);

void	RecursiveResolveSymLink(sptr_list<DirectoryEntry> *tree, 
				sptr_list<DirectoryEntry> &d,
				const string &current_dir);

#endif	/* __K_DIRTREE_H */
