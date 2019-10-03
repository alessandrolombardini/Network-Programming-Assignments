#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>

//Constants and global variable declaration goes here
#define TRUE 1
#define FALSE 0
#define PID_NULL -1 
#define MAX_NUMBER_OF_SERVICES 10
#define BACK_LOG 2 // Maximum queued requests

int numberOfServicesLoaded; // Number of services inside inetd.conf
int signalEvent = FALSE; // TRUE if signal has been called

// Service structure definition goes here
typedef struct node {
	char transportProtocol[4];
	char serviceMode[7];
	char servicePort[6];
	char serviceFullPathName[256];
	char serviceName[256];
	int socketFileDescriptor;
	int pid;
} serviceNode;

serviceNode services[MAX_NUMBER_OF_SERVICES];
fd_set readSet;

// handle_signal implementation
void handle_signal (int sig){
	// Call to wait system-call goes here
	pid_t p;

	switch (sig) {
		case SIGCHLD : 
			// Implementation of SIGCHLD handling goes here	
			p = wait(NULL);
			for(int i = 0; i < numberOfServicesLoaded; i++) {
				if(services[i].pid == p && strcmp(services[i].serviceMode, "wait") == 0){
					FD_SET(services[i].socketFileDescriptor, &readSet);
					services[i].pid = PID_NULL;
				}
			}
		break;

		default: 
			printf ("Signal not known!\n"); 
			fflush(stdout);
		break;
	}
	signalEvent = TRUE; // Note signal call
}

/* Read inetd.conf file and save all its data on data structure ('services') */
void readConfiguration() {
	FILE *fd;

	fd = fopen("inetd.conf","r");
	if(fd==NULL){
		printf("Si è verificato un errore in apertura del file");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	int i = 0;
	while(fscanf(fd, "%s %s %s %s\n", services[i].serviceFullPathName, services[i].transportProtocol, services[i].servicePort, services[i].serviceMode) != EOF) {
		strcpy(services[i].serviceName, strrchr(services[i].serviceFullPathName, '/') + 1);
		services[i].pid = PID_NULL;
		i++;
	}
	fclose(fd);
	numberOfServicesLoaded = i;
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

/* Start all services readed inside inetd.conf file */
void startServices() {
	int fd;
	for(int i = 0; i < numberOfServicesLoaded; i++){
		if(strcmp(services[i].transportProtocol, "udp") == 0){
			fd = openUDPSocket(atoi(services[i].servicePort));
		} else {
			fd = openTCPSocket(atoi(services[i].servicePort));
		}
		// Save socket file descriptor inside data structure
		services[i].socketFileDescriptor = fd;
	}
}

void printConfiguration() {
	for(int i = 0; i < numberOfServicesLoaded; i++){
		printf("%s %s %s %s\n", services[i].serviceName, services[i].transportProtocol, services[i].servicePort, services[i].serviceMode);
		fflush(stdout);
	}
}

void manageMessage(char **env) {
	struct sockaddr_in client_address;
	socklen_t client_size = sizeof(client_address);
	
	// Scan file descriptors of services to know which one has been activated 
	for(int i = 0; i < numberOfServicesLoaded; i++){
		if(FD_ISSET(services[i].socketFileDescriptor, &readSet)){
			// Accept connection if it's TCP
			int newSocket;
			if(strcmp(services[i].transportProtocol, "tcp") == 0) {
				newSocket = accept(services[i].socketFileDescriptor, (struct sockaddr *)&client_address, &client_size);
				if(newSocket < 0){
					perror("Accept error");
					fflush(stdout);
					exit(EXIT_FAILURE);
				}
			}	
			// Create a new child to manage new connection
			int forkPid = fork();
			int socketToManage;
			if(forkPid == 0){
				// Son process
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
				if(execle(services[i].serviceFullPathName, services[i].serviceName, NULL, env) < 0){
					perror("\nError while calling execle.\n");
					fflush(stdout);
					exit(EXIT_FAILURE);
				}
			} else {
				// Father process
				if(strcmp(services[i].transportProtocol, "tcp") == 0){
					close(newSocket);
				}
				if(strcmp(services[i].serviceMode, "wait") == 0) {
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
		// Load file descriptors of services on a fd_set before empty that
		FD_ZERO(&readSet);
		int maxFD = 0;
		for(int i = 0; i < numberOfServicesLoaded; i++){
			if(services[i].pid == PID_NULL || strcmp(services[i].serviceMode, "nowait") == 0) {
				FD_SET(services[i].socketFileDescriptor, &readSet);
				if(services[i].socketFileDescriptor > maxFD){
					maxFD = services[i].socketFileDescriptor;
				}
			}
		}
		timeToWait.tv_sec = 10;
		timeToWait.tv_usec = 0;
		int temp = select(maxFD + 1, &readSet, NULL, NULL, &timeToWait);
		if(signalEvent == FALSE) { // Check if select has been interrupted by a signal
			if(temp < 0){
				printf("Select error\n");
			} else if(temp == 0){
				printf("Timeout expired\n");
			} else {
				manageMessage(env);
			}
		} else {
			signalEvent = FALSE;
		}
	}
}

int  main(int argc,char **argv,char **env){ // NOTE: env is the variable to be passed, as last argument, to execle system-call
	// Other variables declaration goes here

	// Server behavior implementation goes here
	readConfiguration();
	startServices();
    printConfiguration();

	signal (SIGCHLD,handle_signal); /* Handle signals sent by son processes - call this function when it's ought to be */

	manageServices(env);

	return 0;
}


