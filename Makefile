CC = gcc
CFLAGS = -Wall -g

all: server client 

server: server.o functions.o
	$(CC) $(CFLAGS) -o dserver dserver.o functions.o

client: client.o functions.o
	$(CC) $(CFLAGS) -o dclient dclient.o functions.o

server.o: dserver.c
	$(CC) $(CFLAGS) -c dserver.c -o dserver.o

client.o: dclient.c functions.o
	$(CC) $(CFLAGS) -c dclient.c -o dclient.o

functions.o: functions.c 
	$(CC) $(CFLAGS) -c functions.c

pipe_to_server:
	mkfifo pipe_to_server

clean:
	rm -f *.o server client 
	rm -f pipe_to_server
	rm -f fifo_client*
	rm -f fifo_server
	rm -f armazenamento