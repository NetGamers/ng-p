## Process this file with automake to produce Makefile.in
# "$Id: Makefile.am,v 1.3 2002-01-23 01:05:47 jeekay Exp $"

lib_LTLIBRARIES = libnickserv.la
libnickserv_la_SOURCES = nickserv.cc \
			nsUser.cc \
			sqlnsUser.cc \
			nickservCommands.cc \
			LOGINCommand.cc \
			RECOVERCommand.cc \
			STATSCommand.cc

libnickserv_la_LDFLAGS = -module -export-dynamic -L@PGSQL_LIB@ -lpq++
INCLUDES = -I@PGSQL_INCLUDE@ $(INCLTDL)
