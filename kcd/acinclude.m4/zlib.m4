dnl *****************************************************************
dnl  zlib Tests for Autoconf
dnl  Version 0.2.0
dnl  (c) 2000, 2002 by Kriang Lerdsuwanakij
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
dnl  As a special exception, the file `configure' generated by
dnl  `autoconf' from this file is not covered by the GNU General Public 
dnl  License.  This exception does not however invalidate any other
dnl  reasons why the generated file might be covered by the GNU General 
dnl  Public License.
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl

dnl *****************************************************************
dnl ZLIBAC_EXTERN_C	Test if extern "C" is required to build
dnl			C++ programs.
dnl
dnl	Results:	zlibac_cv_need_extern_c and ZLIBAC_NEED_EXTERN_C
dnl			defined if extern "C" is required.
dnl
AC_DEFUN([ZLIBAC_EXTERN_C], [
  AC_LANG_PUSH([C++])
  AC_MSG_CHECKING([whether zlib requires extern "C" for C++ programs])
  AH_TEMPLATE([ZLIBAC_NEED_EXTERN_C],
	      [Define if extern "C" is required for including <zlib.h> in
	       C++ programs])
  AC_CACHE_VAL(zlibac_cv_need_extern_c, [
	AC_TRY_LINK([
		#include <zlib.h>
	], [
		gzFile gz = gzopen("test", "r");
		gzclose(gz);
	], [
	  zlibac_cv_need_extern_c=no
	], [
	  zlibac_cv_need_extern_c=yes
	])
  ])
  AC_MSG_RESULT($zlibac_cv_need_extern_c)
  if test $zlibac_cv_need_extern_c = yes; then
    AC_DEFINE(ZLIBAC_NEED_EXTERN_C)
  fi

  AC_LANG_POP([C++])
])

dnl *****************************************************************
dnl ZLIBAC_GZ_FUNC	Test for zlib gz* functions
dnl
dnl	Results:	zlibac_cv_have_* and ZLIBAC_HAVE_*
dnl			defined if the corresponding function is 
dnl			available.
dnl
AC_DEFUN([ZLIBAC_GZ_FUNC], [
  AC_LANG_PUSH([C])

  AC_CHECK_FUNC(gzprintf, zlibac_cv_have_gzprintf=yes, zlibac_cv_have_gzprintf=no)
  AH_TEMPLATE([ZLIBAC_HAVE_GZPRINTF], [Define if gzprintf is available.])
  if test $zlibac_cv_have_gzprintf = yes; then
    AC_DEFINE(ZLIBAC_HAVE_GZPRINTF)
  fi

  AC_CHECK_FUNC(gzputc, zlibac_cv_have_gzputc=yes, zlibac_cv_have_gzputc=no)
  AH_TEMPLATE([ZLIBAC_HAVE_GZPUTC], [Define if gzputc is available.])
  if test $zlibac_cv_have_gzputc = yes; then
    AC_DEFINE(ZLIBAC_HAVE_GZPUTC)
  fi

  AC_CHECK_FUNC(gzgetc, zlibac_cv_have_gzgetc=yes, zlibac_cv_have_gzgetc=no)
  AH_TEMPLATE([ZLIBAC_HAVE_GZGETC], [Define if gzgetc is available.])
  if test $zlibac_cv_have_gzgetc = yes; then
    AC_DEFINE(ZLIBAC_HAVE_GZGETC)
  fi

  AC_CHECK_FUNC(gzputs, zlibac_cv_have_gzputs=yes, zlibac_cv_have_gzputs=no)
  AH_TEMPLATE([ZLIBAC_HAVE_GZPUTS], [Define if gzputs is available.])
  if test $zlibac_cv_have_gzputs = yes; then
    AC_DEFINE(ZLIBAC_HAVE_GZPUTS)
  fi

  AC_CHECK_FUNC(gzgets, zlibac_cv_have_gzgets=yes, zlibac_cv_have_gzgets=no)
  AH_TEMPLATE([ZLIBAC_HAVE_GZGETS], [Define if gzgets is available.])
  if test $zlibac_cv_have_gzgets = yes; then
    AC_DEFINE(ZLIBAC_HAVE_GZGETS)
  fi

  AC_CHECK_FUNC(gzeof, zlibac_cv_have_gzeof=yes, zlibac_cv_have_gzeof=no)
  AH_TEMPLATE([ZLIBAC_HAVE_GZEOF], [Define if gzeof is available.])
  if test $zlibac_cv_have_gzeof = yes; then
    AC_DEFINE(ZLIBAC_HAVE_GZEOF)
  fi

  AC_LANG_POP([C])
])


dnl *****************************************************************
dnl  End of zlib Tests for Autoconf
dnl *****************************************************************