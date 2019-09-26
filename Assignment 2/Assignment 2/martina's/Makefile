all:		superserver.exe udpClient.exe udpServer.exe tcpClient.exe tcpServer.exe

superserver.exe:     superserver.o
		gcc ${CFLAGS} -o superserver.exe superserver.o

superserver.o:   superserver.c
		gcc -c ${CFLAGS} superserver.c

udpClient.exe:     udpClient.o 
		gcc ${CFLAGS} -o udpClient.exe udpClient.o 

udpClient.o:   udpClient.c
		gcc -c ${CFLAGS} udpClient.c

udpServer.exe:     udpServer.o 
		gcc ${CFLAGS} -o udpServer.exe udpServer.o 

udpServer.o:   udpServer.c
		gcc -c ${CFLAGS} udpServer.c

tcpClient.exe:     tcpClient.o 
		gcc ${CFLAGS} -o tcpClient.exe tcpClient.o 

tcpClient.o:   tcpClient.c
		gcc -c ${CFLAGS} tcpClient.c

tcpServer.exe:     tcpServer.o 
		gcc ${CFLAGS} -o tctServer.exe tcpServer.o 

tcpServer.o:   tcpServer.c
		gcc -c ${CFLAGS} tcpServer.c

.PHONY: clean

clean: 
	-rm -f superserver.exe  superserver.o

