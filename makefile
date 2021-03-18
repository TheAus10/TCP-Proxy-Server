all: client server

server: proxyServer.o
	gcc -o server proxyServer.o

proxyServer.o: proxyServer.c
	gcc -c proxyServer.c

client: client.o
	gcc -o client client.o

client.o: client.c
	gcc -c client.c

clean:
	rm *.o
	rm server
	rm client
