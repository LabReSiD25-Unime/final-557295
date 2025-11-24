CC=gcc
CFLAGS=-I. -g
LDFLAGS=-pthread 

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)


all: server

server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)


%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o  server