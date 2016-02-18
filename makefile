all: client server

client: client.o
	gcc -o client client.o -lncurses

server: server.o
	gcc -o server server.o

client.o: client.c
	gcc -c client.c

server.o: client.c
	gcc -c server.c

clean:
	rm client.o server.o
