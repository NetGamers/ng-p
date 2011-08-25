## Process this file with automake to produce Makefile.in

bin_PROGRAMS = gnuworld

gnuworld_LDFLAGS = -rpath $(bindir) $(glibdir)/libgnuworld.la \
	-R$(bindir) -export-dynamic -module $(LIBLTDL)
gnuworld_DEPENDENCIES = $(LIBLTDL) $(glibdir)/libgnuworld.la
gnuworld_SOURCES = Channel.cc \
	ChannelUser.cc \
	LoadClientTimerHandler.cc \
	Network.cc \
	ServerTimerHandlers.cc \
	UnloadClientTimerHandler.cc \
	client.cc \
	iClient.cc \
	iServer.cc \
	ip.cc \
	main.cc \
	server.cc
