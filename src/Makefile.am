
MAINTAINERCLEANFILES    = Makefile.in
MOSTLYCLEANFILES        = @TEST_CLEAN@

AM_CFLAGS		= @CPPFLAGS@ @OPENHPI_CFLAGS@

# providerdir defines where provider libraries will be installed (@PROVIDERDIR@ is setby the configure script).
providerdir = @PROVIDERDIR@

# ==================================================================
# Automake instructions for ./src subdir
# ==================================================================
# MAKE CHANGES HERE TO COMPILE ALL THE SOURCE FILES FOR YOUR CMPI PROVIDER

# LIST EACH CMPI CLASS PROVIDER LIBRARY, ITS SOURCE FILE(S), AND ANY LIBS REQUIRED FORLINKING HERE
# Files and Directories CMPI provider libraries
provider_LTLIBRARIES = libHpi.la
libHpi_la_SOURCES = Hpi.c

libHpi_la_LDFLAGS   = @OPENHPI_LIBS@ -version-info @HPI_CIM_VERSION@ 
