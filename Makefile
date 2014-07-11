CC=gcc
WFLAGS=-W -Wall -pedantic -std=c99 -g -O0
LFLAGS=-lm -ledit
TARGET=clisp

all: main.o mpc.o parser.o
	$(CC) main.o mpc.o parser.o $(WFLAGS) $(LFLAGS) -o $(TARGET)

main.o:
	$(CC) main.c $(WFLAGS) -c

parser.o:
	$(CC) parser.c $(WFLAGS) -c

mpc.o:
	$(CC) mpc/mpc.c -c

clean:
	rm -rf *.o $(TARGET)


