## Process this file with automake to produce Makefile.in
# "$Id: Makefile.am,v 1.1 2002-01-14 23:33:39 jeekay Exp $"

lib_LTLIBRARIES = libnickserv.la
libnickserv_la_SOURCES = nickserv.cc nsUser.cc sqlnsUser.cc LOGINCommand.cc \
			nickservCommands.cc
    	
libnickserv_la_LDFLAGS = -module -export-dynamic -L@PGSQL_LIB@ -lpq++
INCLUDES = -I@PGSQL_INCLUDE@ $(INCLTDL)
