all:		tcpClient.exe tcpServer.exe

tcpClient.exe:     tcpClient.o 
		gcc ${CFLAGS} -o tcpClient.exe tcpClient.o 

tcpClient.o:   tcpClient.c
		gcc -c -g ${CFLAGS} tcpClient.c

tcpServer.exe:     tcpServer.o 
		gcc ${CFLAGS} -o tcpServer.exe tcpServer.o 

tcpServer.o:   tcpServer.c
		gcc -c -g ${CFLAGS} tcpServer.c

.PHONY: clean

clean: 
	-rm -f *.exe  *.o

