PREFIX	?= /usr/local
LOCALBASE	?= /usr/local
CFLAGS	+= -Wall -O -I. -I${LOCALBASE}/include
CC	?= cc
OBJS = ascii.o

ascii:	$(OBJS)
	$(CC) -o ascii $(OBJS) -L${LOCALBASE}/lib -ltwintk -lbacon

ascii.o: ascii.c ascii.h
	$(CC) -c ${CFLAGS} ascii.c
