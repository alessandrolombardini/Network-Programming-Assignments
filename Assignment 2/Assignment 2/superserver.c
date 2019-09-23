#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<signal.h>
#include<errno.h>

//Constants and global variable declaration goes here
#define maxNumberOfService 10

int numberOfServicesLoaded;

//Service structure definition goes here
typedef struct node {
	char transportProtocol[4];
	char serviceMode[7];
	char servicePort[6];
	char serviceFullPathName[256];
	char serviceName[256];
	int socketFileDescriptor;
	int pid;
} serviceNode;

serviceNode services[maxNumberOfService];


//Function prototype devoted to handle the death of the son process
void handle_signal (int sig);
//Other prototype 
void readConfiguration();
void printConfiguration();

// handle_signal implementation
void handle_signal (int sig){
	// Call to wait system-call goes here


	switch (sig) {
		case SIGCHLD : 
			// Implementation of SIGCHLD handling goes here	


			break;
		default : printf ("Signal not known!\n");
			break;
	}
}

void readConfiguration() {
	FILE *fd;

	fd = fopen("inetd.conf","r");
	if(fd==NULL){
		printf("Si è verificato un errore in apertura del file");
		exit(1);
	}

	int i = 0;
	while(fscanf(fd, "%s %s %s %s\n", services[i].serviceName, services[i].transportProtocol, services[i].servicePort, services[i].serviceMode) != EOF) {
		i++;
	}
	fclose(fd);
	numberOfServicesLoaded = i;
}

void printConfiguration() {
	for(int i = 0; i < numberOfServicesLoaded; i++){
		printf("%s %s %s %s\n", services[i].serviceName, services[i].transportProtocol, services[i].servicePort, services[i].serviceMode);
	}
}

void startServices() {

}

void startTCP(int port) {
	struct sockaddr_in server_addr;
	struct sockaddr_in client_add;
	int serverFD;
	int acceptFD;
	int bindResult;
	int listenResult;
	ssize_t byteRecv;
	ssize_t byteSent;

	// Open server socket
	serverFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(serverFD < 0){
		perror("Socket error");
		exit(EXIT_FAILURE);
	}
	// Initilize server address information
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;




}

void startUDP(int port){
	struct sockaddr_in server_addr;
	struct sockaddr_in client_add;
	int serverFD;
	ssize_t byteRecv;
	ssize_t byteSent;
	
	// Open server socket
	serverFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(serverFD < 0){
		perror("Socket error");
		exit(EXIT_FAILURE);
	}
	// Initilize server address information
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	// Binding socket
	int resBinding = bind(serverFD, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if(resBinding < 0){
		perror("Binding error");
		exit(EXIT_FAILURE);
	}
}



int  main(int argc,char **argv,char **env){ // NOTE: env is the variable to be passed, as last argument, to execle system-call
	// Other variables declaration goes here


	// Server behavior implementation goes here
	readConfiguration();
	printConfiguration();


	signal (SIGCHLD,handle_signal); /* Handle signals sent by son processes - call this function when it's ought to be */

	return 0;
}

