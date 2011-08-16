## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libgnuworld_db.la
libgnuworld_db_la_SOURCES = gnuworldDB.cc pgsqlDB.cc

libgnuworld_db_la_LDFLAGS = -version-info 0:0:0 -rpath $(bindir) \
	-L@PGSQL_LIB@ -lpq++
INCLUDES = -I@PGSQL_INCLUDE@
