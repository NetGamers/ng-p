## Process this file with automake to produce Makefile.in
# "$Id: Makefile.am,v 1.4 2002-01-30 21:57:54 jeekay Exp $"

lib_LTLIBRARIES = libnickserv.la
libnickserv_la_SOURCES = nickserv.cc \
			nsUser.cc \
			sqlnsUser.cc \
			nickservCommands.cc \
			SAYCommand.cc \
			STATSCommand.cc

libnickserv_la_LDFLAGS = -module -export-dynamic -L@PGSQL_LIB@ -lpq++
INCLUDES = -I@PGSQL_INCLUDE@ $(INCLTDL)
