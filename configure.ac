#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(CMPI HPI-CIM Provider, 0.1.0, renierm@us.ibm.com, HpiCimProvider)
# CHANGE THIS TO THE RELATIVE PATHNAME OF *ONE* OF THE SOURCE FILES
AC_CONFIG_SRCDIR([src/Hpi.c])
AM_INIT_AUTOMAKE([1.8])

# DECLARE ANY SPECIAL CUSTOM CONFIGURE COMMAND LINE OPTIONS HERE
AC_ARG_VAR([PROVIDERDIR],[the directory where the CMPI providers will be installed.])
AC_ARG_VAR([CIMSERVER],[the target CIM server (pegasus|sfcb|openwbem|sniacimom).])

# ADD CHECKS FOR ANY SPECIAL REQUIRED PROGRAMS HERE. e.g.
#AC_CHECK_PROG(YACC,bison,[bison -y])
AC_CHECK_PROG([found_pkg_config],[pkg-config],[yes])
if test "x$found_pkg_config" != "xyes"; then
    MY_CHECK_FAIL(pkg-config,pkg-config)
fi

# ADD CHECKS FOR ANY SPECIAL REQUIRED HEADER FILES HERE
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

CHECK_OPENHPI_HEADERS
AC_SUBST(OPENHPI_CFLAGS)

# ADD CHECKS FOR ANY SPECIAL REQUIRED LIBRARIES
OPENHPI_LIBS=""
AC_CHECK_LIB(openhpi, saHpiVersionGet, OPENHPI_LIBS="-lopenhpi",
             AC_MSG_ERROR("Couldn't load libopenhpi. Get from http://openhpi.sf.net"),
	     $LIBS)
AC_SUBST(OPENHPI_LIBS)

# ADD CHECKS FOR ANY SPECIAL REQUIRED TYPEDEFS, STRUCTURES AND COMPILER OPTIONS HERE
AC_C_CONST
AC_TYPE_SIZE_T

# ADD CHECKS FOR ANY SPECIAL REQUIRED LIBRARY FUNCTIONS HERE
AC_CHECK_FUNCS([popen])


### NO CHANGES SHOULD BE NECESSARY BELOW

AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

# Check for required programs.
AC_PROG_CC
AC_PROG_LIBTOOL

HPI_CIM_VERSION=0:0:0
AC_SUBST(HPI_CIM_VERSION)

# Check for required headers
AC_HEADER_STDC

# Check for required CMPI header files (defined in acinclude.m4)
CHECK_CMPI
AC_SUBST(CMPI_CFLAGS)

# Check for required libraries (defined in acinclude.m4)
CHECK_PROVIDERDIR

# Check for CIM Server (defined in acinclude.m4)
CHECK_CIMSERVER

# Display configuration options
echo "-------------------------------------------------------"
echo "Configuration for $PACKAGE complete."
echo ""
echo "The following configuration options have been selected:"
echo "CIMSERVER: " $CIMSERVER
echo "PROVIDERDIR: " $PROVIDERDIR
echo "CPPFLAGS:" $CPPFLAGS
echo "-------------------------------------------------------"

# CHANGE THIS TO THE NAME OF THE RPM SPEC FILE
AC_CONFIG_FILES([hpi-cim.spec Makefile src/Makefile])

# Generate configure scripts for the Makefile
AC_OUTPUT

echo "You may now run make"

