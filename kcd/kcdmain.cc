/*
	kcdmain.cc	Kriang's Change Directory
	Copyright (c) 1996-9,2000-4,2005,2007,2009 Kriang Lerdsuwanakij
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

#include "config.h"

#include CXX__HEADER_iostream
#include CXX__HEADER_iomanip

#include CXX__HEADER_algorithm

#ifdef HAVE_LOCALE_H
# include CXX__HEADER_clocale
#endif

#include "cxxlib.h"

#include "error.h"
#include "conffile.h"
#include "scandir.h"
#include "gentree.h"
#include "strmisc.h"
#include "cstrlib.h"
#include "dirutil.h"
#include "dirtree.h"

// setupcurses.h is included in miscobj.h

// Get version number and release date
#include "version.cc"
char	progName[] = "kcd";

string	saveFile;		// For home dir + ".kcd.save.gz"
string	bookmarkFile;		// For home dir + ".kcd.bookmark.gz"
string	dirFile;		// For home dir + ".kcd.newdir"
string	confLocalFile;		// For home dir + ".kcd.conf"

/*************************************************************************
	Display program version, help.
*************************************************************************/

void	Version()
{
	cout << progName << ' ' << version << '\n';
}

void	PrintBanner()
{
	struct tm rel_time;
	rel_time.tm_mday = rel_day;
	rel_time.tm_mon = rel_month;
	rel_time.tm_year = rel_year;

	cout << progName << ' ' << version
	     << _(" - Kriang's Change Directory\n");
	gtout(cout, _("(c) 1996-9,2000-5,2007,2009 Kriang Lerdsuwanakij    "
		      "Release date: %$\n\n"))
	     << my_strftime(_("%b %e, %Y"), &rel_time);

#ifdef DEBUG_EXPAND_DIR
	cout << ExpandDirectory("~zsh/bin") << '\n'
	     << ExpandDirectory("~zsh") << '\n'
	     << ExpandDirectory("~") << '\n'
	     << ExpandDirectory("~/bin") << '\n'
	     << ExpandDirectory("////bin///") << '\n'
	     << ExpandDirectory("~/../bin") << '\n'
	     << ExpandDirectory("/././...//") << '\n'
	     << ExpandDirectory("/../usr1") << '\n'
	     << ExpandDirectory("/usr1/../usr2/../../usr3") << '\n';
#endif
}

void	Help()
{
	PrintBanner();
	gtout(cout, _("Usage:   %$ [-B] [PATTERN] [NUMBER]\n"
		      "  or:    %$ [OPTION]\n")) << progName << progName;
	cout << _("Available options:\n");
	cout << _("     -b [PATTERN]   Display bookmarked directories matching PATTERN\n"
		  "                    (default = display all)\n"
		  "     -ba [DIR ...]  Bookmark DIR list (default = current directory)\n"
		  "     -br [DIR ...]  Remove DIR list bookmark\n"
		  "     -B             Use entire tree even a bookmarked match is found\n");
	cout << _("     --config       Display current configuration\n"
		  "     --configattr   Display current screen attribute configuration\n"
		  "     --features     Display features included during compilation\n"
		  "     -h, --help     Display this help\n"
		  "     --helpinst     Display help about shell alias/function installation\n");
	cout << _("     -M             Don't scan directories given by MountDir\n");
	cout << _("     -p NAME        Use configuration profile NAME.  If used, this must be\n"
		  "                    the first option in the command line\n"
		  "     --profiles     Display the list of configuration profile\n");
	cout << _("     -r             Rescan directory tree\n"
		  "     -rf            Rescan directory tree using full rescan mode\n"
		  "     -rp DIR        Rescan part of directory tree beginning at DIR\n"
		  "     -rq            Rescan directory tree, do not display progress\n"
		  "     -rQ            Rescan directory tree, display progress\n"
		  "     -t [DIR]       Display DIR as directory tree (default = current\n"
		  "                    directory)\n"
		  "     -tr [DIR]      Like -t but rescan directory tree before display\n"
		  "     -T             Display full directory tree\n"
		  "     -v, --version  Display version number\n\n");
}

void	HelpInst()
{
	PrintBanner();
	gtout(cout, _("Usage:   eval `%$ [OPTION]`\n")) << progName;
	cout << _("Available options:\n");
	cout << _("     -ia            Print command for shell alias installation\n"
		  "                    (Works with bash, pdksh, zsh)\n");
	cout << _("     -ic            Print command for shell alias installation\n"
		  "                    (Works with tcsh)\n");
	cout << _("     -if            Print command for shell function installation\n"
		  "                    (Works with ash)\n");
	cout << _("     -ias DIR,      Same as -ia,-ic or -if but use the specified\n"
		  "        -ics DIR,   directory\n"
		  "        -ifs DIR\n\n");
}

void	Features()
{
	PrintBanner();
	
	cout << _("     xterm/rxvt resize   : ")
#ifdef	HAVE_RESIZETERM
	     << _("Yes")
#else
	     << _("No")
#endif
	     << "\n";

	cout << _("     mouse support       : ")
#ifdef	NCURSES_MOUSE_VERSION
	     << _("Yes")
#else
	     << _("No")
#endif
	     << "\n";

	cout << _("     default scroll bar  : ")
#ifdef	KCD_NC_GOOD_SCROLL
	     << _("Yes")
#else
	     << _("No")
#endif
	     << "\n";
	
	cout << _("     UTF-8 support       : ")
#ifdef	USE_UTF8_MODE
	     << _("Yes")
#else
	     << _("No")
#endif
	     << "\n";
	
	cout << _("     d_type speedup      : ")
#ifdef	CLIB_HAVE_D_TYPE
	     << _("Yes")
#else
	     << _("No")
#endif
	     << "\n";
	
	cout << _("     regular expression  : ")
#ifdef	CLIB_HAVE_REGEX
	     << _("Yes")
#else
	     << _("No")
#endif
	     << "\n";
	
	cout << '\n';
}

void	Profiles()
{
	PrintBanner();

	cout << _("Profiles:\n");

	for (configMap::iterator iter = allConfig.begin();
	     iter != allConfig.end();
	     ++iter) {
		cout << "     " << iter->first << '\n';
	}
	cout << '\n';
}

/*************************************************************************
	Initialize ~/.kcd.newdir to contain "."
*************************************************************************/

void	InitNewDir()
{
	FILE	*file;

	file = k_fopen(dirFile, "wb");
	if (file == NULL) {		// Cannot create or truncate
					// ~/.kcd.newdir
		throw ErrorGenericFile(_("cannot create file %$"), dirFile);
	}
	fputs(".", file);		// Make sure cd "`cat ~/.kcd.newdir`"
					// will not cause problem if we are
					// not going to change dir., i.e.,
					// when displaying help, etc.
	fclose(file);
}

/*************************************************************************
	Main
*************************************************************************/

string	home_dir;

void	SetSaveFile()
{
					// Initialize ~/.kcd.save.*.gz,
	const char *env = getenv("KCD_DIRTREE");
	if (env && strlen(env)) {
		saveFile = env;
	}
	else {
		saveFile = home_dir;
		saveFile += ".kcd.save.";
		saveFile += GetCurrentProfile();
		saveFile += ".gz";
	}

					// Initialize ~/.kcd.bookmark.*.gz,
	env = getenv("KCD_BOOKMARK");
	if (env && strlen(env)) {
		bookmarkFile = env;
	}
	else {
		bookmarkFile = home_dir;
		bookmarkFile += ".kcd.bookmark.";
		bookmarkFile += GetCurrentProfile();
	}
}

void	mainReal(int argc, char *argv[])
{
	const char *env = getenv("IFS");
	if (env && env[0] != 0 && env[0] != ' ' && env[0] != '\t')
		throw ErrorGenericFile(_("invalid IFS environment variable setting"));

#ifdef HAVE_SETENV
	setenv("IFS", " \t\n", 1);	// Security precaution
	unsetenv("NLSPATH");
#else
	putenv("IFS= \t\n");		// Security precaution
	putenv("NLSPATH=");
#endif

#ifdef HAVE_LOCALE_H
	setlocale(LC_ALL, "");		// FIXME: Switch to C++ locale object
					// when available

	bindtextdomain(PACKAGE, KCD_LOCALEDIR);
	textdomain(PACKAGE);
#endif

	SetCurrentProfile("kcd");

					// Save current working dir
	saveCwd = k_getcwd();
					// Initialize ~/.kcd.newdir and
					// ~/.kcd.conf filename
	home_dir = ExpandDirectory("~");
	if (home_dir[home_dir.size()-1] != '/')	// Not end with "/"
		home_dir += '/';	// Need to add a "/" after home dir

	env = getenv("KCD_TEMP");
	if (env && strlen(env)) {
		dirFile = env;
	}
	else {
		dirFile = home_dir;
		dirFile += ".kcd.newdir";	// Add appropriate filename
	}

	env = getenv("KCD_CONFIG");
	if (env && strlen(env)) {
		confLocalFile = env;
		NeedSecureUserConfig();		// Turn on security check
	}
	else {
		confLocalFile = home_dir;
		confLocalFile += ".kcd.conf";	// Add appropriate filename
	}

	InitNewDir();			// Initialize ~/.kcd.newdir

	if (argc == 1) { 		// No parameters
		SetSaveFile();

		CD("");			// Show entire tree
	}
	else {
		int	argcCur = 1;
		int	argcLeft = argc-1;

		bool	rescan = false;
		bool	entire_tree = false;
		bool	forceFullScan = false;
		bool	use_mount_dir = true;

		vector<string>	partial_dir;

		while (argcLeft) {
			if (strcmp(argv[argcCur], "-h") == 0) {
				Help();
				return;
			}
			else if (strcmp(argv[argcCur], "--help") == 0) {
				Help();
				return;
			}

			else if (strcmp(argv[argcCur], "--helpinst") == 0) {
				HelpInst();
				return;
			}

			else if (strcmp(argv[argcCur], "-v") == 0) {
				Version();
				return;
			}
			else if (strcmp(argv[argcCur], "--version") == 0) {
				Version();
				return;
			}
			else if (strcmp(argv[argcCur], "--features") == 0) {
				Features();
				return;
			}
			else if (strcmp(argv[argcCur], "--profiles") == 0) {
				LoadAllConfig(false);
				Profiles();
				return;
			}
			else if (strcmp(argv[argcCur], "--config") == 0) {
				LoadAllConfig(false);
				PrintConfig();
				return;
			}
			else if (strcmp(argv[argcCur], "--configattr") == 0) {
				LoadAllConfig(false);
				PrintAttrConfig();
				return;
			}
			else if (strcmp(argv[argcCur], "-ia") == 0) {
				LoadAllConfig(true);
				if (argcLeft == 1) {
					for (configMap::iterator i = allConfig.begin(); i != allConfig.end(); ++i) {
						if ((*i).first != "*")
							cout << "alias " << (*i).first << "=\'. /usr/bin/kcdscr -p "
							     << (*i).first << "\';\n";
					}
				}
				else
					throw ErrorGenericCommandLine(_("argument not allowed for `-ia\'"));
				return;
			}
			else if (strcmp(argv[argcCur], "-ic") == 0) {
				LoadAllConfig(true);
				if (argcLeft == 1)
					for (configMap::iterator i = allConfig.begin(); i != allConfig.end(); ++i) {
						if ((*i).first != "*")
						cout << "alias " << (*i).first << " \'source /usr/bin/kcdscr -p "
						     << (*i).first << "\';\n";
					}
				else
					throw ErrorGenericCommandLine(_("argument not allowed for `-ic\'"));
				return;
			}
			else if (strcmp(argv[argcCur], "-if") == 0) {
				LoadAllConfig(true);
				if (argcLeft == 1)
					for (configMap::iterator i = allConfig.begin(); i != allConfig.end(); ++i) {
						if ((*i).first != "*")
						cout << (*i).first << " () { . /usr/bin/kcdscr -p "
						     << (*i).first << " \"$@\" ; };\n";
					}
				else
					throw ErrorGenericCommandLine(_("argument not allowed for `-if\'"));
				return;
			}
			else if (strcmp(argv[argcCur], "-iau") == 0) {
				throw ErrorGenericCommandLine(_("user-specific installation deprecated"));
			}
			else if (strcmp(argv[argcCur], "-icu") == 0) {
				throw ErrorGenericCommandLine(_("user-specific installation deprecated"));
				return;
			}
			else if (strcmp(argv[argcCur], "-ifu") == 0) {
				throw ErrorGenericCommandLine(_("user-specific installation deprecated"));
				return;
			}
			else if (strcmp(argv[argcCur], "-ias") == 0) {
				LoadAllConfig(true);
				if (argcLeft == 2)
					for (configMap::iterator i = allConfig.begin(); i != allConfig.end(); ++i) {
						if ((*i).first != "*")
						cout << "alias " << (*i).first << "=\'. " << argv[argcCur+1] << "/kcdscr -p "
						     << (*i).first << "\';\n";
					}
				else
					throw ErrorGenericCommandLine(_("one argument required for `-ias\'"));
				return;
			}
			else if (strcmp(argv[argcCur], "-ics") == 0) {
				LoadAllConfig(true);
				if (argcLeft == 2)
					for (configMap::iterator i = allConfig.begin(); i != allConfig.end(); ++i) {
						if ((*i).first != "*")
						cout << "alias " << (*i).first << " \'source " << argv[argcCur+1] << "/kcdscr -p "
						     << (*i).first << "\';\n";
					}
				else
					throw ErrorGenericCommandLine(_("one argument required for `-ics\'"));
				return;
			}
			else if (strcmp(argv[argcCur], "-ifs") == 0) {
				LoadAllConfig(true);
				if (argcLeft == 2)
					for (configMap::iterator i = allConfig.begin(); i != allConfig.end(); ++i) {
						if ((*i).first != "*")
						cout << (*i).first << " () { . " << argv[argcCur+1] << "/kcdscr -p "
						     << (*i).first << " \"$@\" ; };\n";
					}
				else
					throw ErrorGenericCommandLine(_("one argument required for `-ifs\'"));
				return;
			}
						// Profile
			else if (strcmp(argv[argcCur], "-p") == 0) {
				if (argcCur != 1)
					throw ErrorGenericCommandLine(_("`-p\' must be the first option used"));
				else if (argcLeft >= 2) {
					if (strcmp(argv[argcCur+1], "*") == 0)
						throw ErrorGenericCommandLine(_("`*\' profile not allowed in command line"));
					SetCurrentProfile(argv[argcCur+1]);
				}
				else
					throw ErrorGenericCommandLine(_("one argument required for `-p\'"));
						// Consume argument of -rp
				argcCur++;
				argcLeft--;
			}
						// Show bookmark
			else if (strcmp(argv[argcCur], "-b") == 0) {
				SetSaveFile();

				if (argcLeft >= 3)
					throw ErrorGenericCommandLine(_("too many arguments for `-b\'"));
				else if (argcLeft == 2)
					ShowBookmarks(argv[argcCur+1]);
				else
					ShowBookmarks("");
				return;
			}
						// Add bookmark
			else if (strcmp(argv[argcCur], "-ba") == 0) {
				SetSaveFile();
				AddBookmarks(argc, argv, argcCur+1);
				return;
			}
						// Remove bookmark
			else if (strcmp(argv[argcCur], "-br") == 0) {
				SetSaveFile();
				RemoveBookmarks(argc, argv, argcCur+1);
				return;
			}
						// Use entire tree
			else if (strcmp(argv[argcCur], "-B") == 0) {
				entire_tree = true;
			}
						// Use entire tree
			else if (strcmp(argv[argcCur], "-M") == 0) {
				use_mount_dir = false;
			}
						// Display tree
			else if (strcmp(argv[argcCur], "-t") == 0) {
				SetSaveFile();

				if (argcLeft >= 3)
					throw ErrorGenericCommandLine(_("too many arguments for `-t\'"));
				else if (argcLeft == 2) {
					LoadDirFile(true);
					DisplayTree(argv[argcCur+1], use_mount_dir);
				}
				else {
					LoadDirFile(true);
					DisplayTree("", use_mount_dir);
				}
				return;
			}
						// Display tree with rescan
			else if (strcmp(argv[argcCur], "-tr") == 0) {
				LoadAllConfig(false);

				SetSaveFile();

				if (argcLeft >= 3)
					throw ErrorGenericCommandLine(_("too many arguments for `-tr\'"));
				else if (argcLeft == 2) {
					LoadDirFile(true);
					DisplayTree(argv[argcCur+1], use_mount_dir, true);
				}
				else {
					LoadDirFile(true);
					DisplayTree("", use_mount_dir, true);
				}
				return;
			}
						// Display full tree
			else if (strcmp(argv[argcCur], "-T") == 0) {
				SetSaveFile();

				if (argcLeft >= 2)
					throw ErrorGenericCommandLine(_("argument not allowed for `-T\'"));
				LoadDirFile(true);
				DisplayFullTree(use_mount_dir);
				return;
			}
						// Rescan dir.
			else if (strncmp(argv[argcCur], "-r", 2) == 0) {
				rescan = true;

				bool	use_partial_dir = false;

				LoadAllConfig(false);

				for (char *p = argv[argcCur]+2; *p; ++p) {
					switch (*p) {
						case 'q':
							kcdConfig.ForceQuiet(true);
							break;
						case 'Q':
							kcdConfig.ForceQuiet(false);
							break;
						case 'f':
							forceFullScan = true;
							break;
						case 'p':
							if (argcLeft < 2) {
								throw ErrorGenericCommandLine(_("argument required for `-rp\'"));
							}
							else if (use_partial_dir) {
								throw ErrorGenericCommandLine(_("`-rpp\' not allowed"));
							}
							else if (strlen(argv[argcCur+1]) == 0) {
								throw ErrorGenericCommandLine(_("argument for `-rp\' cannot be empty"));
							}
							else if (saveCwd.size() == 0 && partial_dir.size()) {
								throw ErrorGenericCommandLine(_("multiple `-rp\' not allowed when current directory cannot be determined"));
							}
							partial_dir.push_back(argv[argcCur+1]);
							use_partial_dir = true;
							break;
						default:
							throw ErrorGenericCommandLine(_("unknown specifier for `-r\' option"));
					}
				}
				if (use_partial_dir) {	// Consume argument of -rp
					argcCur++;
					argcLeft--;
				}
			}
			else if (argv[argcCur][0] == '-') {	// Unrecognized option
				throw ErrorGenericCommandLine(_("unknown option"));
			}
			else {				// Not an option, must be a directory name
				SetSaveFile();

				if (argcLeft == 1 && rescan == false)
					CD(argv[argcCur], entire_tree, use_mount_dir);
				else if (argcLeft == 2 && rescan == false) {
					try {
						size_t u = StringToUnsigned(argv[argcCur+1], 0);
						if (u == 0)
							throw ErrorRange();
						CD(argv[argcCur], entire_tree, use_mount_dir, u);
					}
					catch (ErrorRange &) {
						throw ErrorGenericCommandLine(_("invalid match number"));
					}
				}
				else			// Too many arguments
					throw ErrorGenericCommandLine(_("too many arguments"));
				return;
			}

			argcCur++;
			argcLeft--;
		}

		if (rescan) {
			SetSaveFile();

			if (partial_dir.size())
				PartialScanDir(partial_dir, forceFullScan);
			else
				ScanDir(forceFullScan);
		}
		else {					// Consumed all option but
							// no action specified yet
			SetSaveFile();

			CD("", entire_tree, use_mount_dir);		// Show entire tree
		}
	}
}

int	main(int argc, char *argv[])
{
	try {
		// Work around for gcc EH optimization bugs
		mainReal(argc, argv);
	}
	catch(bad_alloc &) {
		cout << flush;			// Avoid out-of-order display
		cerr << progName << _(": not enough memory\n");
		return RET_MEMERR;
	}
	catch(ErrorBase &e) {
		cout << flush;			// Avoid out-of-order display
		cerr << progName << ": " << e.what() << '\n';
		return e.ret();
	}
	catch(...) {
		cout << flush;			// Avoid out-of-order display
		cerr << progName << _(": unknown exception\n");
		return RET_UNKERR;
	}
}
