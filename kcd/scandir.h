/*
	scandir.h	Scan Directory Functions Header
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

#ifndef __K_SCANDIR_H
#define __K_SCANDIR_H

#include "config.h"

#include <string>
#include CXX__HEADER_vector

#include CXX__HEADER_iostream

USING_NAMESPACE_STD;

class ScanLog {
	protected:
		bool	quiet;
	public:
		virtual void PrintDirectory(const string &str) = 0;
		virtual void PrintWork(const string &str) = 0;
		virtual void PrintTime(long secdiff, const string &mode) = 0;
		virtual void PrintStartDirNotDirectory(const string &str) = 0;
		virtual void PrintStartDirNotFound(const string &str) = 0;
		virtual void PrintChdirError(const string &dir, const string &str) = 0;
		void SetQuiet(bool quiet_) { quiet = quiet_; }
		virtual ~ScanLog() {}
};

class CommandLineScanLog : public ScanLog {
	public:
		void PrintDirectory(const string &str) { if (!quiet) cout << str << '\n'; }
		void PrintWork(const string &str) { if (!quiet) cout << str << '\n'; }
		void PrintTime(long secdiff, const string &mode);
		void PrintChdirError(const string &dir, const string &str);
		virtual ~CommandLineScanLog() {}
};

class ScanCommandScanLog : public CommandLineScanLog {
	public:
		void PrintStartDirNotDirectory(const string &str);
		void PrintStartDirNotFound(const string &str);
};

class TreeCommandScanLog : public CommandLineScanLog {
	public:
		void PrintStartDirNotDirectory(const string &str);
		void PrintStartDirNotFound(const string &str);
};

class MountCommandScanLog : public CommandLineScanLog {
	public:
		void PrintStartDirNotDirectory(const string &str);
		void PrintStartDirNotFound(const string &str);
};

void	FullScanDir();
void	SmartScanDir();
void	ScanDir(bool full_scan);
void	PartialScanDir(const vector<string> &str_, bool full_scan);
void	ScanDirAndDisplay(const string &str_, bool full_scan);
void	MountDirScanDir(const string &str_);

#endif	/* __K_SCANDIR_H */
