/*
	dirutil.cc	Directory Utility Functions
	Copyright (c) 2000, 2001, 2003, 2004, 2005 Kriang Lerdsuwanakij
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

#include "dirutil.h"
#include "cstrlib.h"
#include "gtstream.h"
#include "error.h"
#include "strmisc.h"

#if HAVE_UNISTD_H
# include <unistd.h>
# include <sys/types.h>
#endif

/*************************************************************************
	Symlink Expansion
*************************************************************************/

string	ResolveSymlink(const string &src)
{
	string save_cwd = k_getcwd();
	if (!save_cwd.size())
		return src;

	if (k_chdir(src)) {
		k_chdir(save_cwd);
		return src;
	}

	string dest = k_getcwd();
	k_chdir(save_cwd);
	return dest;
}

/*************************************************************************
	Convert Relative to Absolute Path
*************************************************************************/

string	RelativeToAbsolutePath(const string &path, const string &cwd)
{
	if (!path.size())
		return path;
	if (path[0] == '/' || path[0] == '~')
		return path;

	string temp = cwd;
	temp += '/';
	temp += path;
	return temp;
}

/*************************************************************************
	Directory Expansion
*************************************************************************/

						// Expand ~ to full dir. path
						// quote = true, remove \ quote
						// regex = true, use \\. instead of .
string	ExpandDirectory(const string &src, bool quote, bool regex)
{
	string	outStr;
	if (!src.size())			// String empty
		return outStr;			// Nothing to do
		
	if (src[0] == '~') {
		struct passwd	*p;
		
					// "~" or "~/..."
		if (src.size() == 1 || src[1] == '/') {
					// Get passwd entry for current user
			p = getpwuid(getuid());
			if (p == NULL) {
				char *env = getenv("HOME");
				if (env == NULL) {
					// Give up
					throw ErrorGenericSyntax(_("error obtaining the user"
								   " home directory"));
				}
	
				outStr = env;
			}
			else {
			
					// Store home directory
				outStr = p->pw_dir;
			}

					// No sysadmin on earth should create user 
					// with strange home directory name but
					// this code is here just in case.
			if (quote) {
				if (regex)
					outStr = QuoteRegexChars(outStr);
				else
					outStr = QuoteGlobChars(outStr);
			}

					// We allow globing in SkipDir
			string	temp(src, 1, string::npos);
			if (quote) {
				try {
					temp = UnquoteConfigChars(temp);
				}
				catch (ErrorRange &) {
					throw ErrorGenericSyntax(_("bad quoting for"
								   " directory `%$\'"),
								 temp);
				}
			}
			outStr.append(temp);
		}
		else {			// ~user

			string::size_type	s = src.find('/');
			if (s == string::npos) {
					// Skip the `~'
				string	temp(src, 1, string::npos);
				if (quote) {
					try {
						temp = UnquoteConfigChars(temp);
					}
					catch (ErrorRange &) {
						throw ErrorGenericSyntax(_("bad quoting for"
									   " directory `%$\'"),
									 temp);
					}
				}

				p = k_getpwnam(temp);
				if (p == NULL) {
					throw ErrorGenericSyntax(_("error obtaining home"
								   " directory for user `%$\'"),
								 temp);
				}
				
				outStr = p->pw_dir;
			}
			else {
			
				string	temp(src, 1, s-1);
				if (quote) {
					try {
						temp = UnquoteConfigChars(temp);
					}
					catch (ErrorRange &) {
						throw ErrorGenericSyntax(_("bad quoting for"
									   " directory `%$\'"),
									 temp);
					}
				}

				p = k_getpwnam(temp);
				if (p == NULL) {
					throw ErrorGenericSyntax(_("error obtaining home"
								   " directory for user `%$\'"),
								 temp);
				}
				
				outStr = p->pw_dir;
			}
					// No sysadmin on earth should create user 
					// with strange home directory name but
					// this code is here just in case.
			if (quote) {
				if (regex)
					outStr = QuoteRegexChars(outStr);
				else
					outStr = QuoteGlobChars(outStr);
			}

					// Append remaining of directory name.
			if (s != string::npos) {
				outStr.append(src, s+1, string::npos);
					// We allow globing in SkipDir
				string	temp(src, s+1, string::npos);
				if (quote) {
					try {
						temp = UnquoteConfigChars(temp);
					}
					catch (ErrorRange &) {
						throw ErrorGenericSyntax(_("bad quoting for"
									   " directory `%$\'"),
									 temp);
					}
				}
				outStr.append(temp);
			}
		}
	}
	else {
		if (quote) {
			try {
				outStr = UnquoteConfigChars(src);
			}
			catch (ErrorRange &) {
				throw ErrorGenericSyntax(_("bad quoting for"
							   " directory `%$\'"),
							 src);
			}
		}
		else
			outStr = src;
	}

	if (outStr[0] != '/' && outStr[0] != '\\'
	    && (regex || (outStr[0] != '*' && outStr[0] != '?' 
			  && outStr[0] != '['))
	    && (!regex || (outStr[0] != '^' && outStr[0] != '.'
			   && outStr[0] != '(' && outStr[0] != '['))) {
		throw ErrorGenericSyntax(_("bad directory `%$\'"), outStr);
	}
	
	string::size_type	ptr;
	while((ptr = outStr.find("//")) != string::npos) {
						// Remove the first `/'
#ifdef _CXX_STRING_ERASE
		outStr.erase(ptr, 1);
#else
		// Same as outStr.erase(ptr, 1);
		outStr.replace(ptr, 1, static_cast<string::size_type>(0), 0);
#endif
	}

	const char *s1 = regex ? "/\\./" : "/./";
	const char *s2 = regex ? "/\\." : "/.";
	int adj = regex ? 1 : 0;
	while((ptr = outStr.find(s1)) != string::npos ||
	      (ptr = StrLast(outStr, s2)) != string::npos) {
		if (ptr+2+adj == outStr.size()) {	// Dir ends with dir/.
		
						// Delete ptr+1 .. ptr+1
						// Turn it to    dir/
						// [Note: dir can be empty.
						//  The trailing / will be
						//  handled later.]
#ifdef _CXX_STRING_ERASE
			outStr.erase(ptr+1, 1+adj);
#else
			// Same as outStr.erase(ptr+1, 1+adj);
			outStr.replace(ptr+1, 1+adj, static_cast<string::size_type>(0), 0);
#endif
		}
		else {
						// Delete ptr .. ptr+1
#ifdef _CXX_STRING_ERASE
			outStr.erase(ptr, 2+adj);
#else
			// Same as outStr.erase(ptr, 2+adj);
			outStr.replace(ptr, 2+adj, static_cast<string::size_type>(0), 0);
#endif
		}
	}

#if 0 /* These don't work when dir/.. where dir is a symlink */
	s1 = regex ? "/\\.\\./" : "/../";
	s2 = regex ? "/\\.\\." : "/..";
	adj = regex ? 2 : 0;
	while((ptr = outStr.find(s1)) != string::npos ||
	      (ptr = StrLast(outStr, s2)) != string::npos) {
		string::size_type	ptr2 = ptr;
		if (ptr2 != 0) {		// Have a parent directory
			do {
				ptr2--;
			}
			while(outStr[ptr2] != '/' && ptr2 != 0);
		}

		if (ptr+3+adj == outStr.size()) {	// Dir ends with dir1/dir2/..
						// Turn it to    dir1/
						// [Note: dir can be empty]
						// FIXME: Only works when dir2 is not a symlink
#ifdef _CXX_STRING_ERASE
			outStr.erase(ptr2+1, string::npos);
#else
			// Same as outStr.erase(ptr2+1, string::npos);
			outStr.replace(ptr2+1, string::npos,
				       static_cast<string::size_type>(0), 0);
#endif
		}
		else {				// dir1/dir2/../dir3
			ptr += 4+adj;
						// Remove dir2/../
						// FIXME: Only works when dir2 is not a symlink
#ifdef _CXX_STRING_ERASE
			outStr.erase(ptr2+1, ptr-(ptr2+1));
#else
			// Same as outStr.erase(ptr2+1, ptr-(ptr2+1));
			outStr.replace(ptr2+1, ptr-(ptr2+1),
				       static_cast<string::size_type>(0), 0);
#endif
		}
	}
#endif

	string::size_type	len = outStr.size();	// Remove trailing `/'
	if (len > 1 && outStr[len-1] == '/') {
#ifdef _CXX_STRING_ERASE
		outStr.erase(len-1, 1);
#else
			// Same as outStr.erase(len-1, 1);
		outStr.replace(len-1, 1, static_cast<string::size_type>(0), 0);
#endif
	}
		
	return outStr;
}

