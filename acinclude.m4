dnl
dnl $Id$
dnl
 dnl
 dnl (C) Copyright IBM Corp. 2004, 2005
 dnl
 dnl THIS FILE IS PROVIDED UNDER THE TERMS OF THE COMMON PUBLIC LICENSE
 dnl ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 dnl CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 dnl
 dnl You can obtain a current copy of the Common Public License from
 dnl  http://www.opensource.org/licenses/cpl1.0.php
 dnl
 dnl Author:       Konrad Rzeszutek <konradr@us.ibm.com>
 dnl Contributors: Viktor Mihajlovski <mihajlov@de.ibm.com>
 dnl Date  :          09/20/2004
dnl
dnl
dnl CHECK_CMPI: Check for CMPI headers and set the CPPFLAGS
dnl with the -I<directory>
dnl 
dnl CHECK_PEGASUS_2_3_2: Check for Pegasus 2.3.2 and set
dnl the HAVE_PEGASUS_2_3_2 
dnl flag 
dnl

AC_DEFUN([CHECK_OPENHPI_HEADERS],
	[
	AC_MSG_CHECKING(for OpenHPI header files)
	OPENHPI_CFLAGS=`PKG_CONFIG_PATH="/usr/local/lib/pkgconfig" pkg-config --cflags openhpi 2>/dev/null`
	if test -z "$OPENHPI_CFLAGS"; then
		AC_MSG_RESULT(no)
		MY_CHECK_FAIL(openhpi,openhpi,http://openhpi.sf.net)
	else
		AC_MSG_RESULT(yes)
	fi
	]
)

AC_DEFUN([CHECK_PEGASUS_2_3_2],
	[
	AC_MSG_CHECKING(for Pegasus 2.3.2)
	test_CIMSERVER=`cimserver -v`
	if test "$test_CIMSERVER" == "2.3.2"; then
		AC_MSG_RESULT(yes)		
		AC_DEFINE_UNQUOTED(HAVE_PEGASUS_2_3_2,1,[Defined to 1 if Pegasus 2.3.2 is used])
	else
		AC_MSG_RESULT(no)

	fi
	]
)

dnl
dnl CHECK_PEGASUS_2_4: Check for Pegasus 2.4 and set the
dnl the -DPEGASUS_USE_EXPERIMENTAL_INTERFACES flag
dnl
AC_DEFUN([CHECK_PEGASUS_2_4],
	[
	AC_MSG_CHECKING(for Pegasus 2.4)
	test_CIMSERVER=`cimserver -v`
	if test "$test_CIMSERVER" == "2.4"; then
		AC_MSG_RESULT(yes)		
		CPPFLAGS="$CPPFLAGS -DPEGASUS_USE_EXPERIMENTAL_INTERFACES"
		AC_DEFINE_UNQUOTED(HAVE_PEGASUS_2_4,1,[Defined to 1 if Pegasus 2.4 is used])
	else
		AC_MSG_RESULT(no)

	fi
	]
)
	
	
dnl
dnl Helper functions
dnl
AC_DEFUN([_CHECK_CMPI],
	[
	AC_MSG_CHECKING($1)
	AC_TRY_LINK(
	[
		#include <cmpimacs.h>
		#include <cmpidt.h>
		#include <cmpift.h>
	],
	[
		CMPIBroker broker;
		CMPIStatus status = {CMPI_RC_OK, NULL};
		CMPIString *s = CMNewString(&broker, "TEST", &status);
	],
	[
		have_CMPI=yes
		dnl AC_MSG_RESULT(yes)
	],
	[
		have_CMPI=no
		dnl AC_MSG_RESULT(no)
	])

])

dnl
dnl The main function to check for CMPI headers
dnl Modifies the CPPFLAGS with the right include directory and sets
dnl the 'have_CMPI' to either 'no' or 'yes'
dnl

AC_DEFUN([CHECK_CMPI],
	[
	AC_MSG_CHECKING(for CMPI headers)
	dnl Check just with the standard include paths
	_CHECK_CMPI(standard)
	if test "$have_CMPI" == "yes"; then
		dnl The standard include paths worked.
		AC_MSG_RESULT(yes)
	else
	  _DIRS_="/usr/include/cmpi \
                  /usr/local/include/cmpi \
		  $PEGASUS_ROOT/src/Pegasus/Provider/CMPI \
		  /opt/tog-pegasus/include/Pegasus/Provider/CMPI \
		  /usr/include/Pegasus/Provider/CMPI \
		  /usr/include/openwbem \
		  /usr/sniacimom/include"
	  for _DIR_ in $_DIRS_
	  do
		 _cppflags=$CPPFLAGS
		 _include_CMPI="$_DIR_"
		 CPPFLAGS="$CPPFLAGS -I$_include_CMPI"
		 _CHECK_CMPI($_DIR_)
		 if test "$have_CMPI" == "yes"; then
		  	dnl Found it
		  	AC_MSG_RESULT(yes)
			dnl Save the new -I parameter  
			CMPI_CFLAGS="-I$_include_CMPI"
			break
		 fi
	         CPPFLAGS=$_cppflags
	  done
	fi	
	
	if test "$have_CMPI" == "no"; then
		AC_MSG_ERROR(no. Sorry cannot find CMPI headers files.)
	fi
	]
)

dnl
dnl The check for the CMPI provider directory
dnl Sets the PROVIDERDIR  variable.
dnl

AC_DEFUN([CHECK_PROVIDERDIR],
	[
	AC_MSG_CHECKING(for CMPI provider directory)
	_DIRS="$libdir/cmpi"
	for _dir in $_DIRS
	do
		AC_MSG_CHECKING( $_dir )
		if test -d $_dir ; then
		  dnl Found it
		  AC_MSG_RESULT(yes)
		  if test x"$PROVIDERDIR" == x ; then
			PROVIDERDIR=$_dir
		  fi
		  break
		fi
        done
	if test x"$PROVIDERDIR" == x ; then
		PROVIDERDIR="$libdir"/cmpi
		AC_MSG_RESULT(implied: $PROVIDERDIR)
	fi
	]
)

dnl
dnl The "check" for the CIM server type
dnl Sets the CIMSERVER variable.
dnl

AC_DEFUN([CHECK_CIMSERVER],
	[
	AC_MSG_CHECKING(for CIM servers)
	_SERVERS="sfcbd cimserver owcimomd"
	for _name in $_SERVERS
	do
	 	AC_MSG_CHECKING( $_name )
	        which $_name > /dev/null 2>&1
		if test $? == 0 ; then
		  dnl Found it
		  AC_MSG_RESULT(yes)
		  if test x"$CIMSERVER" == x ; then
			case $_name in
			   sfcbd) CIMSERVER=sfcb;;
			   cimserver) CIMSERVER=pegasus;;
			   owcimomd) CIMSERVER=openwbem;;
			esac
		  fi
		  break;
		fi
        done
	if test x"$CIMSERVER" == x ; then
		CIMSERVER=sfcb
		AC_MSG_RESULT(implied: $CIMSERVER)
	fi
	]
)

#
# MY_CHECK_FAIL($LIBNAME,$PACKAGE_SUGGEST,$URL,$EXTRA)
#

AC_DEFUN([MY_CHECK_FAIL],
    [
        MY_MSG=`echo -e "- $1 not found!\n"`
	if test "x" != "x$4"; then
		MY_MSG=`echo -e "$MY_MSG\n- $4"`
	fi
	if test "x$2" != "x"; then
		MY_MSG=`echo -e "$MY_MSG\n- Try installing the $2 package or set PKG_CONFIG_PATH\n- to where the $2.pc file is located\n"`
	fi
	if test "x$3" != "x"; then
	        MY_MSG=`echo -e "$MY_MSG\n- or get the latest software from $3\n"`
	fi

	AC_MSG_ERROR(
!
************************************************************
$MY_MSG
************************************************************
	)
    ]
)

