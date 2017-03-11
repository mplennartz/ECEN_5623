INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lpthread -lrt

PRODUCT=pthread posix_clock 

HFILES= 
CFILES= pthread.c posix_clock.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	${PRODUCT}

clean:
	-rm -f *.o *.d
	-rm -f ${PRODUCT} ${GARBAGE}

pthread: pthread.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o $(LIBS)

posix_clock: posix_clock.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o $(LIBS)
	
depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
