#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<sys/wait.h>
#include<netinet/in.h>
#include<signal.h>
#include<errno.h>
#include<unistd.h>

//Constants and global variable declaration goes here
#define TRUE 1
#define FALSE 0
#define PID_NULL -1
#define maxNumberOfService 10

#define BACK_LOG 2 // Maximum queued requests

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
fd_set readSet;


//Function prototype devoted to handle the death of the son process
void handle_signal (int sig);

// handle_signal implementation
void handle_signal (int sig){
	// Call to wait system-call goes here
	pid_t p;

	switch (sig) {
		case SIGCHLD : 
			// Implementation of SIGCHLD handling goes here	
			p = wait(NULL);
			for(int i = 0; i < numberOfServicesLoaded; i++){
				if(services[i].pid == p){
					FD_SET(services[i].socketFileDescriptor, &readSet);
					services[i].pid = PID_NULL;
				}
			}
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
		fflush(stdout);
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
		fflush(stdout);
	}
}

int openTCPSocket(int port) {
	struct sockaddr_in server_addr;
	int serverFD;

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
	// Binding socket
	int resBinding = bind(serverFD, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if(resBinding < 0){
		perror("Binding error");
		exit(EXIT_FAILURE);
	}
	int resListen = listen(serverFD, BACK_LOG);
	if(resListen < 0){
		perror("Listen error");
		exit(EXIT_FAILURE);
	}
	return serverFD;
}

int openUDPSocket(int port){
	struct sockaddr_in server_addr;
	int serverFD;
	
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
	return serverFD;
}

void startServices() {
	int fd;

	for(int i = 0; i < numberOfServicesLoaded; i++){
		if(strcmp(services[i].transportProtocol, "udp") == 0){
			fd = openUDPSocket(atoi(services[i].servicePort));
		} else {
			fd = openTCPSocket(atoi(services[i].servicePort));
		}
		services[i].socketFileDescriptor = fd;
	}
}

void manageMessage(char **env) {
	struct sockaddr_in client_address;
	socklen_t client_size = sizeof(client_address);

	// Scan file descriptors of services to know which one has been activated 
	for(int i = 0; i < numberOfServicesLoaded; i++){
		if(FD_ISSET(services[i].socketFileDescriptor, &readSet)){
			// Accept connection
			int newSocket;
			if(strcmp(services[i].transportProtocol, "tcp") == 0) {
				newSocket = accept(services[i].socketFileDescriptor, (struct sockaddr *)&client_address, &client_size);
					FD_ZERO(&readSet);
				if(newSocket < 0){
					perror("Accept error");
					fflush(stdout);
					exit(EXIT_FAILURE);
				}
			}	
			// Create a new child to manage new connection
			int forkPid = fork();
			if(forkPid == 0){
				int socketToManage;		
				if (strcmp(services[i].transportProtocol, "tcp") == 0) {
					socketToManage = newSocket;
				} else {
					socketToManage = services[i].socketFileDescriptor;
				}
				close(0);
				close(1);
				close(2);
				dup(socketToManage);
				dup(socketToManage);
				dup(socketToManage);
				if(strcmp(services[i].transportProtocol, "tcp") == 0){
					int c = execle("./tcpServer.exe", "tcpServer.exe", NULL, env);
				} else {
					execle("./udpServer.exe", "udpServer.exe", NULL, env);
				}
			} else {
				if(strcmp(services[i].transportProtocol, "tcp") == 0){
					close(newSocket);
				}
				if(strcmp(services[i].transportProtocol, "wait") == 0) {
					services[i].pid = forkPid;
					FD_CLR(services[i].socketFileDescriptor, &readSet);
				}	
			}
		}
	}
}

void manageServices(char **env) {
	struct timeval timeToWait;
	
	while(TRUE) {
		// Load file descriptors of services on a set 
		FD_ZERO(&readSet);
		int maxFD = 0;
		for(int i = 0; i < numberOfServicesLoaded; i++){
			if(services[i].pid == PID_NULL || strcmp(services[i].transportProtocol, "nowait") == 0){
				FD_SET(services[i].socketFileDescriptor, &readSet);
				if(services[i].socketFileDescriptor > maxFD){
					maxFD = services[i].socketFileDescriptor;
				}
			}
		}
		timeToWait.tv_sec = 15;
		timeToWait.tv_usec = 0;
		int temp = select(maxFD + 1, &readSet, NULL, NULL, &timeToWait);
		if(temp < 0){
			printf("Select error\n");
		} else if(temp == 0){
			printf("Timeout expired\n");
		} else {
			manageMessage(env);
		}
	}
}

int  main(int argc,char **argv,char **env){ // NOTE: env is the variable to be passed, as last argument, to execle system-call
	// Other variables declaration goes here

	// Server behavior implementation goes here
	signal (SIGCHLD,handle_signal); /* Handle signals sent by son processes - call this function when it's ought to be */

	readConfiguration();
	printConfiguration();
	startServices();
	manageServices(env);

	return 0;
}

