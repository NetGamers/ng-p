## Process this file with automake to produce Makefile.in
# "$Id: Makefile.am,v 1.7 2002-03-18 20:01:45 jeekay Exp $"

lib_LTLIBRARIES = libnickserv.la
libnickserv_la_SOURCES = nickserv.cc \
			nsUser.cc \
			juUser.cc \
			nickservCommands.cc \
			JUPECommand.cc \
			RELEASECommand.cc \
			SAYCommand.cc \
			STATSCommand.cc

libnickserv_la_LDFLAGS = -module -export-dynamic -L@PGSQL_LIB@ -lpq++
INCLUDES = -I@PGSQL_INCLUDE@ $(INCLTDL)
