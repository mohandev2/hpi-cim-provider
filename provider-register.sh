#!/bin/sh
# $Id$
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
# Author:       Viktor Mihajlovski <mihajlov@de.ibm.com>
# Contributors:
# Description:  Script to install class definitions (MOFs) and registration data
#		for a variety of supported CIMOMs
# ==================================================================

function pegasus_install()
{
    if ps -C cimserver > /dev/null 2>&1 
    then
	CIMMOF=cimmof
	state=active
    else
	CIMMOF=cimmofl
	state=inactive
    fi
    
    if test $1 == mofs 
    then
	NAMESPACE=root/cimv2
	action="Installing Schemas"
    else if test $1 == regs
    then
	NAMESPACE=root/PG_Interop
	action="Registering Providers"
    else
	echo "Invalid install mode " $1
	return 1
    fi
    fi

    shift
    echo $action with $state cimserver
    $CIMMOF -n $NAMESPACE $*
}

function sfcb_install()
{
    if test $1 == mofs 
    then
	action="Staging Schemas"
	shift
	params="$*"
    else if test $1 == regs
    then
	action="Staging Provider Registration -- rebuild repository and restart sfcb!"
	shift
	params="-r $*"
    else
	echo "Invalid install mode " $1
	return 1
    fi
    fi
    
    echo $action
    sfcbstage $params
}

function usage() 
{
    echo "usage: $0 [-h] -t <cimserver> [ -s mof ... | -r  regfile ... ]"
}

args=`getopt ht:r:s: $*`

if [ $? != 0 ]
then
    usage $0
    exit 1
fi

set -- $args

while [ -n "$1" ]
do
  case $1 in
      -h) help=1; 
	  shift;
	  break;;
      -t) cimserver=$2;
	  shift 2;;
      -s) mofs=$2;
	  shift 2;;
      -r) regs=$2;
	  shift 2;;
      --) shift;
	  break;;
      **) break;;
  esac
done

if [ "$help" == "1" ]
then
    usage
    echo -e "\t-h display help message"
    echo -e "\t-t specify cimserver type (pegasus|sfcb|openwbem|sniacimom)"
    echo -e "\t-s specify schema mofs"
    echo -e "\t-r specify registration files"
    echo
    echo Use this command install schema mofs or register provider
    echo CIM Server Type is required, schema and registration files are
    echo mutually exclusive.
    exit 0
fi

if test x$cimserver == x
then
    usage $0
    exit 1
fi

if test x"$mofs" == x && test x"$regs" == x
then
    usage $0
    exit 1
fi

if test x"$mofs" != x && test x"$regs" != x
then
    usage $0
    exit 1
fi

if test x"$mofs" != x
then
    mode=mofs
    mofs="$mofs $*"
else if test x"$regs" != x
then
    mode=regs
    regs="$regs $*"
fi
fi

case $cimserver in
    pegasus) pegasus_install $mode $mofs $regs;;
    sfcb)    sfcb_install $mode $mofs $regs;;
    openwbem) echo openwbem not yet supported && exit 1 ;;
    sniacimom) echo sniacimom not yet supported && exit 1 ;;
    **)	echo "Invalid CIM Server Type " $cimserver && exit 1;;
esac
