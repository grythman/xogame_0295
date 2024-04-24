CC=gcc
CFLAGS=-g -O1 -Wall
LDLIBS=-lpthread 

all: client server

client: client.c 
server: server.c 

clean:
	rm -f *.o *~ *.exe client server 

