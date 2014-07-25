CC=gcc
WFLAGS=-W -Wall -pedantic -std=c99 -g -O0
LFLAGS=-lm -ledit
TARGET=toylisp

all: $(TARGET) test
	mkdir -p logs
	./test_toylisp

$(TARGET): mpc.o *.c *.h
	$(CC) mpc.o common.c parser.c eval.c main.c $(WFLAGS) $(LFLAGS) -o $(TARGET)

test: mpc.o *.c *.h
	$(CC) mpc.o common.c parser.c eval.c test_toylisp.c $(WFLAGS) $(LFLAGS) -o test_$(TARGET)

mpc.o: mpc/mpc.c
	$(CC) mpc/mpc.c -g -c -o mpc.o

clean:
	rm -rf *.o $(TARGET)


