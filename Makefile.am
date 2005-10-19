# $Id:
# ==================================================================
# (C) Copyright IBM Corp. 2005
#
# THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
# BUT WITHOUGH ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
# MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. THIS FILE 
# AND PROGRAM ARE LICENSED UNDER A BSD STYLE LICENSE. SEE THE
# COPYING FILE INCLUDED WITH THE OPENHPI DISTRIBUTION FOR FULL
# LICENSING TERMS.
#
# Author:       Renier Morales, <renierm@users.sf.net>
# Contributors:
# Description:  Automake input file for sample CMPI process provider
# ==================================================================

# Start from an empty extra distribution file list
EXTRA_DIST=

# We must explicity add the RPM spec file to the distribution package
EXTRA_DIST+=hpi-cim.spec

#SUBDIRS = src
AM_CFLAGS 	= @CFLAGS@
INCLUDES	= @OPENHPI_CFLAGS@ @CMPI_CFLAGS@

# ==================================================================
# Automake instructions for documentation
# ==================================================================

# docdir defines where the documentation goes
docdir=$(datadir)/doc/$(PACKAGE)-$(VERSION)
doc_DATA=README AUTHORS COPYING 

# ADD EXTRA DOC FILES IF PRESENT
#doc_DATA+=doc/rpmbuild.txt
#EXTRA_DIST+=doc

# providerdir defines where provider libraries will be installed (PROVIDERDIR is set by the configure script). 
providerdir = @PROVIDERDIR@

# ==================================================================
# Automake instructions for ./src subdir
# ==================================================================
# MAKE CHANGES HERE TO COMPILE ALL THE SOURCE FILES FOR YOUR CMPI PROVIDER

# LIST EACH CMPI CLASS PROVIDER LIBRARY, ITS SOURCE FILE(S), AND ANY LIBS REQUIRED FOR LINKING HERE
# Files and Directories CMPI provider libraries
provider_LTLIBRARIES = libHPI_ManagedElement.la
libHPI_ManagedElement_la_SOURCES = src/Hpi.c
libHPI_ManagedElement_la_LDFLAGS = @OPENHPI_LIBS@ -version-info @HPI_CIM_VERSION@

# ==================================================================
# Automake instructions for ./schema subdir
# ==================================================================
# CHANGES THE MOF AND REGISTRATION FILENAMES TO MATCH THE FILES IN ./schema

# LIST ALL PROVIDER CLASS DEFINITION FILES (MOFS) HERE
MOFS=mofs/Hpi.mof

# LIST ALL THE CIMOM-SPECIFIC CLASS REGISTRATION DATA FILES HERE
REGISTRATIONS=mofs/Hpi.registration
REGISTRATIONS.pegasus=mofs/Hpi.registration

# Register the provider(s) and class definition(s) to the current CIM server/CIMOM
postinstall:
	sh provider-register.sh -t @CIMSERVER@ -r $(REGISTRATIONS) -m $(MOFS)

preuninstall:
	sh provider-register.sh -d -t @CIMSERVER@ -r $(REGISTRATIONS) -m $(MOFS)

# Explicity add all the schema files to the distribution package
pkgdata_DATA=$(MOFS) $(REGISTRATIONS)
pkgdata_SCRIPTS=provider-register.sh
EXTRA_DIST+=$(pkgdata_DATA) $(pkgdata_SCRIPTS)

