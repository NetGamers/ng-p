## Process this file with automake to produce Makefile.in
## "$Id: Makefile.am,v 1.2 2002-07-01 00:14:22 jeekay Exp $"

lib_LTLIBRARIES = libgnuworld_db.la
libgnuworld_db_la_SOURCES = gnuworldDB.cc pgsqlDB.cc

libgnuworld_db_la_LDFLAGS = -version-info 0:0:0 -rpath $(bindir) \
	-L@PGSQL_LIB@ -lpq++
INCLUDES = -I@PGSQL_INCLUDE@
