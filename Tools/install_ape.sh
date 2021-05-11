#!/bin/sh -e


##########################################################################
#   Function description:
#       Pause until user presses return
##########################################################################

pause()
{
    local junk
    
    printf "Press return to continue..."
    read junk
}


if [ `uname` = SunOS ]; then
    MAKE=gmake
    INSTALL=/usr/ucb/install
    PATH=${PATH}:/usr/lib
    export PATH
    if [ ! -f /usr/ucb/install ]; then
	printf "Missing /usr/ucb/install.  Install the SUNSWscp package.\n"
	exit 1
    fi
else
    MAKE=make
    INSTALL=install
fi

# APE Makefile must be in '..'
if [ $0 != ./install_ape.sh ]; then
    printf "Error: $0 must be run as ./install_ape.sh from the Tools directory.\n"
    exit 1
fi

cat << EOM

Wherever possible, software should be installed via a ports/packages system
that handles downloading, patching, dependencies, etc. and has a clean
uninstall mechanism.  Examples include BSD ports, MacPorts, 
Debian packages, Gentoo Portage, Fink, etc.

This script will install APE on systems that do not have such a port/package.

Before using this script, see if APE can be installed via a ports/packages
system on your operating system.  If there is no APE port/package for your
system, consider creating one rather than using this script. 

EOM
pause

if [ $# = 1 ]; then
    PREFIX=$1
else
    default_prefix="/usr/local"
    printf "Install PREFIX? [default=${default_prefix}]"
    read PREFIX
    if [ '0'${PREFIX} = '0' ]; then 
	PREFIX=${default_prefix}
    fi
fi
export PREFIX

LOCALBASE=${PREFIX}
export LOCALBASE

INSTALL_PROGRAM="cp"
export INSTALL

INSTALL_DATA="cp -r"
export INSTALL_DATA

CFLAGS="-g -Wall"
export CFLAGS

CC="gcc"
export CC

master_site=http://acadix.biz/Ports/distfiles

# Currently supported versions of dependencies
libbacon=libbacon-1.2.2
libpare=libpare-1.1.1
libtwin=twintk-0.9.4

# Get latest distfiles
for archive in  ${libbacon}.tar.xz \
		${libpare}.tar.xz \
		${libtwin}.tar.xz
do
    if [ ! -f ${archive} ]; then
	curl -O ${master_site}/${archive}
    else
	printf "${archive} already exists.  Skipping...\n"
	printf "If this is not what you want, remove it and run $0 again.\n"
    fi
done

# Unpack, build, and install in the proper order
# Lippare and libtwin depend on libbacon
for dir in ${libbacon} ${libpare} ${libtwin}; do
    printf "\n${dir}:\n\n"
    if [ ! -d ${dir} ]; then
	printf "Unpacking...\n"
	xz -dc ${dir}.tar.xz | tar xf -
    else
	printf "${dir} already exists.  Skipping...\n"
    fi
    cd ${dir}

    # Some default compilers don't support -MM, so use gcc
    ${MAKE} CC=${CC} LOCALBASE=${LOCALBASE} PREFIX=${PREFIX} depend
    ${MAKE} LOCALBASE=${LOCALBASE} PREFIX=${PREFIX} INSTALL=${INSTALL} install
    cd ..
done

# Build APE
cd ..

# Some default compilers don't support -MM, so use gcc
${MAKE} CC=${CC} LOCALBASE=${LOCALBASE} PREFIX=${PREFIX} depend
${MAKE} LOCALBASE=${LOCALBASE} PREFIX=${PREFIX} INSTALL=${INSTALL} install

