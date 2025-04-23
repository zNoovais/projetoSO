CC = gcc
CFLAGS = -Wall -g

all: server client 

server: server.o functions.o
	$(CC) $(CFLAGS) -o server server.o functions.o

client: client.o functions.o
	$(CC) $(CFLAGS) -o client client.o functions.o

server.o: dserver.c
	$(CC) $(CFLAGS) -c dserver.c -o server.o

client.o: dclient.c functions.o
	$(CC) $(CFLAGS) -c dclient.c -o client.o

functions.o: functions.c 
	$(CC) $(CFLAGS) -c functions.c

pipe_to_server:
	mkfifo pipe_to_server

clean:
	rm -f *.o server client 
	rm -f pipe_to_server
