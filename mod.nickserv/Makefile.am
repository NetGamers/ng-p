## Process this file with automake to produce Makefile.in
# "$Id: Makefile.am,v 1.5 2002-02-04 04:45:34 jeekay Exp $"

lib_LTLIBRARIES = libnickserv.la
libnickserv_la_SOURCES = nickserv.cc \
			nsUser.cc \
			sqlnsUser.cc \
			juUser.cc \
			nickservCommands.cc \
			JUPECommand.cc \
			SAYCommand.cc \
			STATSCommand.cc

libnickserv_la_LDFLAGS = -module -export-dynamic -L@PGSQL_LIB@ -lpq++
INCLUDES = -I@PGSQL_INCLUDE@ $(INCLTDL)
