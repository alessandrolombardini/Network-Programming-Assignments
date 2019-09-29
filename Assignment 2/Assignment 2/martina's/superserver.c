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
#define MAX_SERVICES 10
#define MAX_CHAR 255
#define BACK_LOG 2 // Maximum queued requests
#define NULL_PID -1
#define TRUE 1
#define FALSE 0

fd_set read_set;
int services_cntr = 0;
int is_sigchld = 0;

//Service structure definition goes here
typedef struct{
	char transport_protocol[MAX_CHAR];
	char service_mode[MAX_CHAR];
	char service_port[MAX_CHAR];
	char service_path[MAX_CHAR];
	char service_name[MAX_CHAR];
	int sfd;
	int pid;
} structure;
structure service_info[MAX_SERVICES];

//Function devoted to handle the death of the son process
void handle_signal(int sig){
	int pid;
	switch (sig)	{
		case SIGCHLD:
			// Implementation of SIGCHLD handling goes here
			pid = wait(NULL);
			is_sigchld = TRUE;
			for (int i = 0; i < services_cntr; i++){
				if (service_info[i].pid == pid && strcmp(service_info[i].service_mode, "wait") == 0){
					printf("\n -> Superserver: La connessione per il servizio %s e' stata chiusa.\n\n", service_info[i].service_port);
					fflush(stdout);
					service_info[i].pid = NULL_PID;
					printf(" -> Superserver: Aggiungo la service port %s in ascolto.\n\n", service_info[i].service_port);
					fflush(stdout);
					FD_SET(service_info[i].sfd, &read_set);
					break;
				}
			}

/*			for (int i = 0; i < services_cntr; i++){
				if (service_info[i].pid == pid){
					printf("\n -> Superserver: La connessione per il servizio %s e' stata chiusa.\n\n", service_info[i].service_port);
					fflush(stdout);
					service_info[i].pid = NULL_PID;
					if (strcmp(service_info[i].service_mode, "wait") == 0){
						printf(" -> Superserver: Aggiungo la service port %s in ascolto.\n\n", service_info[i].service_port);
						fflush(stdout);
						FD_SET(service_info[i].sfd, &read_set);
					}
					break;
				}
			}*/

			break;
		default:
			printf("\nSignal not known!\n");
			fflush(stdout);
			break;
	}
}

void print_structure(){
	for (int i = 0; i < services_cntr; i++)	{
		printf("\n\n\tElemento %d della struttura.\n", i);
		printf("\tService mode: %s.\n", service_info[i].service_mode);
		printf("\tService name: %s.\n", service_info[i].service_name);
		printf("\tService sfd: %d.\n", service_info[i].sfd);
		printf("\tService port: %s.\n\n", service_info[i].service_port);
	}
}

void read_configuration_file(){
	static char *conf_name = "inetd.conf";
	FILE *fp;
	services_cntr = 0;
	fp = fopen(conf_name, "r");
	if (fp == NULL)	{
		perror("\nError while opening configuration file.\n");
		exit(EXIT_FAILURE);
	}

	while (fscanf(fp, "%s %s %s %s\n", service_info[services_cntr].service_path, service_info[services_cntr].transport_protocol, service_info[services_cntr].service_port, service_info[services_cntr].service_mode) != EOF){
		char * service_name = strrchr(service_info[services_cntr].service_path, '/');
		service_name++;
		strcpy(service_info[services_cntr].service_name,service_name);
		service_info[services_cntr].pid = NULL_PID;
		services_cntr++;
	}
	fclose(fp);
}

int create_tcp_socket(structure service){
	int sfd, br, lr;
	struct sockaddr_in server_addr;

	sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sfd < 0){
		perror("Error while creating UDP socket.\n");
		exit(EXIT_FAILURE);
	}

	// Initialize server address information
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(service.service_port)); // Convert to network byte order
	server_addr.sin_addr.s_addr = INADDR_ANY;				  // Bind to any address
	br = bind(sfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (br < 0){
		perror("Error while executing bind on TCP socket.\n"); // Print error message
		exit(EXIT_FAILURE);
	}

	// Listen for incoming requests
	lr = listen(sfd, BACK_LOG);
	if (lr < 0){
		perror("Error while calling listen on TCP socket.\n");
		exit(EXIT_FAILURE);
	}

	return sfd;
}

int create_udp_socket(structure service){
	int sfd, br;
	struct sockaddr_in server_addr;

	sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sfd < 0){
		perror("Error while creating UDP socket.\n");
		exit(EXIT_FAILURE);
	}

	// Initialize server address information
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(service.service_port)); // Convert to network byte order
	server_addr.sin_addr.s_addr = INADDR_ANY;				  // Bind to any address
	br = bind(sfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (br < 0){
		perror("Error while executing bind on TCP socket.\n");
		exit(EXIT_FAILURE);
	}

	return sfd;
}

void create_sockets(){
	int i, sfd;

	for (i = 0; i < services_cntr; i++)	{
		if (strcmp(service_info[i].transport_protocol, "tcp") == 0)		{
			sfd = create_tcp_socket(service_info[i]);
		}else{
			sfd = create_udp_socket(service_info[i]);
		}
		service_info[i].sfd = sfd;
	}
}

void manage_select(char **env){
	struct timeval time_wait;
	int temp, i, max_sfd, new_sfd, pid;
	struct sockaddr_in client_addr; // struct containing client address information
	socklen_t cli_size = sizeof(client_addr);

	while(1){
		time_wait.tv_sec = 10;
		time_wait.tv_usec = 0;
		max_sfd = 0;
		FD_ZERO(&read_set);

		for (i = 0; i < services_cntr; i ++){
			if(strcmp(service_info[i].service_mode, "wait") != 0  || service_info[i].pid == NULL_PID){ 
					FD_SET(service_info[i].sfd, &read_set);
					if (service_info[i].sfd > max_sfd){ //Check for max sfd
						max_sfd = service_info[i].sfd; 
					}
			}
		}

		printf("\t---> CALLING SELECT.\n");
		temp = select(max_sfd+1, &read_set, NULL, NULL, &time_wait);
		if (temp < 0){
			if (!is_sigchld){
				printf("\nError while executing select on TCP socket.\n");
				break;
			} else {
				is_sigchld = FALSE;
			}
			
		} else if (temp == 0){
			printf("\t---> Timeout expired.\n");
		} else { 
			for (int i = 0; i < services_cntr; i++){ 
				if (FD_ISSET(service_info[i].sfd, &read_set)){ //There is a connection pending
						printf("\n -> Superserver: Selected %s service port.\n", service_info[i].service_port);

						if (strcmp(service_info[i].transport_protocol, "tcp") == 0){
							new_sfd = accept(service_info[i].sfd, (struct sockaddr *) &client_addr, &cli_size);
							if (new_sfd < 0){
								perror("\nError while executing accept.\n");
								exit(EXIT_FAILURE);
							}
						}

						pid = fork();
						
						if (pid == 0){
							//Son Process
							close(0);
							close(1);
							close(2);
							if (strcmp(service_info[i].transport_protocol, "tcp") == 0){
								close(service_info[i].sfd);
								dup(new_sfd);
								dup(new_sfd);
								dup(new_sfd);
							} else {
								dup (service_info[i].sfd);
								dup (service_info[i].sfd);
								dup (service_info[i].sfd);
							}

							if (execle(service_info[i].service_path, service_info[i].service_name, NULL, env) == -1) {
								perror("\nError while calling execle.\n");
								fflush(stdout);
								exit(EXIT_FAILURE);
							}							
						} else {
							//Father Process
							
							if (strcmp(service_info[i].transport_protocol, "tcp") == 0){
								close(new_sfd);
							}
							if (strcmp(service_info[i].service_mode, "wait") == 0){
								service_info[i].pid = pid;
								FD_CLR(service_info[i].sfd, &read_set);
							} 
						}	
						printf(" -> Superserver: torno alla select.\n\n");
						fflush(stdout);
				}
			}
		}
	}

}

int main(int argc, char **argv, char **env){
	
	read_configuration_file();
	fflush(stdout);
	create_sockets();
	print_structure();
	signal(SIGCHLD, handle_signal); 
	manage_select(env);
	return 0;

}
