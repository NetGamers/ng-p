## Process this file with automake to produce Makefile.in
# "$Id: Makefile.am,v 1.19 2003-11-04 20:47:38 jeekay Exp $"

lib_LTLIBRARIES = libcservice.la
libcservice_la_LDFLAGS = -module -export-dynamic -L@PGSQL_LIB@ \
	-lpq++

libcservice_la_SOURCES = cservice.cc \
	networkData.cc \
	sqlChannel.cc \
	sqlCommandLevel.cc \
	sqlUser.cc \
	sqlLevel.cc \
	sqlBan.cc \
	sqlPendingChannel.cc \
	sqlPendingTraffic.cc \
	ACCESSCommand.cc \
	ADDUSERCommand.cc \
	ADMINCMDSCommand.cc \
	BANCommand.cc \
	BANLISTCommand.cc \
	CHANINFOCommand.cc \
	CHINFOCommand.cc \
	CLEARMODECommand.cc \
	COMMENTCommand.cc \
	CONFIGCommand.cc \
	DEBUGCommand.cc \
	DEOPCommand.cc \
	DEVOICECommand.cc \
	FORCECommand.cc \
	GETLEVELCommand.cc \
	GLOBNOTICECommand.cc \
	GSUSPENDCommand.cc \
	GUNSUSPENDCommand.cc \
	HELPCommand.cc \
	INVITECommand.cc \
	INVMECommand.cc \
	ISREGCommand.cc \
	JOINCommand.cc \
	KICKCommand.cc \
	LBANLISTCommand.cc \
	LOGINCommand.cc \
	MODECommand.cc \
	MODINFOCommand.cc \
	MOTDCommand.cc \
	NEWPASSCommand.cc \
	NOTECommand.cc \
	OFFICIALCommand.cc \
	OPCommand.cc \
	OPERJOINCommand.cc \
	OPERPARTCommand.cc \
	OPERSUSPENDCommand.cc \
	PARTCommand.cc \
	PURGECommand.cc \
	QUOTECommand.cc \
	RECOVERCommand.cc \
	REGISTERCommand.cc \
	REHASHCommand.cc \
	RELEASECommand.cc \
	REMIGNORECommand.cc \
	REMOVEALLCommand.cc \
	REMUSERCommand.cc \
	REMUSERIDCommand.cc \
	SAYCommand.cc \
	SCANCommand.cc \
	SEARCHCommand.cc \
	SERVNOTICECommand.cc \
	SETCommand.cc \
	SETTIMECommand.cc \
	SHOWCOMMANDSCommand.cc \
	SHOWIGNORECommand.cc \
	SHUTDOWNCommand.cc \
	STATSCommand.cc \
	STATUSCommand.cc \
	SUPPORTCommand.cc \
	SUSPENDCommand.cc \
	TOPICCommand.cc \
	UNBANCommand.cc \
	UNFORCECommand.cc \
	UNSUSPENDCommand.cc \
	UPDATEDBCommand.cc \
	UPDATEIDLECommand.cc \
	VERIFYCommand.cc \
	VOICECommand.cc

INCLUDES = -I@PGSQL_INCLUDE@ $(INCLTDL)
