echo Configuring GNUWorld...
echo

AC_INIT([gnuworld],[1.0],[jeekay@netgamers.org])
AC_CONFIG_SRCDIR([src/main.cc])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_HEADERS([include/defs.h])

dnl Enable building of the convenience library
dnl and set LIBLTDL accordingly
dnl Configure libtool
LT_CONFIG_LTDL_DIR([libltdl])
LT_INIT([dlopen])
LTDL_INIT([convenience])

dnl Substitute INCLTDL and LIBLTDL in the Makefiles
dnl For some reason, INCLTDL and LIBLTDL aren't set
dnl properly on OpenBSD
INCLTDL="-I`pwd`/libltdl"
LIBLTDL="`pwd`/libltdl/libltdlc.la"

AC_SEARCH_LIBS([strerror],[cposix])

CFLAGS="$CFLAGS -Wall -pipe"
CXXFLAGS="$CXXFLAGS -Wall -pipe"

prefix="`pwd`"
libdir="`pwd`/bin"
glibdir="`pwd`/lib"

AC_PROG_CXX
AC_LANG([C++])

AC_PROG_MAKE_SET(gmake)

threadLib=no

dnl AC_CHECK_LIB(lthread, pthread_create, threadLib=-llthread)

if [test "$threadLib" = no]; then
	AC_CHECK_LIB(pthread, pthread_create, threadLib=-lpthread)
fi

if [test "$threadLib" = no]; then
	AC_CHECK_LIB(c, pthread_create, threadLib=-lc)
fi

if [test "$threadLib" = no]; then
	AC_CHECK_LIB(c_r, pthread_create, threadLib=-lc_r)
fi

if [test "$threadLib" = no]; then
	AC_MSG_RESULT([Unable to locate pthread_create in any library])
else
	LIBS="$LIBS $threadLib"
fi

dnl ----------------------------------------------------------
dnl ----------------------------------------------------------
dnl
dnl System specific checks
dnl
dnl ----------------------------------------------------------
dnl ----------------------------------------------------------

AC_CHECK_LIB(socket,socket)
AC_CHECK_LIB(nsl,gethostbyname)
dnl AC_CHECK_LIB(pthread,pthread_create)
dnl AC_CHECK_LIB(posix4,sem_init)
AC_SEARCH_LIBS(pthread_create,lthread pthread c c_r)
AC_SEARCH_LIBS(pthread_kill,lthread pthread c c_r)
AC_CHECK_FUNCS(pthread_create)

dnl ----------------------------------------------------------
dnl
dnl Checking for headers, functions, and a type declarations
dnl

AC_CHECK_HEADERS(sys/file.h unistd.h sys/stat.h sys/ioctl.h sys/resource.h )
AC_CHECK_HEADERS(sys/time.h asm/ioctls.h xti_inet.h sys/filio.h getopt.h)
AC_CHECK_HEADERS(sys/types.h sys/socket.h netinet/in.h sys/un.h pthread.h)

AC_HEADER_STDC
AC_HEADER_TIME

AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T

found_socklen_t=no
AC_EGREP_HEADER(socklen_t,sys/socket.h, AC_DEFINE(HAVE_SOCKLEN_T) found_socklen_t=yes)

if test $found_socklen_t = yes; then
  AC_EGREP_HEADER(socklen_t,bits/socket.h,AC_DEFINE(HAVE_SOCKLEN_T))
fi

if test $found_socklen_t = yes; then
  AC_MSG_CHECKING(for socklen_t)
  AC_MSG_RESULT(yes)
else
  AC_CHECK_TYPE(socklen_t,int)
fi

AC_ARG_WITH(debug,
	AS_HELP_STRING([--without-debug],[Disable debugging in the executables]),
	[dummy=true],
	[CXXFLAGS="$CXXFLAGS -g"]
)

AC_ARG_WITH(pedantic,
	AS_HELP_STRING([--with-pedantic],[Add -pedantic to compile options]),
	[CXXFLAGS="$CXXFLAGS -pedantic"]
)

AC_ARG_WITH(optimization,
	AS_HELP_STRING([--with-optimization],[Optimize the executables]),
	[CXXFLAGS="$CXXFLAGS -O"],
	[CXXFLAGS="$CXXFLAGS -O0"]
)

AC_ARG_WITH(prof,
	AS_HELP_STRING([--with-prof],[Generate profiling information]),
	[CXXFLAGS="$CXXFLAGS -pg"]
)

PGSQL_LIB=/usr/local/pgsql/lib
AC_ARG_WITH(pgsql-lib,
	AS_HELP_STRING([--with-pgsql-lib=PGSQLLIBDIR],[Specify location to find libpq++.so, default is \
/usr/local/pgsql/lib]),
	[PGSQL_LIB=$withval]
)
dnl echo PGSQL_LIB: $PGSQL_LIB

PGSQL_INCLUDE=/usr/local/pgsql/include
AC_ARG_WITH(pgsql-include,
	AS_HELP_STRING([--with-pgsql-include=PGSQLINCLUDEDIR],[Specify location to find libpq++.h, default is \
/usr/local/pgsql/include]),
	[PGSQL_INCLUDE=$withval]
)
dnl echo PGSQL_INCLUDE: $PGSQL_INCLUDE

AC_MSG_CHECKING(for libpq++.h)
if [test -f "$PGSQL_INCLUDE/libpq++.h"]; then
	/bin/true
elif [test -f "/usr/local/include/libpq++.h"]; then
	PGSQL_INCLUDE=/usr/local/include
elif [test -f "/usr/include/postgresql/libpq++.h"]; then
	PGSQL_INCLUDE=/usr/include/postgresql
elif [test -f "/usr/include/pgsql/libpq++.h"]; then
	PGSQL_INCLUDE=/usr/include/pgsql
else
        AC_MSG_ERROR( Unable to find libpq++.h )
fi
AC_MSG_RESULT(yes)

AC_MSG_CHECKING(for libpq++.so)
if [test -f "$PGSQL_LIB/libpq++.so"]; then
	/bin/true
elif [test -f "/usr/local/lib/libpq++.so"]; then
	PGSQL_LIB=/usr/local/lib
elif [test -f "/usr/lib/libpq++.so"]; then
	PGSQL_LIB=/usr/lib
elif [test -f "/usr/lib/postgresql/libpq++.so"]; then
	PGSQL_LIB=/usr/lib/postgresql
elif [test -f "/usr/lib/pgsql/libpq++.so"]; then
	PGSQL_LIB=/usr/lib/pgsql
elif [test -f "/usr/local/pgsql/lib/libpq++.so"]; then
	PGSQL_LIB=/usr/local/pgsql/lib
elif [test -f "/usr/local/postgresql/lib/libpq++.so"]; then
	PGSQL_LIB=/usr/local/postgresql/lib
else
        AC_MSG_ERROR( Unable to find libpq++.so)
fi
AC_MSG_RESULT(yes)

dnl echo "PGSQL_INCLUDE: $PGSQL_INCLUDE"
dnl echo "PGSQL_LIB: $PGSQL_LIB"
dnl exit

CXXFLAGS="$CXXFLAGS -I`pwd`/include -I`pwd`/lib"

hashMapFile=

if [test -f "/usr/include/g++/ext/hash_map"]; then
	hashMapFile="/usr/include/g++/ext/hash_map"
	AC_DEFINE(GNU_EXTENSIONS)
elif [test -f "/usr/local/include/g++/ext/hash_map"]; then
	hashMapFile="usr/local/include/g++/ext/hash_map";
	AC_DEFINE(GNU_EXTENSIONS)
fi

gccVersion=`\gcc -v 2>&1 | \tail -1 | \cut -d" " -f3`
dnl echo gccVersion: $gccVersion

gccMajor=`echo $gccVersion | \cut -d"." -f1`
dnl echo gccMajor: $gccMajor

if [test "$gccMajor" = "3"]; then
	# 3.0.x have ext/hash_map, but std namespace
	gccMid=`echo $gccVersion | \cut -d"." -f2`
dnl	echo gccMid: $gccMid

	if [test "$gccMid" = "0"]; then
		AC_DEFINE(HASHMAPNS, std)
	else
		# 3.1 or above
		AC_DEFINE(HASHMAPNS, __gnu_cxx)
	fi
else
	AC_DEFINE(HASHMAPNS, std)
fi

echo
echo "Detecting modules..."
echo

MOD_DIRS=""
searchForMods=yes

AC_ARG_ENABLE(modules,
	AS_HELP_STRING([--enable-modules=mods],[Enable one or more modules; all modules found in the \
current directory are enabled by default.  Specify "no" (without quotes) \
to build server only]),
[
dnl echo "enableval: $enableval"
if [test $enableval = "yes"]; then
	searchForMods=yes
elif [test $enableval = "no"]; then
	searchForMods=no
else
	tmp="mod.$enableval"
dnl 	echo "tmp: $tmp"
	tmp=`echo $tmp | sed 's/,/ mod./g'`
dnl	echo "tmp: $tmp"
	MOD_DIRS="$MOD_DIRS $tmp"
	searchForMods=no
fi
],
[
dnl echo "running later"
searchForMods=yes
]) dnl AC_ARG_ENABLE

dnl echo "searchForMods: $searchForMods"
if [test $searchForMods = yes]; then
	mod_list=`\ls | \grep "mod."`
dnl	echo "mod_list: $mod_list"

	for file in $mod_list; do
		if [ ! test -d $file ]; then
			echo "$file is not a directory!"
			continue
		fi

		if [ test -f $file/Makefile.in ]; then
			echo Adding module `echo $file | \cut -f2 -d"."`
			MOD_DIRS="$MOD_DIRS $file"
		fi
	 done
fi

dnl echo "MOD_DIRS = $MOD_DIRS"
AC_SUBST(MOD_DIRS)

AC_SUBST(CXXFLAGS)
AC_SUBST(INCLTDL)
AC_SUBST(LIBLTDL)
AC_SUBST(glibdir)
AC_SUBST(LIBTOOL_DEPS)
AC_SUBST(PGSQL_INCLUDE)
AC_SUBST(PGSQL_LIB)
AC_SUBST(bindir)

AC_CONFIG_FILES([Makefile \
db/Makefile \
lib/Makefile \
libircu/Makefile \
src/Makefile \
mod.cservice/Makefile \
mod.nickserv/Makefile])

AC_OUTPUT

echo
echo Configuration has been finished, and everything seems to be alright
echo Now you have the chance to modify include/config.h to set your
echo preferred options...
echo
