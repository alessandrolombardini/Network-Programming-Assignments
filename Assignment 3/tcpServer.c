#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "myfunction.h"

#define MAX_BUF_SIZE 1024
#define BACK_LOG 2 // Maximum queued requests
#define STRING_SPLITTER " "
#define BOOL int 
#define TRUE 1
#define FALSE 0
#define NO_CONNECTION_STATE 0
#define READY_TO_REQUEST 1 
#define ECHO_STATE 2
#define BYE_STATE 3

BOOL manageMessage(char * mess);
BOOL checkHelloMessage(char * mess); 
BOOL checkProbeMessage(char mess[]);
BOOL checkByeMessage(char mess[]);
BOOL manageHelloMessage(char * message);
BOOL manageProbeMessage(char * message);
BOOL manageByeMessage(char * message);
BOOL isNumber(char * token);
int getNumber(char * token);
void sendMessage(char * message);
void printService();
void initilizeService();

// Service structure definition goes here
typedef struct node {
	char protocolPhase[2];
	char measureType[4];
	int nProbes;
	int messageSize;
	int serverDelay;
    int connectionFD; /* File descriptor of socket */
    int phaseNumber; /* 0: Ready to accept connection | 
                        1: Ready to accept service request | 
                        2: Ready to echo back | 
                        3: Ready response to bye */
    int probeSequenceNumberAwaited; /* Sequence awaited: have to be in phase 2 to be usefull*/
} serviceNode;

serviceNode service;

void initilizeService() {
    /* First state of machine - other informations are invalid in NO_CONNECTION_STATE */
    service.phaseNumber = NO_CONNECTION_STATE;
    service.probeSequenceNumberAwaited = 1;
}

int main(int argc, char *argv[]){
    struct sockaddr_in server_addr; // struct containing server address information
    struct sockaddr_in client_addr; // struct containing client address information
    int serverFD; // Server socket filed descriptor
    int acceptFD; // Client communication socket - Accept result
    int bindResult; // Bind result
    int listenResult; // Listen result
    int i;
    int stop = 0;
    ssize_t byteRecv; // Number of bytes received
    ssize_t byteSent; // Number of bytes to be sent
    socklen_t cli_size;
    char receivedData [MAX_BUF_SIZE]; // Data to be received
    char sendData [MAX_BUF_SIZE]; // Data to be sent

    if (argc != 2) {
            printf("\nErrore numero errato di parametri\n");
            printf("\n%s <server port>\n", argv[0]);
            exit(EXIT_FAILURE);
    }
    serverFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverFD < 0){
        perror("socket error"); 
        exit(EXIT_FAILURE);
    }
    // Initialize server address information
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1])); 
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    // Binding address
    bindResult = bind(serverFD, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (bindResult < 0){
        perror("bind error"); 
        exit(EXIT_FAILURE);
    }
    cli_size = sizeof(client_addr);
    // Listen for incoming requests
    listenResult = listen(serverFD, BACK_LOG);
    if (listenResult < 0){
        perror("listen error"); 
        exit(EXIT_FAILURE);
    }
    initilizeService();
    while(TRUE){
        // Wait for incoming requests
        acceptFD = accept(serverFD, (struct sockaddr *) &client_addr, &cli_size);
        if (acceptFD < 0){
            perror("accept error"); 
            exit(EXIT_FAILURE);
        }
        
        printf("Connessione eseguita (IP: %s | Porta: %d)\n",  inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        // Accept connection and be ready to accept request of service
        service.phaseNumber = READY_TO_REQUEST;
        service.connectionFD = acceptFD;
        while(service.phaseNumber != NO_CONNECTION_STATE){  // While there is communication keep connection alive
            char data[MAX_BUF_SIZE]="\0"; /* In this way i'm sure to take only byteRecv byte */
            byteRecv = recv(service.connectionFD, data, MAX_BUF_SIZE, 0);
            if (byteRecv < 0){
                perror("recv");
                exit(EXIT_FAILURE);
            }
            printf("(SERVER) Message received: %s\n", data);
            if(service.serverDelay > 0){
                sleep(service.serverDelay);
            }
            if(manageMessage(data)==FALSE) {
                initilizeService();
            }
        }
        printf("Connection close\n");
        close(acceptFD);
        initilizeService();
    }
}

void sendMessage(char * message) {
    send(service.connectionFD, message, strlen(message), 0);
    printf("(SERVER) Message sent: %s\n", message);
}

BOOL manageMessage(char mess[]){
    BOOL check = TRUE;
    /* Check message with message format awaited */
    if(service.phaseNumber == READY_TO_REQUEST){
       return manageHelloMessage(mess);
    } else if(service.phaseNumber == ECHO_STATE){
        return manageProbeMessage(mess);
    } else if(service.phaseNumber == BYE_STATE) {
        return manageByeMessage(mess);
    }
    return check;
}

BOOL manageHelloMessage(char * message){
    if(checkHelloMessage(message)){
        sendMessage("200 OK - Ready");
        service.phaseNumber = ECHO_STATE;
        return TRUE;
    }else{
        sendMessage("404 ERROR - Invalid Hello message");
        return FALSE;
    }
}
BOOL manageProbeMessage(char * message){
    if(checkProbeMessage(message)){
        sendMessage(message);
        return TRUE;
    }else{
        sendMessage("404 ERROR - Invalid Measurement message");
        return FALSE;
    }
}
BOOL manageByeMessage(char * message){
    if(checkByeMessage(message)){
        sendMessage("200 OK - Closing");
        initilizeService();
    }else{
        /* Don't know how to do when the message is wrong: close however. */
        return FALSE;
    }
}


BOOL checkHelloMessage(char mess[]){
    int numero;
    char * token;
    char message[strlen(mess)];

    strcpy(message, mess);
    /* Check protocol phase */
    token = strtok(message, STRING_SPLITTER);
    if(token == NULL || strcmp(token, "h") != 0){
        return FALSE;
    }
    strcpy(service.protocolPhase, token);
    /* Check measure type */
    token = strtok(NULL, STRING_SPLITTER);
    if(token == NULL || (strcmp(token, "rtt") != 0 && strcmp(token, "thput") != 0) ){
        return FALSE;
    }
    strcpy(service.measureType, token);
    /* Check number of probes */
    token = strtok(NULL, STRING_SPLITTER);
    if(token == NULL || !isNumber(token) || getNumber(token)<0){
        return FALSE;
    }
    service.nProbes = getNumber(token);
    /* Check message size */
    token = strtok(NULL, STRING_SPLITTER);
    if(token == NULL || !isNumber(token) || getNumber(token)<0){
        return FALSE;
    }
    service.messageSize = getNumber(token);
    /* Check server delay */
    token = strtok(NULL, STRING_SPLITTER);
    if(token == NULL || !isNumber(token) || getNumber(token)<0){
        return FALSE;
    }
    service.serverDelay = getNumber(token);
    return TRUE;
} 

BOOL checkProbeMessage(char mess[]){
    char * token;
    char message[strlen(mess)];

    strcpy(message, mess);
    /* Check protocol phase */
    token = strtok(message, STRING_SPLITTER);
    if(token == NULL || strcmp(token, "m") != 0){
        return FALSE;
    }
    /* Check probe sequence number */
    token = strtok(NULL, STRING_SPLITTER);
    if(token != NULL && isNumber(token)){
        int numero = atoi(token);
        if(numero != service.probeSequenceNumberAwaited){
            return FALSE;
        }
    } else {
        return FALSE;
    }
    /* Check number of bytes of payload*/
    int numberOfBytes = 0;
    token = strtok(NULL, "");
    if(token != NULL){
        numberOfBytes = strlen(token);
    }
    if(service.messageSize != numberOfBytes){
        return FALSE;
    }
    /* All is ok */
    if(service.probeSequenceNumberAwaited == service.nProbes){
        service.phaseNumber = BYE_STATE;
    }
    service.probeSequenceNumberAwaited++;
    return TRUE;
} 

BOOL checkByeMessage(char mess[]){
    char * token;
    char message[strlen(mess)];

    strcpy(message, mess);
    /* Check protocol phase */
    token = strtok(message, STRING_SPLITTER);
    if(token == NULL || strcmp(token, "b") != 0){
        return FALSE;
    }
    /* All is ok */
    return TRUE;
} 

/* Check if a string is a number and if that number is greater than zero */
BOOL isNumber(char * token){
    int numero = atoi(token);
    if(numero == 0 && token[0] != '0'){
        return FALSE;
    }
    return TRUE;
}

/* Get number by a string */
int getNumber(char * token){
    return atoi(token);
}

void printService(){
    printf("Protocol phase: %s\n",service.protocolPhase);
    printf("Measure type: %s\n",service.measureType);
    printf("Number of probes: %i\n",service.nProbes);
    printf("Message size: %i\n",service.messageSize);
    printf("Server delay: %i\n",service.serverDelay);
}