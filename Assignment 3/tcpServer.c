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

#define MAX_BUF_SIZE 32768          /* Maximum size of TCP messages */
#define BACK_LOG 2                  /* Maximum queued requests */
#define WAIT_CONNECTION    0        /* First state */
#define WAIT_HELLO_MESSAGE 1        /* Second state */
#define WAIT_PROBE_MESSAGE 2        /* Third state */
#define WAIT_BYE_MESSAGE   3        /* Fourth state */
#define STRING_SPLITTER " "         /* String splitter of the hello message */
#define BOOL int 
#define TRUE 1
#define FALSE 0

BOOL manageMessage(char * mess);            /* Call to manage every message passed */
BOOL checkHelloMessage(char * mess);        /* Check if the message passed is a valid hello message */
BOOL checkProbeMessage(char mess[]);        /* Check if the message passed is a valid probe message */
BOOL checkByeMessage(char mess[]);          /* Check if the message passed is a valid bye message */
BOOL manageHelloMessage(char * message);    /* Manage the response to the hello message */
BOOL manageProbeMessage(char * message);    /* Manage the response to the probe message */
BOOL manageByeMessage(char * message);      /* Manage the response to the bye message */
BOOL isNumber(char * token);                /* Check if a string is a number */
int getNumber(char * token);                /* Get a number by a string */
void sendMessage(char * message);           /* Send the message passed to the server */
void printService();                        /* Print all information about the service requested by the client */
void initilizeService();                    /* Set the server on the intial state */

/* Service structure definition goes here */
typedef struct node {
	char measureType[4];    /* Service requested - RTT or THROUGHPUT */
	int nProbes;      /* Number of probes to send */
	int messageSize;  /* Size of payload to send in the probe message */
	int serverDelay;  /* Delay of response to the probe message (in seconds) */
    int connectionFD; /* File descriptor of socket */
    int phaseNumber;  /* 0: Ready to accept connection | 
                         1: Ready to accept service request | 
                         2: Ready to echo back | 
                         3: Ready response to bye */
    int probeSequenceNumberAwaited; /* Sequence awaited: have to be in phase 2 to be usefull*/
} serviceNode;
serviceNode service;

void initilizeService() {
    /* First state of machine - other informations are invalid in WAIT_CONNECTION */
    service.phaseNumber = WAIT_CONNECTION;
    service.probeSequenceNumberAwaited = 1;
    service.serverDelay = 0;
}

int main(int argc, char *argv[]){
    struct sockaddr_in server_addr;         /* Struct containing server address information */
    struct sockaddr_in client_addr;         /* struct containing client address information */
    int serverFD;                           /* Server socket filed descriptor */
    int acceptFD;                           /* Accept result */
    int bindResult;                         /* Bind result */
    int listenResult;                       /* Listen result */
    ssize_t byteRecv;                       /* Number of bytes received */
    ssize_t byteSent;                       /* Number of bytes to be sent */
    socklen_t cli_size;
    char sendData [MAX_BUF_SIZE];           /* Buffer of data to be sent */
    char * receivedData;                    /* Buffer of received data */
    char * completeMessageReceived;         /* Complete message received: this is the sum of all pieces that can arrive*/ 
    BOOL messageIsComplete = FALSE;         /* Check if the message is all arrived or not */

    /* Check if number of params passed is correct */
    if (argc != 2) {
        printf("\nErrore numero errato di parametri\n");
        printf("\n%s <server port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    /* Open TCP server */
    serverFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverFD < 0){
        perror("socket error"); 
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1])); 
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    bindResult = bind(serverFD, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (bindResult < 0){
        perror("bind error"); 
        exit(EXIT_FAILURE);
    }
    cli_size = sizeof(client_addr);
    listenResult = listen(serverFD, BACK_LOG);
    if (listenResult < 0){
        perror("listen error"); 
        exit(EXIT_FAILURE);
    }
    /* Set the first state of the server */
    initilizeService();
    /* Start server */
    while(TRUE){
        /* Wait for incoming requests by clients */
        acceptFD = accept(serverFD, (struct sockaddr *) &client_addr, &cli_size);
        if (acceptFD < 0){
            perror("accept error"); 
            exit(EXIT_FAILURE);
        }
        printf("Connessione eseguita (IP: %s | Porta: %d)\n",  inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        /* Be ready to accept request of service */
        service.phaseNumber = WAIT_HELLO_MESSAGE;
        service.connectionFD = acceptFD;
        while(service.phaseNumber != WAIT_CONNECTION ){  /* While there is connection keep connection alive */
            completeMessageReceived = (char *)calloc(MAX_BUF_SIZE, sizeof(char));
            /** In this phase I need to check if the message just arrived is complete:
                this becouse it's possible that it doesnt' arrive in just one message,
                but in multiple message **/
            for(;messageIsComplete==FALSE;){
                /* Receive one piece of the message */
                receivedData = (char *)calloc(MAX_BUF_SIZE, sizeof(char));
                byteRecv = recv(service.connectionFD, receivedData, MAX_BUF_SIZE, 0);
                if (byteRecv < 0){
                    perror("recv");
                    close(service.connectionFD);
                    exit(EXIT_FAILURE);
                }
                strcat(completeMessageReceived, receivedData);
                free(receivedData);
                /* Check if the message received at the moment is complete (has \n) */
                if(completeMessageReceived[strlen(completeMessageReceived) - 1] == '\n') { 
                    printf("(SERVER) Message received: %s", completeMessageReceived);
                    usleep(service.serverDelay > 0 ? service.serverDelay : 0);
                    if(manageMessage(completeMessageReceived)==FALSE) {
                        close(service.connectionFD);
                        initilizeService();
                    }
                    free(completeMessageReceived);
                    messageIsComplete=TRUE;
                }
            } 
            messageIsComplete=FALSE;
        }
        /* Close connection */
        printf("Connection close\n");
        close(acceptFD);
    }
}

void sendMessage(char * message) {
    send(service.connectionFD, message, strlen(message), 0);
    /* If the message has alredy the character \n in the end, the printf doesn't add one */
    if(message[strlen(message)-1]=='\n'){
        printf("(SERVER) Message sent: %s", message);
    } else {
        printf("(SERVER) Message sent: %s\n", message);
    }
}

BOOL manageMessage(char mess[]){
    BOOL check = TRUE;
    /* Compare message with the message format awaited and return if it's valid or not */
    if(service.phaseNumber == WAIT_HELLO_MESSAGE){
        return manageHelloMessage(mess);
    } else if(service.phaseNumber == WAIT_PROBE_MESSAGE){
        return manageProbeMessage(mess);
    } else if(service.phaseNumber == WAIT_BYE_MESSAGE)   {
        return manageByeMessage(mess);
    }
    return check;
}

BOOL manageHelloMessage(char * message){
    if(checkHelloMessage(message)){
        sendMessage("200 OK - Ready");
        service.phaseNumber = WAIT_PROBE_MESSAGE;
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
    } else {
        sendMessage("404 ERROR - Invalid Measurement message");
        return FALSE;
    }
}
BOOL manageByeMessage(char * message){
    if(checkByeMessage(message)){
        sendMessage("200 OK - Closing");
        close(service.connectionFD);
        initilizeService();
        return TRUE;
    } else{
        /* Don't know how to do when the message is wrong: close however. */
        return FALSE;
    }
}


BOOL checkHelloMessage(char mess[]){
    char * token;
    char message[strlen(mess)];

    strcpy(message, mess);
    /* Check protocol phase */
    token = strtok(message, STRING_SPLITTER);
    if(token == NULL || strcmp(token, "h") != 0){
        return FALSE;
    }
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
        if(token[strlen(token)-1]!='\n'){
            return FALSE;
        }
        numberOfBytes = strlen(token)-1;
    }
    if(service.messageSize != numberOfBytes){
        return FALSE;
       }
     /* All is ok - set next phase */
    if(service.probeSequenceNumberAwaited == service.nProbes){
     service.phaseNumber = WAIT_BYE_MESSAGE;  
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
    if(token == NULL || strcmp(token, "b\n") != 0){
        return FALSE;
    }
    /* All is ok */
    return TRUE;
} 

BOOL isNumber(char * token){
    int numero = atoi(token);
    if(numero == 0 && token[0] != '0'){
        return FALSE;
    }
    return TRUE;
}

int getNumber(char * token){
    return atoi(token);
}

void printService(){
    printf("Measure type: %s\n",service.measureType);
    printf("Number of probes: %i\n",service.nProbes);
    printf("Message size: %i\n",service.messageSize);
    printf("Server delay: %i\n",service.serverDelay);
}