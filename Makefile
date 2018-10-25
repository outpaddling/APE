
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

OBJS2   = Ascii/ascii.o
OBJS    = ${OBJS1} ${OBJS2}

#####################################
# Compile, link, and install options

PREFIX  ?= /usr/local
MANPREFIX  ?= ${PREFIX}
LOCALBASE ?= /usr/local
DATADIR ?= ${PREFIX}/share/ape
DOCSDIR ?= ${PREFIX}/share/doc/ape

# APE depends on signed chars.  Some compilers treat chars as unsigned
# by default, so adjust compiler flags as needed. (e.g. gcc -fsigned-char)
CC      ?= cc
CFLAGS  ?= -g -Wall
INCLUDES = -I${LOCALBASE}/include
CFLAGS  += ${INCLUDES} -DINSTALL_PREFIX="\"${PREFIX}\"" -fsigned-char

LDFLAGS += -L${LOCALBASE}/lib -ltwintk -lpare -lbacon

INSTALL         ?= install
INSTALL_PROGRAM ?= install -m 0755
INSTALL_DATA    ?= install -m 0644
LN              ?= ln
RM              ?= rm
CHMOD           ?= /bin/chmod
MAKE            ?= make

#####################################
# Standard targets required by ports

all:    ${BINS} ${LIBS}

# Link rules
${BIN1}:        ${OBJS1}
		${CC} -o ${BIN1} ${OBJS1} ${LDFLAGS}

${BIN2}:        ${OBJS2}
		(cd Ascii; ${MAKE})

include Makefile.depend

depend:
	rm -f Makefile.depend
	for file in *.c; do \
	    ${CC} ${INCLUDES} -MM $${file} >> Makefile.depend; \
	    printf "\t\$${CC} -c \$${CFLAGS} $${file}\n" >> Makefile.depend; \
	done

# Remove generated files (objs and nroff output from man pages)
clean:
	rm -f ${OBJS} ${BINS} ${LIBS} *.nr

realclean: clean
	rm -f .*.bak *.bak *.BAK *.core

install: all
	mkdir -p ${STAGEDIR}${PREFIX}/bin \
		${STAGEDIR}${MANPREFIX}/man/man1 \
		${STAGEDIR}${DATADIR} \
		${STAGEDIR}${DOCSDIR}
	${INSTALL_PROGRAM} ${BINS} ${STAGEDIR}${PREFIX}/bin
	${INSTALL_PROGRAM} ${SCRIPTS} ${STAGEDIR}${PREFIX}/bin
	${INSTALL_DATA} ${MANS} ${STAGEDIR}${MANPREFIX}/man/man1
	${INSTALL_DATA} ${HTML} ${STAGEDIR}${DOCSDIR}
	cp -Rp Aperc/Languages Aperc/options.rc Aperc/custom_menu \
		${STAGEDIR}${DATADIR}
	${CHMOD} -R u+rwX,go-w+rX ${STAGEDIR}${DATADIR}

protos:
	(cproto ${INCLUDES} *.c > temp_protos.h && mv -f temp_protos.h protos.h)

