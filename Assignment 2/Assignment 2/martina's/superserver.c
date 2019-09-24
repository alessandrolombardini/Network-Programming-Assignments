#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>

//Constants and global variable declaration goes here
#define MAX_SERVICES 10
#define MAX_CHAR 255

//Service structure definition goes here
typedef struct{
	char transport_protocol[MAX_CHAR];
	char service_mode[MAX_CHAR];
	char service_port[MAX_CHAR];
	char service_path[MAX_CHAR];
	char service_name[MAX_CHAR];
	char sfd[MAX_CHAR];
	int pid;
}structure;

structure service_info[MAX_SERVICES];

//Function devoted to handle the death of the son process
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

void read_configuration_file(){
	static char* conf_name = "inetd.conf";
        FILE* fp;
        int services_cntr = 0;

	fp = fopen(conf_name, "r");
        if (fp == NULL){
                perror("Error while opening configuration file.\n");
                exit(1);
        }

        while(fscanf(fd, "%s %s %s %s\n", services_info[i].service_name, services_info[i].transport_protocol, services_info[i].service_port, services_info[i].service_mode) != EOF) {
                services_cntr++;
        }

        fclose(fp);
}

int  main(int argc,char **argv,char **env){ // NOTE: env is the variable to be passed, as last argument, to execle system-call

// Other variables declaration goes here


// Server behavior implementation goes here

	// --- open configuration file in reading mode. for each line extract params and save info into data structure
        read_configuration_file();
	// --- create required socket for the service (if the TCP service then invoke listen() + save sfd into data structure

	// --- fill select()

	// --- invoke signal
	signal (SIGCHLD,handle_signal); /* Handle signals sent by son processes - call this function when it's ought to be */

	// --- run infinite loop based on select operation

	// --- if select is fired, if service belongs to TCP then invoce accept operation in order to retrieve the connected sfd

	// --- invokes fork() and process it as follow
	//	- Father:
	//		if service is tcp then close the connected socket
	//		if service is no-wait, go back to select
	//		if service is wait, he register PID inside data structure, removes its sfd from select and go back to select
	//	- Son:
	//		closes all std I/O fd (0,1,2)
	//		if service is tcp then closes welcome socket
	//		it calls 3 times dup operation in order to associate sfd to stdin, stdout and stderr
	//		it invokes execle in order to execute requested service.

	return 0;
}
