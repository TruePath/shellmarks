Summary: Directory change program
Name: kcd
Version: 7.15.0
Release: 1
Copyright: GPL
Group: Applications/System
Source: http://download.sourceforge.net/kcd/kcd-7.15.0.tar.gz
URL: http://kcd.sourceforge.net
Packager: Kriang Lerdsuwanakij <lerdsuwa@users.sourceforge.net>
BuildRoot: /var/tmp/%{name}-buildroot
Prefix: %{_prefix}

%description
kcd allows user to change directory by using various methods:
- By using cursor keys to navigate through the directory tree 
  screen
- By searching in directory tree screen
- By typing part of the directory name directly at the 
  command line.

Install kcd if you'd like to have a tool to navigate and explore
the directory tree quickly in the command line.

%prep
%setup

%build
export RPM_OPT_FLAGS=`echo $RPM_OPT_FLAGS | sed 's/-fno-exceptions//;s/-fno-rtti//'`
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" \
  ./configure --prefix=%{_prefix} --datadir=%{_datadir} --mandir=%{_mandir}
make

%install
if test "$RPM_BUILD_ROOT" != /; then
	rm -rf "$RPM_BUILD_ROOT"
fi
mkdir -p "$RPM_BUILD_ROOT"/etc/profile.d
make DESTDIR="$RPM_BUILD_ROOT"/                 \
     prefix="$RPM_BUILD_ROOT"%{_prefix}         \
     etcdir="$RPM_BUILD_ROOT"/etc               \
     mandir="$RPM_BUILD_ROOT"%{_mandir} install

%clean
if test "$RPM_BUILD_ROOT" != /; then
	rm -rf "$RPM_BUILD_ROOT"
fi

%files
%defattr(-,root,root)
%doc README NEWS COPYING FILELIST INSTALL TODO BUGREPORT UNICODE RPM
%{_prefix}/bin/kcdscr
%{_prefix}/bin/kcdmain
%{_prefix}/bin/kcd-inst
%{_mandir}/man1/kcd.1*
%{_datadir}/locale/
/etc/profile.d/kcd.sh
/etc/profile.d/kcd.csh

%changelog
