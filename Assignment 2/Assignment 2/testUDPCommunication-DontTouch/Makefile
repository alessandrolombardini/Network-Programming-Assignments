all:		clientUDP.exe serverUDP.exe

clientUDP.exe:     udpClient.o 
		gcc ${CFLAGS} -o udpClient.exe udpClient.o 

udpClient.o:   udpClient.c
		gcc -c ${CFLAGS} udpClient.c

serverUDP.exe:     udpServer.o 
		gcc ${CFLAGS} -o udpServer.exe udpServer.o 

udpServer.o:   udpServer.c
		gcc -c ${CFLAGS} udpServer.c

.PHONY: clean

clean: 
	-rm -f *.exe  *.o

