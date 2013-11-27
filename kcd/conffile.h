/*
	conffile.h	Configuration File Functions Header
	Copyright (c) 2000, 2002 Kriang Lerdsuwanakij
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

#ifndef __K_CONFFILE_H
#define __K_CONFFILE_H

#include "config.h"
#include "confobj.h"

bool	IsProfileValid(const string &s, bool allowGlobal);
void	SetCurrentProfile(const string &s);
string	GetCurrentProfile();

void	NeedSecureUserConfig();
void	LoadAllConfig(bool quiet);
void	PrintConfig();
void	PrintAttrConfig();

void	ReadDirFileConfig(gzFile file);
void	WriteDirFileConfig(gzFile file);

#endif	/* __K_CONFFILE_H */
