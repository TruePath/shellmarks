dnl *****************************************************************
dnl  configure.in for kcd
dnl  (c) 1997-8,1999,2000,2001,2002,2003,2004,2007,2009 by Kriang Lerdsuwanakij
dnl  lerdsuwa@users.sourceforge.net
dnl
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 2 of the License, or
dnl  (at your option) any later version.
dnl
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl
dnl *****************************************************************
dnl Process this file with autoconf to produce a configure script.
AC_REVISION($Revision: 1.27 $)
AC_PREREQ(1.13)
AC_INIT
AC_CONFIG_SRCDIR([kcdmain.cc])

dnl *****************************************************************
dnl  Initialization
dnl *****************************************************************

PACKAGE=kcd
dnl Get VERSION
. $srcdir/version.cc
AH_TEMPLATE([PACKAGE],
	    [Define to the name of the distribution.])
AH_TEMPLATE([VERSION],
	    [Define to the version of the distribution.])
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

ALL_LINGUAS="en_US.UTF-8 en_GB en_GB.UTF-8"
LINGUAS="$ALL_LINGUAS"

top_builddir=`pwd`
AC_SUBST(top_builddir)

dnl *****************************************************************
dnl  Set header output file
dnl *****************************************************************
AC_CONFIG_HEADER(config.h)
AC_CONFIG_AUX_DIR(scripts)

dnl *****************************************************************
dnl  Preliminary check
dnl *****************************************************************
dnl
dnl Script below assumes that $bindir = ${exec_prefix}/bin and
dnl 	${prefix} = ${exec_prefix}
dnl Display error message if $bindir is redefined
dnl
if test ! "$bindir" = '${exec_prefix}/bin'; then
	{ _AS_ECHO([configure: error: --bindir option not supported in this script],[2]); exit 1; }
fi
if test ! "$exec_prefix" = NONE; then
	{ _AS_ECHO([configure: error: --exec-prefix option not supported in this script],[2]); exit 1; }
fi

dnl *****************************************************************
dnl  Checks for programs
dnl *****************************************************************
AC_PROG_CC
AC_ISC_POSIX
AC_PROG_CC_STDC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP

dnl Add -W -Wall (produce all warnings) if gcc is used
if test "$GXX" = yes; then
	CFLAGS="$CFLAGS -W -Wall"
	CXXFLAGS="$CXXFLAGS -W -Wall"
fi

AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_LANG([C++])

dnl *****************************************************************
dnl  Checks for C++ features
dnl *****************************************************************

CXXAC_TYPE_BOOL
if test "$cxxac_cv_have_bool" = no; then
	AC_MSG_ERROR(C++ compiler that supports bool is required)
fi
CXXAC_SIZE_BOOL
CXXAC_DEFAULT_EXCEPTION
CXXAC_HAVE_TYPENAME
CXXAC_NO_GUIDE_DECL

dnl *****************************************************************
dnl  Checks for libraries
dnl *****************************************************************

kWIDE_CHAR=no
kSTATICncurseslib="libncurses.a"
kDYNAMICncurseslib="-lncurses"
kNAMEncurseslib="ncurses"
kWARNncurseslib=""
kNAMEncursesincludedir="ncurses"
AH_TEMPLATE([NCURSESAC_WIDE],
	    [Define if wide char support of ncurses is used.])
AC_ARG_ENABLE(wide-chars,
	[  --enable-wide-chars     include wide char support],
	[	if test "$enableval" = "yes"; then
			kWIDE_CHAR="yes"
			kSTATICncurseslib="libncursesw.a"
			kDYNAMICncurseslib="-lncursesw"
			kNAMEncurseslib="ncursesw"
			kWARNncurseslib=".
Read UNICODE file for system requirement for --enable-wide-chars option"
			kNAMEncursesincludedir="ncursesw"
			AC_DEFINE_UNQUOTED(NCURSESAC_WIDE, "1")
			AC_SUBST(NCURSESAC_WIDE)
		elif test "$enableval" = "no"; then
			kWIDE_CHAR="no"
			kSTATICncurseslib="libncurses.a"
			kDYNAMICncurseslib="-lncurses"
			kNAMEncurseslib="ncurses"
			kWARNncurseslib=""
			kNAMEncursesincludedir="ncurses"
		else
			{ _AS_ECHO([configure: error: invalid argument for --enable-wide-chars],[2]); exit 1; }
		fi
	])

kSTATICncurses="no";
kSTATICDIRncurses="";
AC_ARG_WITH(static-ncurses,
	[  --with-static-ncurses   link with ncurses statically],
	[	if test "$withval" = "yes"; then
			kSTATICncurses="yes"
		fi
	])

if test "x$with_static_ncurses" != "x"; then
	if test "$with_static_ncurses" != "yes"; then
		if test "$with_static_ncurses" != "no"; then
			kSTATICncurses="yes"
			kSTATICDIRncurses="$with_static_ncurses"
		fi
	fi
fi

kPUTLIBncurses="";
kMSGLIBncurses="";

dnl
dnl Check for local libraries in ../lib
dnl

dnl Check for ../lib/libncurses.a or ../lib/libncursesw.a
AC_MSG_NOTICE(checking local copy of ncurses library file)
kHAVE_LOCALLIBncurses=no
AC_CHECK_FILE(../lib/"$kSTATICncurseslib", kHAVE_LOCALLIBncurses=yes)
if test "$kHAVE_LOCALLIBncurses" = yes; then
	LDFLAGS="$LDFLAGS -Wl,-L../lib"
	kHAVE_SHAREDLIBncurses=no
	kMSGLIBncurses="../lib/$kSTATICncurseslib"
fi

dnl Check for ../lib/libz.a
AC_MSG_NOTICE(checking local copy of zlib library file)
kHAVE_LOCALLIBz=no
AC_CHECK_FILE(../lib/libz.a, kHAVE_LOCALLIBz=yes)
if test "$kHAVE_LOCALLIBz" = yes; then
	if test "$kHAVE_LOCALLIBncurses" = no; then
		LDFLAGS="$LDFLAGS -Wl,-L../lib"
	fi
	kHAVE_SHAREDLIBz=no
fi

dnl
dnl Check for global libraries if local libraries are not found in ../lib
dnl

if test "$kSTATICncurses" = yes; then

	if test -z "$kSTATICDIRncurses"; then
		AC_MSG_NOTICE(checking global copy of static ncurses library in /usr/local/lib)
		kHAVE_USRLOCALLIBncurses=no
		AC_CHECK_FILE(/usr/local/lib/"$kSTATICncurseslib", kHAVE_USRLOCALLIBncurses=yes)
		if test "$kHAVE_USRLOCALLIBncurses" = yes; then
			LIBS="$LIBS /usr/local/lib/$kSTATICncurseslib"
			kPUTLIBncurses="yes"
			kMSGLIBncurses="/usr/local/lib/$kSTATICncurseslib"
		else
			AC_MSG_NOTICE(checking global copy of static ncurses library in /usr/lib)
			kHAVE_USRLIBncurses=no
			AC_CHECK_FILE(/usr/lib/"$kSTATICncurseslib", kHAVE_USRLIBncurses=yes)
			if test "$kHAVE_USRLIBncurses" = yes; then
				LIBS="$LIBS /usr/lib/$kSTATICncurseslib"
				kPUTLIBncurses="yes"
				kMSGLIBncurses="/usr/lib/$kSTATICncurseslib"
			else
				AC_MSG_NOTICE(checking global copy of static ncurses library in /lib)
				kHAVE_LIBncurses=no
				AC_CHECK_FILE(/lib/"$kSTATICncurseslib", kHAVE_USRLIBncurses=yes)
				if test "$kHAVE_LIBncurses" = yes; then
					LIBS="$LIBS /lib/$kSTATICncurseslib"
					kPUTLIBncurses="yes"
					kMSGLIBncurses="/lib/$kSTATICncurseslib"
				else
					AC_MSG_ERROR(cannot find static ncurses library$kWARNncurseslib)
				fi
			fi
		fi
	else
		AC_MSG_NOTICE(checking global copy of static ncurses library in $kSTATICDIRncurses)
		kHAVE_FROMARGLIBncurses=no
		AC_CHECK_FILE("$kSTATICDIRncurses"/"$kSTATICncurseslib", kHAVE_FROMARGLIBncurses=yes)
		if test "$kHAVE_FROMARGLIBncurses" = yes; then
			LIBS="$LIBS $kSTATICDIRncurses/$kSTATICncurseslib"
			kPUTLIBncurses="yes"
			kMSGLIBncurses="$kSTATICDIRncurses/$kSTATICncurseslib"
		else
			AC_MSG_ERROR(cannot find $kSTATICncurseslib in ${kSTATICDIRncurses}$kWARNncurseslib)
		fi
	fi
else

	dnl Check for a function in -lncurses or -lncursesw
	if test "$kHAVE_LOCALLIBncurses" = no; then
		AC_MSG_NOTICE(checking global copy of ncurses library)
		AC_CHECK_LIB($kNAMEncurseslib, initscr, kHAVE_SHAREDLIBncurses=yes,\
			kHAVE_SHAREDLIBncurses=no)
		if test "$kHAVE_SHAREDLIBncurses" = no; then
			AC_MSG_ERROR(cannot find ncurses library file$kWARNncurseslib)
		fi
	fi
fi

dnl Check for a function in -lz
if test "$kHAVE_LOCALLIBz" = no; then
	AC_MSG_NOTICE(checking global copy of zlib library)
	AC_CHECK_LIB(z, gzopen, kHAVE_SHAREDLIBz=yes, kHAVE_SHAREDLIBz=no)
	if test "$kHAVE_SHAREDLIBz" = no; then
		AC_MSG_ERROR(cannot find zlib library file)
	fi
fi

dnl
dnl We now have both libraries checked
dnl

if test -z "$kMSGLIBncurses"; then
	kMSGLIBncurses="$kDYNAMICncurseslib"
fi

if test -z "$kPUTLIBncurses"; then
	LIBS="$LIBS $kDYNAMICncurseslib"
fi
LIBS="$LIBS -lz"

dnl *****************************************************************
dnl  Checks for header files
dnl *****************************************************************
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS(fcntl.h limits.h malloc.h sys/ioctl.h unistd.h string.h)
AC_CHECK_HEADERS(wchar.h)
AC_CHECK_HEADER(regex.h, kHAVE_HEADERregex=yes, kHAVE_HEADERregex=no)

dnl Delete created curses.h to avoid confusion in tests
rm -f curses.h

dnl
dnl Check for local headers in ../include
dnl

dnl Check for ../include/curses.h
AC_MSG_NOTICE(checking local copy of ncurses header files)
kHAVE_LOCALHEADERncurses=no
AC_CHECK_FILE(../include/curses.h, kHAVE_LOCALHEADERncurses=yes)
if test "$kHAVE_LOCALHEADERncurses" = yes; then
	CPPFLAGS="$CPPFLAGS -I../include"
	kHAVE_SHAREDHEADERncurses=no
fi

dnl Check for ../include/zlib.h
AC_MSG_NOTICE(checking local copy of zlib header file)
kHAVE_LOCALHEADERz=no
AC_CHECK_FILE(../include/zlib.h, kHAVE_LOCALHEADERz=yes)
if test "$kHAVE_LOCALHEADERz" = yes; then
	if test "$kHAVE_LOCALHEADERncurses" = no; then
		CPPFLAGS="$CPPFLAGS -I../include"
	fi
	kHAVE_SHAREDHEADERz=no
fi

dnl
dnl Check for global headers if local headers are not found in ../include
dnl

dnl Check for curses.h
AH_TEMPLATE([NCURSESAC_NEW_INCLUDE],
	    [Define if curses.h is in ncurses and ncursesw subdirectory.])
AH_TEMPLATE([NCURSESAC_INCLUDE],
	    [Define to curses.h header.])
kNCURSES_HEADER=curses.h
if test "$kHAVE_LOCALHEADERncurses" = no; then
	AC_MSG_NOTICE(checking global copy of ncurses header files)
	AC_CHECK_HEADER($kNAMEncursesincludedir/curses.h,\
			kHAVE_SHAREDHEADERncurses=yes,\
			kHAVE_SHAREDHEADERncurses=no)
	if test "$kHAVE_SHAREDHEADERncurses" = no; then
		AC_CHECK_HEADER(curses.h, kHAVE_SHAREDHEADERncurses=yes,\
				kHAVE_SHAREDHEADERncurses=no)
		if test "$kHAVE_SHAREDHEADERncurses" = no; then
			AC_MSG_ERROR(cannot find ncurses header file)
		fi
	else
		kNCURSES_HEADER="$kNAMEncursesincludedir/curses.h"
		AC_DEFINE_UNQUOTED(NCURSESAC_NEW_INCLUDE, "1")
		AC_SUBST(NCURSESAC_NEW_INCLUDE)
	fi
fi
AC_DEFINE_UNQUOTED(NCURSESAC_INCLUDE, "$kNCURSES_HEADER")
AC_SUBST(NCURSESAC_INCLUDE)

dnl Check for zlib.h
if test "$kHAVE_LOCALHEADERz" = no; then
	AC_MSG_NOTICE(checking global copy of zlib header file)
	AC_CHECK_HEADER(zlib.h, kHAVE_SHAREDHEADERz=yes, \
			kHAVE_SHAREDHEADERz=no)
	if test "$kHAVE_SHAREDHEADERz" = no; then
		AC_MSG_ERROR(cannot find zlib header file)
	fi
fi

dnl Add current directory for config.h
dnl The header file in current directory override global ones
dnl in case of ncurses bool work around
CPPFLAGS="-I. $CPPFLAGS"

dnl
dnl Check for C++ headers
dnl
CXXAC_HEADER_ALL

dnl *****************************************************************
dnl  Checks for typedefs, structures, and compiler characteristics
dnl *****************************************************************
AC_TYPE_SIZE_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_UID_T

dnl
dnl Check for C++ library features
dnl
CXXAC_LIB_NEED_USING_STD
CXXAC_STRING_ERASE
CXXAC_STRING_IO

if test "$cxxac_cv_default_exception" != yes; then
	AC_MSG_ERROR(exception handling not available)
fi

dnl *****************************************************************
dnl  Checks for library functions
dnl *****************************************************************

dnl -----------------------------------------------------------------
dnl  C library tests
dnl -----------------------------------------------------------------

dnl strftime is defined when including <stdlib.h> on Solaris 8.
dnl This confuses configure when tested using C++ language.
dnl The same applies to mbtowc, etc. on glibc.

AC_LANG([C])

AC_FUNC_CLOSEDIR_VOID

dnl FIXME: We need to check the result of this
AC_FUNC_FNMATCH

AC_FUNC_STRFTIME
if test "$ac_cv_func_strftime" != yes; then
	{ _AS_ECHO([configure: error: strftime is not available],[2]); exit 1; }
fi

AH_TEMPLATE([CLIB_HAVE_STRCASECMP],
	    [Define if you have strcasecmp in <string.h> header file.])
AH_TEMPLATE([CLIB_HAVE_STRNCASECMP],
	    [Define if you have strncasecmp in <string.h> header file.])
AH_TEMPLATE([CLIB_HAVE_WCWIDTH],
	    [Define if you have wcwidth in <wchar.h> header file.])
AH_TEMPLATE([CLIB_HAVE_ISWPRINT],
	    [Define if you have iswprint in <wctype.h> header file.])
AH_TEMPLATE([CLIB_HAVE_MBTOWC],
	    [Define if you have mbtowc in some header file.])
AH_TEMPLATE([CLIB_HAVE_MBRTOWC],
	    [Define if you have mbrtowc in some header file.])
AH_TEMPLATE([CLIB_HAVE_MBSTOWCS],
	    [Define if you have mbstowcs in some header file.])
AH_TEMPLATE([CLIB_HAVE_MBSRTOWCS],
	    [Define if you have mbsrtowcs in some header file.])
AH_TEMPLATE([CLIB_HAVE_REGEX],
	    [Define if you have regex library.])
AH_VERBATIM([CLIB_HAVE_WCWIDTH2], [
#if defined(CLIB_HAVE_WCWIDTH) && defined(CLIB_HAVE_ISWPRINT) \
    && defined(CLIB_HAVE_MBTOWC) && defined(CLIB_HAVE_MBRTOWC) \
    && defined(CLIB_HAVE_MBSTOWCS) && defined(CLIB_HAVE_MBSRTOWCS)
# define USE_UTF8_MODE
#endif
])

AC_CHECK_FUNC(strcasecmp, AC_DEFINE(CLIB_HAVE_STRCASECMP))
AC_CHECK_FUNC(strncasecmp, AC_DEFINE(CLIB_HAVE_STRNCASECMP))

if test "$kWIDE_CHAR" = "yes"; then
	AC_CHECK_FUNC(wcwidth, AC_DEFINE(CLIB_HAVE_WCWIDTH))
	AC_CHECK_FUNC(iswprint, AC_DEFINE(CLIB_HAVE_ISWPRINT))
	AC_CHECK_FUNC(mbtowc, AC_DEFINE(CLIB_HAVE_MBTOWC))
	AC_CHECK_FUNC(mbrtowc, AC_DEFINE(CLIB_HAVE_MBRTOWC))
	AC_CHECK_FUNC(mbstowcs, AC_DEFINE(CLIB_HAVE_MBSTOWCS))
	AC_CHECK_FUNC(mbsrtowcs, AC_DEFINE(CLIB_HAVE_MBSRTOWCS))
fi

kHAVE_LIBregex=yes;
AC_CHECK_FUNC(regcomp, [], kHAVE_LIBregex=no)
AC_CHECK_FUNC(regexec, [], kHAVE_LIBregex=no)
AC_CHECK_FUNC(regerror, [], kHAVE_LIBregex=no)
AC_CHECK_FUNC(regfree, [], kHAVE_LIBregex=no)
if test "$kHAVE_HEADERregex" = yes -a "$kHAVE_LIBregex" = yes; then
	AC_DEFINE(CLIB_HAVE_REGEX)
fi

dnl Test dirent::d_type
AC_MSG_CHECKING([for dirent::d_type])
AC_CACHE_VAL(clib_have_d_type, [
  AC_TRY_RUN([
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #include <stdio.h>
    #include <errno.h>
    #include <unistd.h>

    int main()
    {
      DIR *d;
      FILE *f;
      struct dirent *dir_buf;
      struct stat stat_buf;

      f = fdopen(5, "a");	/* Handle 5 corresponds to config.log output */
      if (!f) {
	return 1;
      }

      d = opendir(".");
      if (!d) {
	fprintf(f, "d_type test: opendir fails %d\n", errno);
	fclose(f);
	return 1;
      }

      while ((dir_buf = readdir(d)) != 0) {

	if (dir_buf->d_type != DT_UNKNOWN) {

	  if (lstat(dir_buf->d_name, &stat_buf)) {
	    fprintf(f, "d_type test: stat fails %d\n", errno);
	    fclose(f);
	    return 1;
	  }

	  if ((dir_buf->d_type == DT_DIR) ^ S_ISDIR(stat_buf.st_mode)) {
	    fprintf(f, "d_type test: DT_DIR conflict %s %d %d\n",
		    dir_buf->d_name, (int)dir_buf->d_type,
		    (int)stat_buf.st_mode);
	    fclose(f);
	    return 1;
	  }

	  if ((dir_buf->d_type == DT_REG) ^ S_ISREG(stat_buf.st_mode)) {
	    fprintf(f, "d_type test: DT_REG conflict %s %d %d\n",
		    dir_buf->d_name, (int)dir_buf->d_type,
		    (int)stat_buf.st_mode);
	    fclose(f);
	    return 1;
	  }

	  if ((dir_buf->d_type == DT_LNK) ^ S_ISLNK(stat_buf.st_mode)) {
	    fprintf(f, "d_type test: DT_LNK conflict %s %d %d\n",
		    dir_buf->d_name, (int)dir_buf->d_type,
		    (int)stat_buf.st_mode);
	    fclose(f);
	    return 1;
	  }
	}
      }
    }
  ], [
    clib_have_d_type="yes"
  ], [
    clib_have_d_type="no"
  ], [
    clib_have_d_type="unknown due to cross-compilation"
  ])
])
AC_MSG_RESULT($clib_have_d_type)
AH_TEMPLATE([CLIB_HAVE_D_TYPE],
	    [Define if you have d_type in struct dirent.])
if test "$clib_have_d_type" = yes; then
	AC_DEFINE(CLIB_HAVE_D_TYPE)
fi

AC_LANG([C++])

dnl *****************************************************************
dnl  I18n, l10n
dnl *****************************************************************

AM_GNU_GETTEXT([external])

dnl -----------------------------------------------------------------
dnl  ncurses library tests
dnl -----------------------------------------------------------------

dnl Must use local library, headers in these tests
AC_CHECK_FUNC(resizeterm, kHAVE_resizeterm=yes, kHAVE_resizeterm=no)
AH_TEMPLATE([HAVE_RESIZETERM],
	    [Define if you have resizeterm in <curses.h> header file.])
if test "$kHAVE_resizeterm" = yes; then
	AC_DEFINE(HAVE_RESIZETERM)
else
	AC_MSG_WARN(window resize under xterm and rxvt disabled)
fi

k_MOUSE=yes
AH_TEMPLATE([NCURSESAC_ENABLE_MOUSE],
	    [Define if you enable mouse support in configure.])
AC_ARG_ENABLE(mouse,
	[  --disable-mouse         do not response to mouse clicks],
	[	if test "$enableval" = "yes"; then
			k_MOUSE=yes;
		elif test "$enableval" = "no"; then
			k_MOUSE=no;
		else
			{ _AS_ECHO([configure: error: invalid argument for --enable-mouse],[2]); exit 1; }
		fi
	])
if test "$k_MOUSE" = yes; then
	AC_DEFINE(NCURSESAC_ENABLE_MOUSE)
fi

NCURSESAC_MOUSEMASK
NCURSESAC_NEW_ATTR

dnl -----------------------------------------------------------------
dnl  zlib library tests
dnl -----------------------------------------------------------------

ZLIBAC_EXTERN_C
ZLIBAC_GZ_FUNC

dnl *****************************************************************
dnl  Final sanity checks
dnl *****************************************************************

dnl -----------------------------------------------------------------
dnl  Check if ncurses is compiled with C++ support
dnl -----------------------------------------------------------------

NCURSESAC_BOOL
if test "$cxxac_cv_bool_size" != "$ncursesac_cv_c_bool_size"; then
	echo "C++ bool size is $cxxac_cv_bool_size but ncurses was built with bool size $ncursesac_cv_c_bool_size"
	AC_MSG_WARN(ncurses is not built to support C++)
	AC_MSG_WARN(kcd may not be built properly in this case)

	header_dir=`perl $srcdir/scripts/locateheader.pl $CPP $CPPFLAGS curses.h`
	if test "$header_dir" = "<unknown>"; then
		AC_MSG_ERROR(cannot locate curses.h)
	else
		perl $srcdir/scripts/uglyboolhack.pl "$header_dir" ./curses.h
	fi
fi

dnl *****************************************************************
dnl  Set directory names
dnl *****************************************************************

dnl -----------------------------------------------------------------
dnl Checks for the location of previously installed kcd
dnl Let prefix=/usr if previous version was installed in /usr/bin
dnl otherwise use the default prefix (usually /usr/local/bin)
dnl -----------------------------------------------------------------

AC_CHECK_PROG(kATusrbin, kcdmain, yes, no, /usr/bin)
if test "$kATusrbin" = yes; then
	if test "x$prefix" = xNONE; then
		prefix=/usr
	fi
fi
if test "x$prefix" = xNONE; then
	prefix=$ac_default_prefix
fi

dnl -----------------------------------------------------------------
dnl Check whether FHS is requested by user
dnl -----------------------------------------------------------------

k_FHS=none
AC_ARG_ENABLE(fhs,
	[  --disable-fhs           use old directory structure],
	[	if test "$enableval" = "yes"; then
			k_FHS=yes;
		elif test "$enableval" = "no"; then
			k_FHS=no;
		else
			{ _AS_ECHO([configure: error: invalid argument for --enable-fhs],[2]); exit 1; }
		fi
	])

dnl -----------------------------------------------------------------
dnl Check current directory structure if FHS is not specified
dnl -----------------------------------------------------------------

if test "$k_FHS" = "none"; then
	AC_MSG_CHECKING([FHS])
	if test -d "/usr/share/man"; then
		k_FHS=yes
	else
		k_FHS=no
	fi
	AC_MSG_RESULT($k_FHS)
fi

dnl -----------------------------------------------------------------
dnl Set various directory names if they are not overrided by user
dnl -----------------------------------------------------------------

AH_TEMPLATE([HAVE_FHS],
	    [Define if FHS is used.])
if test "$k_FHS" = "yes"; then
	AC_DEFINE(HAVE_FHS)
	if test "$sysconfdir" = '${prefix}/etc'; then
		if echo "$prefix" | grep "^/opt/" > /dev/null; then
			sysconfdir="/etc$prefix"
		else
			sysconfdir=/etc
		fi
	fi

	if test "$mandir" = '${prefix}/man'; then
		if test -d "$prefix/share/man/en_US"; then
			mandir="$prefix/share/man/en_US"
		elif test -d "$prefix/share/man/en"; then
			mandir="$prefix/share/man/en"
		else
			mandir="$prefix/share/man"
		fi
	fi
else
	if test "$sysconfdir" = '${prefix}/etc'; then
		sysconfdir=/etc
	fi
fi

dnl *****************************************************************
dnl  Custom configuration macros
dnl *****************************************************************

AH_BOTTOM([
/* For gettext */
#include <libintl.h>
#define _(str)	gettext(str)
#define N_(str)	str

/* Disable features in khdoc.cc */
#define TRIM_NO_DOC_FILE
#define TRIM_NO_DOC_STREAM
#define TRIM_NO_HEADER
#define TRIM_NO_EXTRA_ATTRIB
#define TRIM_NO_INDENT
#define TRIM_NO_RULE
#define TRIM_NO_LATIN
#define TRIM_NO_NUM
#define TRIM_NO_CHECK
])

dnl *****************************************************************
dnl  Print configuration information
dnl *****************************************************************

echo "ncurses library used: $kMSGLIBncurses"
echo "ncurses mouse support: $ncursesac_cv_have_mousemask"

echo "kcd programs will be installed in $prefix/bin"
echo "system configuration dir: $sysconfdir"
echo "alternate initialization script dir: $prefix/etc"

dnl *****************************************************************
dnl  Done
dnl *****************************************************************

AC_CONFIG_FILES([Makefile kcd.sh.init kcd.csh.init kcd-inst po/Makefile.in])

AC_OUTPUT
