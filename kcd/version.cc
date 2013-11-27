//bin/true; VERSION="7.15.0"; return 0
// Above magic is for configure to get version number
// Generate automatically from scripts/versions.cc.src
char	version[] = "7.15.0";

// Below magic is for scripts/makepkg, scripts/makepkg-release
#define HAVE_RELEASE_DATE
#ifdef HAVE_RELEASE_DATE
int	rel_day = 18;
int	rel_month = 2 -1;
int	rel_year = 2009 -1900;
#else
int	rel_day = 1;
int	rel_month = 0;
int	rel_year = 100;
#endif
