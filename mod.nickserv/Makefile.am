## Process this file with automake to produce Makefile.in

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
AM_CPPFLAGS = $(all_includes) -I@PGSQL_INCLUDE@ $(INCLTDL)
