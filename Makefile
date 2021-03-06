CFILES=blueshock.c
CC=gcc
CFLAGS=-Wall -std=gnu99 -O2
LDFLAGS=-lpthread
EXE="libblueshock.so"
O=${CFILES:.c=.o}

all:
	${CC} ${CFLAGS} -c -fpic ${CFILES}
	${CC} -shared -o ${EXE} ${O} ${LDFLAGS}

install:
	cp -R include/blueshock.h /usr/include/
	cp -R ${EXE} /usr/lib

clean:
	rm -vf *.o

distclean: clean
	rm -vf ${EXE}
