/*
	dirtree.cc	Directory Tree Functions
	Copyright (c) 2000,2002,2003,2005 Kriang Lerdsuwanakij
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

#include "dirtree.h"
#include "conffile.h"
#include "scandir.h"
#include "strmisc.h"
#include "cstrlib.h"
#include "gentree.h"

sptr_list<DirectoryEntry> dirTree;

const char	uniqueTag[] = "<KCDID#9>";	// File ID
const int	endianTest = 0x1234;		// For endian test

/*************************************************************************
	Time of last scan
*************************************************************************/

time_t	scanTime;		// Initialized by SmartScanDir(...),
				// FullScanDir(...) or ReadSaveFile(...)

					// Provide access to functions
					// in other files.
time_t	GetScanTime()
{
	return scanTime;
}

void	SetScanTime(time_t t)
{
	scanTime = t;
}

/*************************************************************************
	Read directory tree inside ~/.kcd.save.gz
*************************************************************************/

void	ReadSaveFile(sptr_list<DirectoryEntry> &tree, gzFile filegz)
{
					// Assume we seek past file header

	tree.clear();			// Clear current directory tree

	char	c = 0;			// Set c in case gzgetT fails
	k_gzgetc(filegz, c);
	while (c != 'E') {
		if (c != 'D' && c != 'L') {
			throw ErrorGenericFile(_("error in saved directory file\n"
						 "Please rebuild it using `kcd -rf\'"));
		}

		tree.push_back(new DirectoryEntry(filegz, c));

		c = 0;			// Set c in case gzgetT fails
		k_gzgetc(filegz, c);
	}
					// Do not close file !!!
}

/*************************************************************************
	For directory tree sorting
*************************************************************************/

int	operator < (const DirectoryEntry &a, const DirectoryEntry &b)
{
	if (kcdConfig.cfgCaseSensitiveSort)
		return a.GetNameStr() < b.GetNameStr();
	else
		return CompareStringCase(a.GetName(), b.GetName()) < 0;
}

/*************************************************************************
	Load/Write directory file
*************************************************************************/

void	LoadDirHeader(gzFile filegz)
{
	string str = k_gzgetsu(filegz, strlen(uniqueTag));
	if (str != uniqueTag)
		throw ErrorBadFileFormat();

	char	c;
	k_gzgetc(filegz, c);		// Size of int
	if (c != static_cast<char>(sizeof(int)))
		throw ErrorBadFileFormat();

	k_gzgetc(filegz, c);		// Size of long
	if (c != static_cast<char>(sizeof(long)))
		throw ErrorBadFileFormat();

	k_gzgetc(filegz, c);		// Size of bool
	if (c != static_cast<char>(sizeof(bool)))
		throw ErrorBadFileFormat();

	k_gzgetc(filegz, c);		// Size of mode_t
	if (c != static_cast<char>(sizeof(mode_t)))
		throw ErrorBadFileFormat();

	k_gzgetc(filegz, c);		// Size of off_t
	if (c != static_cast<char>(sizeof(off_t)))
		throw ErrorBadFileFormat();

	k_gzgetc(filegz, c);		// Size of time_t
	if (c != static_cast<char>(sizeof(time_t)))
		throw ErrorBadFileFormat();

	k_gzgetc(filegz, c);		// Size of size_t
	if (c != static_cast<char>(sizeof(size_t)))
		throw ErrorBadFileFormat();

	int	i;
	k_gzgetT(filegz, i);		// Read endian check
	if (i != endianTest)
		throw ErrorBadFileFormat();

	k_gzgetT(filegz, scanTime);

	k_gzgetc(filegz, c);		// Read an 'H' (end of header)
	if (c != 'H')
		throw ErrorBadFileFormat();

	ReadDirFileConfig(filegz);

	k_gzgetc(filegz, c);		// Read an 'P' (end of parameters)
	if (c != 'P')
		throw ErrorBadFileFormat();
}

void	LoadDirFile(bool quiet)
{
	FILE	*file;
	gzFile	filegz;
	
	LoadAllConfig(quiet);

					// Test the dir file by openning
					// it without compression

	file = k_fopen(saveFile, "rb");
	if (file == NULL) {		// ~/.kcd.save.gz not found
					// Rescan system dir 
		gtout(cout, _("%$: cannot find %$, file will be rebuilt\n"))
		     << progName << saveFile;
		FullScanDir();

					// Open again
		filegz = gzopen(saveFile, "rb");

		try {
					// Load file
			LoadDirHeader(filegz);
			ReadSaveFile(dirTree, filegz);
		}
		catch(ErrorBadFileFormat &) {
			gzclose(file);
			throw ErrorGenericFile(_("invalid file format in %$"), saveFile);

		}
		catch(ErrorGZIO &) {
			gzclose(file);
			throw ErrorGenericFile(_("error reading file %$"), saveFile);
		}
		gzclose(filegz);
	}
	else {
					// Check if dir file is
					// not empty.
		int	filesize;
		try {
			fseek(file, 0, SEEK_END);
			filesize = ftell(file);
		}
		catch (...) {
			fclose(file);
			throw;
		}

		fclose(file);

		if (filesize <= 0) {	// Empty file or error 
					// Rescan system dir 
			gtout(cout, _("%$: %$ is empty, file will be rebuilt\n"))
			     << progName << saveFile;
			FullScanDir();
		}

					// Open as .gz mode
		filegz = gzopen(saveFile, "rb");
		if (filegz == NULL) {
			gtout(cout, _("%$: %$ is empty, file will be rebuilt\n"))
			     << progName << saveFile;
			FullScanDir();
					// Open as .gz mode
			filegz = gzopen(saveFile, "rb");
			if (filegz == NULL) {
				throw ErrorGenericFile(_("cannot read file %$"), saveFile);
			}
		}

		try {
			try {
				LoadDirHeader(filegz);
			}
			catch (ErrorBadFileFormat &) {
				gzclose(filegz);
				filegz = NULL;

				cout << flush;	// Avoid out-of-order display
					// Rescan system dir
				gtout(cerr, _("%$: %$"
					      " is not compatible with current of "
					      "version of kcd, file will be rebuilt\n"))
				     << progName << saveFile;
				FullScanDir();

						// Open again
				filegz = gzopen(saveFile, "rb");

				LoadDirHeader(filegz);
			}

							// Load file
			ReadSaveFile(dirTree, filegz);
		}
		catch(ErrorBadFileFormat &) {
			if (filegz)
				gzclose(filegz);
			throw ErrorGenericFile(_("invalid file format in %$"), saveFile);
		}
		catch(ErrorGZIO &) {
			if (filegz)
				gzclose(filegz);
			throw ErrorGenericFile(_("error reading file %$"), saveFile);
		}
		catch (...) {
			if (filegz)
				gzclose(filegz);
			throw;
		}

		gzclose(filegz);
	}
}

void	WriteDirFile()
{
	gzFile	file;

	file = gzopen(saveFile, "wb");	// Create new ~/.kcd.save.gz 
	if (file == NULL) {
		throw ErrorGenericFile(_("cannot create file %$"), saveFile);
	}

	try {
	
		k_gzputsu(file, uniqueTag);		// .kcd.save.gz version info 

						// Variable size information
		k_gzputc(file, static_cast<char>(sizeof(int)));
		k_gzputc(file, static_cast<char>(sizeof(long)));
		k_gzputc(file, static_cast<char>(sizeof(bool)));
		k_gzputc(file, static_cast<char>(sizeof(mode_t)));
		k_gzputc(file, static_cast<char>(sizeof(off_t)));
		k_gzputc(file, static_cast<char>(sizeof(time_t)));
		k_gzputc(file, static_cast<char>(sizeof(size_t)));

		k_gzputT(file, endianTest);

		k_gzputT(file, GetScanTime());
	
		k_gzputc(file, 'H');		// End of header

		WriteDirFileConfig(file);

		k_gzputc(file, 'P');		// End of parameters
						// (Reserved for future use)

		for (sptr_list<DirectoryEntry>::iterator iter = dirTree.begin();
		     iter != dirTree.end(); ++iter) {
			(*iter)->gzWrite(file);
		}
		k_gzputc(file, 'E');			// End of list
	}
	catch(ErrorGZIO &) {
		gzclose(file);
		throw ErrorGenericFile(_("error writing file %$"), saveFile);
	}
	catch(...) {
		gzclose(file);
		throw;
	}

	gzclose(file);			// Close ~/.kcd.save.gz 
}

/*************************************************************************
	Expand every symbolic link to real directory names
*************************************************************************/

void	ResolveSymLink(sptr_list<DirectoryEntry> *tree, 
		       sptr<DirectoryEntry> &d, const string &current_dir)
{
	int	ret;
	
	if (k_access(current_dir, F_OK))
						// Shouldn't come here
		d->isLinkDestSkip = 3;		// Not found
	else if (k_access(current_dir, X_OK)) {
		d->isLinkDestSkip = 0;
		d->isLinkDestUnreadable = 1;	// Cannot change to
						// this dir
	}
	else if (k_access(current_dir, R_OK)) {
		d->isLinkDestSkip = 0;
		d->isLinkDestUnreadable = 2;	// Cannot read dir
	}
	else {
		d->isLinkDestUnreadable = 0;	// Access OK

		ret = k_chdir(current_dir);
		if (ret == 0) {
					// Return empty string if unreadable
					// otherwise it's the current working 
					// dir with no symbolic link involved
			string	linked_dir = k_getcwd();
			if (!linked_dir.size()) {
				d->isLinkDestSkip = 3;
				return;
			}
			d->UpdateLinkDest(linked_dir);

			FindDirInfo *dir_info = FindDir(tree, linked_dir, false, false);
			if (dir_info)
				d->isLinkDestSkip = 0;
			else
				d->isLinkDestSkip = 1;
			delete dir_info;
		}
		else {				// Shouldn't come here
			d->isLinkDestSkip = 3;
		}
	}
}

void	RecursiveResolveSymLink(sptr_list<DirectoryEntry> *tree,
				sptr_list<DirectoryEntry> &d,
				const string &current_dir)
{
	for (sptr_list<DirectoryEntry>::iterator iter = d.begin();
	     iter != d.end(); ++iter) {

		string new_dir = current_dir;
		new_dir += (*iter)->GetNameStr();

		if ((*iter)->IsSymLink()) {
			ResolveSymLink(tree, *iter, new_dir);
		}
		else {
						// Append trailing slash
			if (new_dir[new_dir.size()-1] != '/')
				new_dir += '/';

						// Recursion...
			RecursiveResolveSymLink(tree, (*iter)->subDir, new_dir);
		}
	}
}

