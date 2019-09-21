all:	client.exe server.exe

client.exe: 	udpClient.o
		gcc ${CFLAGS} -o client.exe udpClient.o

udpClient.o:	udpClient.c myfunction.h
		gcc -c ${CFLAGS} udpClient.c

server.exe:	udpServer.o
		gcc ${CFLAGS} -o server.exe udpServer.o

udpServer.o:	udpServer.c myfunction.h
		gcc -c ${CFLAGS} udpServer.c

