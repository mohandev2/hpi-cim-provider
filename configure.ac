
AC_INIT
AM_INIT_AUTOMAKE(gaim-meanwhile-ibm, 1.0.1.99)

AC_PREREQ([2.50])

AC_PROG_CC

AC_DISABLE_STATIC
AC_PROG_LIBTOOL
LIBTOOL="$LIBTOOL --silent"

AC_PROG_INSTALL

AC_HEADER_STDC
AC_HEADER_SYS_WAIT

AC_C_CONST
AC_STRUCT_TM



# tell pkgconfig to look in the same prefix we're installing this to,
# as that's likely where gaim will be found if it's not in the default
# pkgconfig path
PREFIX=$ac_default_prefix
if test "$prefix" != "NONE"; then
   PREFIX=$prefix
fi

PKG_CONFIG_PATH="$PKG_CONFIG_PATH":"$PREFIX/lib/pkgconfig"
export PKG_CONFIG_PATH



dnl debug mode
AC_ARG_ENABLE(debug,
	[  --enable-debug	  compile with debugging support],,
	enable_debug=no)

if test "$enable_debug" = yes; then
   AC_DEFINE(DEBUG, 1, [Define if debugging is enabled.])
fi



dnl Check for Gaim
PKG_CHECK_MODULES(GAIM, gaim,
[
	AC_DEFINE(HAVE_GAIM, 1, [Define if we've found gaim.])
])

AC_SUBST(GAIM_CFLAGS)



dnl require glib
AM_PATH_GLIB_2_0(2.0.0,,AC_MSG_ERROR([
*** GLib 2.0 is required.]))
AC_SUBST(GLIB_CFLAGS)



dnl require gtk+-2.0
AM_PATH_GTK_2_0(2.0.0,,AC_MSG_ERROR([
*** GTK+ 2.0 is required.]))
AC_SUBST(GTK_CFLAGS)



dnl require ghttp
GHTTP_LIBS=
AC_CHECK_LIB(ghttp, ghttp_request_new, GHTTP_LIBS="-lghttp",
		    AC_MSG_ERROR("Couldn't load libghttp"), $LIBS)
AC_SUBST(GHTTP_LIBS)



dnl require openldap
OPENLDAP_LIBS=
AC_CHECK_LIB(ldap, ldap_simple_bind, OPENLDAP_LIBS="-lldap",
		   AC_MSG_ERROR("Couldn't load libldap"), $LIBS)
AC_SUBST(OPENLDAP_LIBS)



AC_CONFIG_FILES(
	[Makefile src/Makefile src/persona/Makefile]
	[gaim-meanwhile-ibm.spec]
)

AC_OUTPUT()


# The End.
