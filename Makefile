
######################################################################
# Makefile template for simple projects.  Just fill in the blanks and
# add or adjust variables as needed.  Remove unnecessary lines if
# desired.
#
# To override any variables conditionally set (with ?=), run
#
#       make VAR=value
# e.g.
#       make PREFIX=/opt/local CC=gcc CFLAGS=-O2 LDFLAGS="-L/usr/X11R6 -lX11"
#
# Author: Jason W. Bacon
#         Medical College of Wisconsin
######################################################################

################################
# Files to be installed by make

BIN1    = ape
# Place in subdir for case-insensitive filesystems, where Ascii == ascii
BIN2    = Ascii/ascii
BINS    = ${BIN1} ${BIN2}

MANS    = Man/*.1
HTML    = Man/*.html
SCRIPTS = Tools/search* Tools/ape_aspell Xape/* Java/runjava

###################################################
# List object files that comprise BIN1, BIN2, etc.

OBJS1   = ape.o files.o file_wins.o editbuff.o edit_m.o search_m.o build_m.o \
	    options.o custom.o init_opts.o cursor.o wins.o reg.o help_m.o \
	    build_opts.o resize.o match.o macros.o synhigh.o init.o \
	    messages.o error.o mouse.o subproc.o signal.o undo_stack.o \
	    undo_item.o
OBJS2   = ascii.o
OBJS    = ${OBJS1} ${OBJS2}

#####################################
# Compile, link, and install options

MANPREFIX   ?= ${PREFIX}
MANDIR      ?= ${MANPREFIX}/man
LOCALBASE   ?= ../local
PREFIX      ?= ${LOCALBASE}
DATADIR     ?= ${PREFIX}/share/APE
DOCSDIR     ?= ${PREFIX}/share/doc/APE

# APE depends on signed chars.  Some compilers treat chars as unsigned
# by default, so adjust compiler flags as needed. (e.g. gcc -fsigned-char)
CC          ?= cc
CFLAGS      ?= -g -Wall
INCLUDES    = -I${LOCALBASE}/include
CFLAGS      += ${INCLUDES} -DINSTALL_PREFIX="\"${PREFIX}\"" -fsigned-char
LDFLAGS     += -L${LOCALBASE}/lib -ltwintk -lpare -lxtend

INSTALL         ?= install
INSTALL_PROGRAM ?= install -m 0755
INSTALL_DATA    ?= install -m 0644
LN              ?= ln
RM              ?= rm
CHMOD           ?= /bin/chmod
MAKE            ?= make

#####################################
# Standard targets required by ports

all:    ${BINS}

# Link rules
${BIN1}: ${OBJS1}
	${CC} -o ${BIN1} ${OBJS1} ${LDFLAGS}

${BIN2}: ${OBJS2}
	${CC} -o ${BIN2} ${OBJS2} -L${LOCALBASE}/lib ${LDFLAGS} \
		-ltwintk -lxtend

include Makefile.depend

depend:
	rm -f Makefile.depend
	for file in *.c Ascii/ascii.c; do \
	    ${CC} ${INCLUDES} -MM $${file} >> Makefile.depend; \
	    printf "\t\$${CC} -c \$${CFLAGS} $${file}\n\n" >> Makefile.depend; \
	done

# Remove generated files (objs and nroff output from man pages)
clean:
	rm -f ${OBJS} ${BINS} *.nr

realclean: clean
	rm -f .*.bak *.bak *.BAK *.core

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin \
		${DESTDIR}${MANDIR}/man1 \
		${DESTDIR}${DATADIR} \
		${DESTDIR}${DOCSDIR}
	${INSTALL_PROGRAM} ${BINS} ${DESTDIR}${PREFIX}/bin
	${INSTALL_PROGRAM} ${SCRIPTS} ${DESTDIR}${PREFIX}/bin
	${INSTALL_DATA} ${MANS} ${DESTDIR}${MANDIR}/man1
	${INSTALL_DATA} ${HTML} ${DESTDIR}${DOCSDIR}
	cp -Rp Aperc/Languages Aperc/options.rc Aperc/custom_menu \
		${DESTDIR}${DATADIR}
	${CHMOD} -R u+rwX,go-w+rX ${DESTDIR}${DATADIR}

protos:
	(cproto ${INCLUDES} *.c > temp_protos.h && mv -f temp_protos.h protos.h)

