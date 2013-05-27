CFILES=dualshock3.c
CC=gcc
CFLAGS=-Wall -std=gnu99 -O2 -g
LDFLAGS=-lpthread
EXE="libps3_controller.so"
O=${CFILES:.c=.o}

all:
	gcc -c -Wall -std=gnu99 -fpic ${CFILES}
	gcc -shared -o libps3_controller.so ${O} ${LDFLAGS}

install:
	cp -R include/ps3_controller.h /usr/include/
	cp -R libps3_controller.so /usr/lib

clean:
	rm -vf *.o

distclean: clean
	rm -vf ${EXE}
