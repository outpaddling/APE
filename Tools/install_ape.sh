#!/bin/sh

# Wherever possible, software should be installed via a ports system
# that handles downloading, patching, dependencies, etc. and have a clean
# uninstall mechanism.  Examples include BSD ports, MacPorts, 
# Debian (source) packages, Gentoo Portage, Fink, etc.
#
# This script will install APE on systems that do not have a port for it.
# Before using this script, see if you can install APE via ports.  If there
# is no APE port for your system, consider creating one rather than using
# this script.

# Some example overrides
# Edit these variables to your liking

if [ `uname` = SunOS ]; then
    MAKE=gmake
    INSTALL=/usr/ucb/install
    PATH=${PATH}:/usr/lib
    export PATH
    if [ ! -f /usr/ucb/install ]; then
	echo "Missing /usr/ucb/install.  Install the SUNSWscp package."
	exit 1
    fi
else
    MAKE=make
    INSTALL=install
fi

if [ $# = 1 ]; then
    PREFIX=$1
else
    default_prefix="/usr/local"
    echo "Install PREFIX? [default=${default_prefix}]"
    read PREFIX
    if [ '0'${PREFIX} = '0' ]; then 
	PREFIX=${default_prefix}
    fi
fi
export PREFIX

LOCALBASE=${PREFIX}
export LOCALBASE

INSTALL_PROGRAM="cp -r"
export INSTALL

INSTALL_DATA="cp -r"
export INSTALL_DATA

case `uname` in
    SunOS|Linux)
	ECHO_CMD="echo"
	;;
    *)
	ECHO_CMD="echo -e"
	;;
esac
export ECHO_CMD

CFLAGS="-g -Wall"
export CFLAGS

CC="gcc"
export CC

master_site=http://personalpages.tds.net/~jwbacon/Ports/distfiles

# Currently supported versions of dependencies
libbacon=libbacon-1.2.1
libpare=libpare-1.1.1
libtwin=twintk-0.9.3
ape=ape-3.5.0

# Get latest distfiles
for archive in  ${libbacon}.tar.gz \
		${libpare}.tar.gz \
		${libtwin}.tar.gz \
		${ape}.tar.gz ; do
    if [ ! -f ${archive} ]; then
	wget ${master_site}/${archive}
    else
	echo "${archive} already exists.  Skipping..."
	echo "If this is not what you want, remove it and run $0 again."
    fi
done

# Unpack, build, and install in the proper order
# Lippare and libtwin depend on libbacon
# Ape depends on all libs
for dir in ${libbacon} ${libpare} ${libtwin} ${ape}; do
    echo ""
    echo "*** ${dir} ***"
    if [ ! -d ${dir} ]; then
	echo "Unpacking..."
	gunzip -c ${dir}.tar.gz | tar xf -
    else
	echo "${dir} already exists.  Skipping..."
	echo "If this is not what you want, remove it and run $0 again."
    fi
    cd ${dir}

    # Some default compilers don't support -MM, so use gcc
    ${MAKE} ECHO=${ECHO} CC=${CC} LOCALBASE=${LOCALBASE} PREFIX=${PREFIX} depend
    ${MAKE} LOCALBASE=${LOCALBASE} PREFIX=${PREFIX} INSTALL=${INSTALL} install
    cd ..
done

