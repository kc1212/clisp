CC=gcc
WFLAGS=-W -Wall -pedantic -std=c99 -g -O0
LFLAGS=-lm -ledit
TARGET=clisp

all: $(TARGET)

$(TARGET): main.o mpc.o parser.o eval.o
	$(CC) main.o mpc.o parser.o eval.o $(WFLAGS) $(LFLAGS) -o $(TARGET)

main.o: main.c
	$(CC) main.c $(WFLAGS) -c

eval.o: eval.c
	$(CC) eval.c $(WFLAGS) -c

parser.o: parser.c
	$(CC) parser.c $(WFLAGS) -c

mpc.o: mpc/mpc.c
	$(CC) mpc/mpc.c -c

clean:
	rm -rf *.o $(TARGET)


