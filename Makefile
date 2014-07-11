CC=gcc
WFLAGS=-W -Wall -pedantic -std=c99 -g -O0
LFLAGS=-lm -ledit
TARGET=lisp

all: main.o mpc.o
	$(CC) main.o mpc.o $(WFLAGS) $(LFLAGS) -o $(TARGET)

main.o:
	$(CC) main.c $(WFLAGS) -c -o main.o

mpc.o:
	$(CC) mpc/mpc.c -c -o mpc.o

clean:
	rm -rf *.o $(TARGET)


