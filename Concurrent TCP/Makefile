all:		client.exe server.exe

client.exe:     tcp_client.o
		gcc ${CFLAGS} -o client.exe tcp_client.o

tcp_client.o:   tcp_client.c myfunction.h
		gcc -c ${CFLAGS} tcp_client.c

server.exe:     tcp_server.o
		gcc ${CFLAGS} -o server.exe tcp_server.o

tcp_server.o:   tcp_server.c myfunction.h
		gcc -c ${CFLAGS} tcp_server.c

