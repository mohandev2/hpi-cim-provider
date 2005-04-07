dnl configure.ac for HpiCimProvider
dnl by Renier Morales <renierm@users.sf.net>


AC_PREREQ([2.57])
AC_INIT(HpiCimProvider, 0.0.1)
AC_CONFIG_SRCDIR([README])
AM_INIT_AUTOMAKE([1.8])
AM_CONFIG_HEADER(config.h)
AH_TOP([#ifndef __OPENHPI_CONFIG_H
#define __OPENHPI_CONFIG_H])
AH_BOTTOM([#endif /* __OPENHPI_CONFIG_H */])

AC_PROG_LIBTOOL
LIBTOOL="$LIBTOOL --silent"

AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([cmpiOSBase_Common], [_check_system_key_value_pairs])
AC_CHECK_LIB([pthread], [__pthread_lock])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h signal.h])
AC_CHECK_HEADERS([cmpidt.h cmpift.h cmpimacs.h OSBase_Common.h ])
# Checks for library functions.
AC_FUNC_VPRINTF

#Substitution variables here

############################

AC_CONFIG_FILES(
	[Makefile
	 utils/Makefile
	 src/Makefile
	 mofs/Makefile]
)

AC_OUTPUT


# The End.
