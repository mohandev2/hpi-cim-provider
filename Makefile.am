# $Id:
# ==================================================================
# (C) Copyright IBM Corp. 2005
#
# THIS FILE IS PROVIDED UNDER THE TERMS OF THE COMMON PUBLIC LICENSE
# ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
# CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
#
# You can obtain a current copy of the Common Public License from
# http://www.opensource.org/licenses/cpl1.0.php
#
# Author:       Dr. Gareth S. Bestor, <bestorga@us.ibm.com>
# Contributors:
# Description:  Automake input file for sample CMPI process provider
# ==================================================================

# Start from an empty extra distribution file list
EXTRA_DIST=

# We must explicity add the RPM spec file to the distribution package
EXTRA_DIST+=hpi-cim.spec

SUBDIRS = src

# ==================================================================
# Automake instructions for documentation
# ==================================================================

# docdir defines where the documentation goes
docdir=$(datadir)/doc/$(PACKAGE)-$(VERSION)
doc_DATA=README AUTHORS COPYING 

# ADD EXTRA DOC FILES IF PRESENT
doc_DATA+=doc/rpmbuild.txt
EXTRA_DIST+=doc

# providerdir defines where provider libraries will be installed (@PROVIDERDIR@ is set by the configure script). 
#providerdir = @PROVIDERDIR@

# ==================================================================
# Automake instructions for ./src subdir
# ==================================================================
# MAKE CHANGES HERE TO COMPILE ALL THE SOURCE FILES FOR YOUR CMPI PROVIDER

# LIST EACH CMPI CLASS PROVIDER LIBRARY, ITS SOURCE FILE(S), AND ANY LIBS REQUIRED FOR LINKING HERE
# Files and Directories CMPI provider libraries
#provider_LTLIBRARIES = libHpiCimProvider.la
#libHpiCimProvider_la_SOURCES = src/HpiCimProvider.c

# ==================================================================
# Automake instructions for ./schema subdir
# ==================================================================
# CHANGES THE MOF AND REGISTRATION FILENAMES TO MATCH THE FILES IN ./schema

# LIST ALL PROVIDER CLASS DEFINITION FILES (MOFS) HERE
MOFS=mofs/Hpi.mof

# LIST ALL THE CIMOM-SPECIFIC CLASS REGISTRATION DATA FILES HERE
REGISTRATIONS=mofs/HpiRegister.mof
REGISTRATIONS.pegasus=mofs/HpiRegister.mof

# Register the provider(s) and class definition(s) to the current CIM server/CIMOM
postinstall:
	test x"$(MOFS)" != x && \
		sh provider-register.sh -t @CIMSERVER@ -s $(MOFS)
	test x"$(REGISTRATIONS.@CIMSERVER@)" != x && \
		sh provider-register.sh -t @CIMSERVER@ -r $(REGISTRATIONS.@CIMSERVER@)

# Explicity add all the schema files to the distribution package
pkgdata_DATA=$(MOFS) $(REGISTRATIONS)
pkgdata_SCRIPTS=provider-register.sh
EXTRA_DIST+=$(pkgdata_DATA) $(pkgdata_SCRIPTS)

