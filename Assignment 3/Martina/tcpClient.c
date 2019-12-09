#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "myfunction.h"
#include <sys/time.h>

#define MAX_BUF_SIZE 1024 // Maximum size of single messages
#define FILE_NAME "init.conf" //file containing main messages information
#define TRUE 1
#define FALSE 0

typedef struct {
    char protocol_phase; // h: hello phase; m: measurement phase; b: bye phase.
    char measure_type[6]; //rtt: RTT; thpur: Throughput.
    int n_probes; //number of probes sent from client to server
    int msg_size; //size of probe's payload
    int server_delay; //time that server wait before echoing back
} service_struct;
service_struct service;

void read_init_file(){
    FILE * fp;
    char fp_line_buffer[MAX_BUF_SIZE];
    char * res; //fgets() result
    int i = 0;
    char * token[MAX_BUF_SIZE]; //buffer for strtok - token will contain the content of init.conf in each element

    /*Check init.conf for messages information*/
    fp = fopen(FILE_NAME, "r");
    if (fp == NULL){
        printf("\nError tcpClient: opening of 'init.conf' file gone wrong.\n\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    /*Read init.conf content*/
    res = fgets(fp_line_buffer, MAX_BUF_SIZE, fp);
    if (res == NULL){
        printf("\nError in tcpClient: there is a problem with the content of init.conf file.\n\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    fclose(fp); // Closing file

    token[0] = strtok(fp_line_buffer, " ");  // first call returns pointer to first part of fp_line_buffer separated by " "
    while (token[i] != NULL) {
        i++;
        token[i] = strtok(NULL, " ");  // every call with NULL uses saved fp_line_buffer value and returns next substring
    }
    
    service.n_probes = atoi(token[1]);
    service.msg_size = atoi(token[2]);
    service.server_delay = atoi(token[3]);
    strcpy(service.measure_type,token[0]);
    service.protocol_phase = 'h';  
}

char * init_buffer(){
    char * string = (char *)calloc(MAX_BUF_SIZE, sizeof(char));
    if (string == NULL){
        printf("\ntcpClient: an error occurred while calling calloc\n\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    return string;
}

int main(int argc, char *argv[]){
    struct sockaddr_in server_addr;     // struct containing server address information
    struct sockaddr_in client_addr;     // struct containing client address information
    int sfd;                            // Server socket file descriptor
    int r;                              // Result of calls
    int i;                              // Loop variable
    ssize_t byteRecv;                   // Number of bytes received
    ssize_t byteSent;                   // Number of bytes sent 
    char * receivedData;                // Data to be received
    char * message;                    // Data to be sent
    char * payload;                     // String that contains payload
    
    struct timeval start, end;          // Variables for timer
    double rtt[MAX_BUF_SIZE];           // Probe's rtt array
    double sum_rtt = 0;                 // Sum of all probe's rtt

    if (argc != 3) {
        printf("\nError tcpClient: wrong number of parameters in function main().\n\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    /*Read init.conf and save its infomation*/
    read_init_file();

    sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd < 0){
        perror("\nError tcpClient: socket file descriptor not created.\n\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2])); //Third parameter passed on main() function
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); //Second parameter passed on main() function

    r = connect(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (r < 0){
        perror("\nError tcpClient: connection with server gone wrong.\n\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    /* phase h ---------------------------------------------------------------------------------------------------------- */
    message = init_buffer();
    sprintf(message, "h %s %d %d %d\n", service.measure_type, service.n_probes, service.msg_size, service.server_delay);
    send(sfd, message, strlen(message), 0);
    printf("\ntcpClient sent message to server: %s ", message);
    fflush(stdout);

    receivedData = init_buffer();
    
    byteRecv = recv(sfd, receivedData, MAX_BUF_SIZE, 0); // I don't need multiple reads in this phase because I only receive short answers from server
    if(byteRecv == -1){
        perror("\nError tcpClient: an error occurred while receiving data from the socket.\n\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    printf("\ntcpClient received message from server: %s \n", receivedData);
    fflush(stdout);
    if (strncmp(receivedData, "200", 3) == 0){ // Received OK
        service.protocol_phase = 'm';
        printf("\ntcpClient received ok, switch phase into measurement.\n\n");
        fflush(stdout);
    } else { // Received FAIL
        //Non mi ricordo cosa devo fare qui--------------- forse chiudere forse boh
        printf("\ntcpClient: server rejected my hello message.\n\n");
        return 0;
    }
    free(receivedData);
    free(message);

    /* phase m ---------------------------------------------------------------------------------------------------------- */
    payload = (char *)calloc(service.msg_size, sizeof(char)); // init of payload
    if (payload == NULL){
        printf("\ntcpClient: an error occurred while calling calloc\n\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    message = (char *)calloc(service.msg_size + 10, sizeof(char)); // init of message bigger enough to  contain message payload
    if (message == NULL){
        printf("\ntcpClient: an error occurred while calling calloc\n\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i<service.msg_size; i++){ // Fill payload with msg_size size
        payload[i] = 'x';
    }

    for(i = 0; i < service.n_probes; i++){ // Send probes to server
        memset(message,0,strlen(message)); //flushing message content
        sprintf(message, "m %d %s\n", (i+1), payload);

        gettimeofday(&start, NULL);

        send(sfd, message, strlen(message), 0);
        printf("tcpClient sent message to server: %s", message);
        fflush(stdout);
        
        // reuse of message on socket because I receive back my probes
        memset(message,0,strlen(message)); //flushing message content
        
        byteRecv = recv(sfd, message, service.msg_size + 10, 0);
        if(byteRecv == -1){
            perror("\nError tcpServer: an error occurred while receiving data from the socket.\n");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        
        printf("tcpClient received message from server: %s", message);
        fflush(stdout);
        if (strncmp(message, "400", 3) == 0 || strncmp(message, "m", 1) != 0){
            printf("\ntcpClient: server rejected my probe message. Closing.\n\n");
            exit(EXIT_FAILURE);
        }

        gettimeofday(&end,NULL);
        
        rtt[i] = (end.tv_sec + (end.tv_usec/1000000.0))-(start.tv_sec + (start.tv_usec/1000000.0)); //Rtt of single probes
        //printf("\nRTT of probe_seq_number %d is %lf\n", i+1, rtt[i]);
    }
    // Print measures ----------------------------------------------
    free(payload);
    free(message);

    for (i = 0; i < service.n_probes; i++){
        printf("Probe %d -> rtt: %lf ms\n",i+1, rtt[i]);
        sum_rtt += rtt[i];
    }
    printf("\nAverage rtt is: %lf ms\n", sum_rtt/service.n_probes);
    printf("Thput is: %lf kbps", (service.msg_size / 1000)/sum_rtt);

    service.protocol_phase = 'b';
    printf("\ntcpClient: measurement phase went ok, switch phase into bye.\n");
    fflush(stdout);

    /* phase b ---------------------------------------------------------------------------------------------------------- */
    message = init_buffer();
    sprintf(message, "b\n");
    send(sfd, message, strlen(message), 0);
    printf("\ntcpClient sent message to server: %s", message);
    fflush(stdout);
    memset(receivedData,0,strlen(receivedData)); //flushing receivedData content
    byteRecv = recv(sfd, receivedData, MAX_BUF_SIZE, 0); // I don't need multiple reads in this phase because I only receive short answers from server
    if(byteRecv == -1){
        perror("\nError tcpClient: an error occurred while receiving data from the socket.\n\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    printf("\ntcpClient received message from server: %s", receivedData);
    fflush(stdout);

    if (strncmp(receivedData, "200", 3) == 0){
        printf("\ntcpClient: confirm received from server. Closing.\n\n");
        exit(EXIT_SUCCESS);
    } else {
        printf("\ntcpClient: an error occurred into bye phase. Closing\n");
    }
    free(message);
    free(receivedData);

    return 0;
}
