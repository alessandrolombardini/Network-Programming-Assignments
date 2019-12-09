#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "myfunction.h"

#define MAX_BUF_SIZE 1024 
#define TRUE 1
#define FALSE 0
#define BACK_LOG 2
#define HP_OK_RESPONSE "200 OK - Ready"
#define MP_FAIL_RESPONSE "400 Error - Invalid measurement message"
#define HP_FAIL_RESPONSE "400 Error - Invalid hello message"
#define BP_OK_RESPONSE "200 OK - Closing"
#define _BSD_SOURCE

typedef struct {
    char protocol_phase; // h: hello phase; m: measurement phase; b: bye phase.
    char measure_type[6]; //rtt: RTT; thpur: Throughput.
    int n_probes; //number of probes sent from client to server
    int msg_size; //size of probe's payload
    int server_delay; //time that server wait before echoing back
} service_struct;
service_struct service;

int probes_counter;

int hello_message_validation(char * message){
	char * pointer;
    int i = 0;
    int number; // Value returned by atoi() function

    if (message[0] != 'h'){
        return FALSE;
    } 
    
    pointer = strchr(message, ' ');
    pointer++;

    if (strncmp(pointer, "rtt", 3) != 0 && strncmp(pointer, "thput", 5) != 0){
        return FALSE;
    } else {
        i = 0;
        while (pointer[i] != ' '){
            service.measure_type[i] = pointer[i];
            i++;
        }
    }

    pointer = strchr(pointer, ' ');
    pointer++;

    number = atoi(pointer);
    if (number == 0){ //The number of probes can't be 0
        return FALSE;
    } else {
        service.n_probes = number;
    }

    pointer = strchr(pointer, ' ');
    pointer++;

    number = atoi(pointer);
    if (number == 0){ //Probe's payload size can't be 0
        return FALSE;
    } else {
        service.msg_size = number;
    }

    pointer = strchr(pointer, ' ');
    pointer++;

    service.server_delay = atoi(pointer);
    
    return TRUE;
}

int measurement_message_validation(char * message){
    int number = 0;
    char * pointer;

    if (message[0] != 'm'){
        return FALSE;
    }

    pointer = strchr(message, ' ');
    pointer++;

    number = atoi(pointer);
    if (number == 0 || number != probes_counter || number > service.n_probes){
        printf("\nError tcpServer: measurement message doesn't the right probe sequence number\n");
        return FALSE;
    } 

    pointer = strchr(pointer, ' ');
    pointer++;
    while (pointer[number] != '\n'){
        number ++;
    }
    if (number != service.msg_size){ //The size of probe payload
        printf("\nError tcpServer: measurement message payload doesn't right number of bytes: %d\n", number);
        return FALSE;
    } 

    return TRUE;
}

char * init_buffer(){
    char * string = (char *)calloc(MAX_BUF_SIZE, sizeof(char));
    if (string == NULL){
        printf("\ntcpServer: an error occurred while calling calloc\n\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    return string;
}

int main(int argc, char *argv[]){
	struct sockaddr_in server_addr;         /* Struct containing server address information */
    struct sockaddr_in client_addr;         /* struct containing client address information */
    int sfd;                           /* Server socket filed descriptor */
    int connection_sfd;                           /* Accept result */
    int bindResult;                         /* Bind result */
    int listenResult;                       /* Listen result */
    ssize_t byteRecv;                       /* Number of bytes received */
    ssize_t byteSent;                       /* Number of bytes to be sent */
    socklen_t cli_size;
    char sendData [MAX_BUF_SIZE];           /* Buffer of data to be sent */
    char * receivedData;                    /* Buffer of received data */
    char * receivedMessage;                  // Complete message received - This could be longer than buffer size
    int recvBufferSize;                    // Buffer size

    
    /* Check if number of params passed is correct */
    if (argc != 2) {
        printf("\nError tcpServer: wrong number of parameters.\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    /* Open TCP socket */
    sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd < 0){
        perror("\nError tcpServer: something went wrong with the creation of a new socket.\n"); 
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1])); 
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    bindResult = bind(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (bindResult < 0){
        perror("\nError tcpServer: something went wrong with bind().\n"); 
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    cli_size = sizeof(client_addr);
    listenResult = listen(sfd, BACK_LOG);
    if (listenResult < 0){
        perror("\nError tcpServer: something went wrong with listen().\n"); 
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    while(TRUE){
        connection_sfd = accept(sfd, (struct sockaddr *) &client_addr, &cli_size);
        if (connection_sfd < 0){
            perror("\nError tcpServer: something went wrong with accept().\n"); 
            fflush(stdout);
            exit(EXIT_FAILURE);
        }

        printf ("\n\n\t\t CONNECTION ESTABLISHED WITH CLIENT: \n\t\t\t IP: %s\n\t\t\t PORT: %d\n\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        /*Initializing service structure*/
        service.protocol_phase = 'h'; 
        probes_counter = 0;
        receivedData = init_buffer();

        while(TRUE){

            // ------------------------------------------------------------------------------------------ Receiving a new message from buffer
            memset(receivedData,0,strlen(receivedData)); //flushing receivingData content
            receivedMessage = init_buffer(); 
            recvBufferSize = MAX_BUF_SIZE;
            
            byteRecv = recv(connection_sfd, receivedData, MAX_BUF_SIZE, 0);
            if(byteRecv == -1){
                perror("\nError tcpServer: an error occurred while receiving data from the socket.\n");
                fflush(stdout);
                close(connection_sfd);
                exit(EXIT_FAILURE);
            }
            strcpy(receivedMessage, receivedData);
            while (recvBufferSize <= service.msg_size && service.protocol_phase == 'm') {
                recvBufferSize = recvBufferSize + MAX_BUF_SIZE;
                
                receivedMessage = (char *) realloc(receivedMessage, recvBufferSize * sizeof(char));
                if (receivedMessage == NULL){
                    printf("\ntcpServer: an error occurred while calling realloc\n\n");
                    fflush(stdout);
                    close(connection_sfd);
                    exit(EXIT_FAILURE);
                }
                memset(receivedData,0,strlen(receivedData)); // flushing receivingData content
                byteRecv = recv(connection_sfd, receivedData, MAX_BUF_SIZE, 0);
                if(byteRecv == -1){
                    perror("\nError tcpServer: an error occurred while receiving data from the socket.\n");
                    fflush(stdout);
                    close(connection_sfd);
                    exit(EXIT_FAILURE);
                }
                strcat(receivedMessage, receivedData);
                
            }
            
            printf("\ntcpServer received message from client %s.", receivedMessage);
            fflush(stdout);

            // ------------------------------------------------------------------------------------------ Looking for the content of the message
            if (service.protocol_phase == 'h'){ //   hello phase ------------------------------------------------------------------------------------------
                if (receivedMessage[0] == 'h'){ //It's an hello message, other messages will be discharged
                    if (hello_message_validation(receivedMessage)){
                        send(connection_sfd, HP_OK_RESPONSE, strlen(HP_OK_RESPONSE), 0);
                        printf("\ntcpServer sent message to client: %s \n", HP_OK_RESPONSE);
                        service.protocol_phase = 'm'; /* incrementing phase */
                        printf("Service phase changed to measurement succesfully.\n");
                    } else {
                        send(connection_sfd, HP_FAIL_RESPONSE, strlen(HP_FAIL_RESPONSE), 0);
                        printf("\ntcpServer sent message to client: %s \n", HP_FAIL_RESPONSE);
                        close(connection_sfd);
                        break;
                    }

                    fflush(stdout);
                } else {
                    printf("\ntcpServer: came unknown message. Closing.\n");
                    fflush(stdout);
                    close(connection_sfd);
                    break;
                }
            } else if (service.protocol_phase == 'm') { //  measurement phase ------------------------------------------------------------------------------------------
                if (receivedMessage[0] == 'm'){
                    probes_counter ++;
                    
                    if (measurement_message_validation(receivedMessage)){
                        if(service.server_delay != 0){
                            usleep(service.server_delay);
                        }
                        send(connection_sfd, receivedMessage, strlen(receivedMessage), 0);
                        printf("\ntcpServer: sent probe back.\n");
                        if (probes_counter == service.n_probes){
                            printf("\ntcpServer: switch into bye phase.\n");
                            service.protocol_phase = 'b';
                        }
                    } else {
                        send(connection_sfd, MP_FAIL_RESPONSE, strlen(MP_FAIL_RESPONSE), 0);
                        printf("\ntcpServer sent message to client: %s \n\n", HP_FAIL_RESPONSE);
                        close(connection_sfd);
                        break;
                    }
                    fflush(stdout);
                } else {
                    printf("\ntcpServer: came unknown message. Closing.\n");
                    fflush(stdout);
                    close(connection_sfd);
                    break;
                }
            } else if (service.protocol_phase == 'b') {
                if (receivedMessage[0] == 'b'){ //   bye phase ------------------------------------------------------------------------------------------
                    if(probes_counter == service.n_probes){
                        // Arrived bye message and probes_counter is the same as n_probes declared into hello message
                        send(connection_sfd, BP_OK_RESPONSE, strlen(BP_OK_RESPONSE), 0);
                        printf("\ntcpServer sent message to client: %s \n", BP_OK_RESPONSE);
                        printf("tcpServer: received bye message. Closing.\n");
                        fflush(stdout);
                        close(connection_sfd);
                        break;
                    } else {
                        //Server received bye message but probes_counter isn't the same as declared into hello message.
                        printf("tcpServer: received bye message but probes weren't the right number. Closing.\n");
                        fflush(stdout);
                        close(connection_sfd);
                        break;
                    }
                }
            } else { 
                break;
            }

            free(receivedMessage);
        }
        free(receivedData);
    }
  return 0;
}
