#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "myfunction.h"

#define MAX_BUFFER_SIZE 1024 // Maximum size of UDP messages
#define INPUT_FILE_NAME "init.conf"
#define STRING_SPLITTER " "
#define PAYLOAD_CHARACTER 'a'
#define BOOL int 
#define TRUE 1
#define FALSE 0

BOOL checkHelloMessage(char mess[]);
void parseHelloMessage(char mess[]);
void sendHelloMessage();
void sendProbeMessages();
void sendByeMessage();

BOOL isNumber(char * token);
int getNumber(char * token);

// Service structure definition goes here
typedef struct node {
	char protocolPhase[2];
	char measureType[4];
	int nProbes;
	int messageSize;
	int serverDelay;
  int serverFD;
} serviceNode;

serviceNode service;

int main(int argc, char *argv[]){
  struct sockaddr_in server_addr; // struct containing server address information
  struct sockaddr_in client_addr; // struct containing client address information
  int serverFD; // Server socket filed descriptor
  int connectResult; // Connect result
  int stop = 0;
  ssize_t byteRecv; // Number of bytes received
  ssize_t byteSent; // Number of bytes sent
  size_t msgLen; 
  socklen_t serv_size;
  char receivedData [MAX_BUFFER_SIZE]; // Data to be received
  char sendData [MAX_BUFFER_SIZE]; // Data to be sent 

  if (argc != 3) {
		printf("\nErrore numero errato di parametri\n");
		printf("\n%s <server IP (dotted notation)> <server port>\n", argv[0]);
		exit(EXIT_FAILURE);
  }
  // Open connection with the tcp server
  serverFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (serverFD < 0){
  	perror("(CLIENT) Socket error"); 
  	exit(EXIT_FAILURE);
  }
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(atoi(argv[2]));
  server_addr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_size = sizeof(server_addr);
  connectResult = connect(serverFD, (struct sockaddr *) &server_addr, sizeof(server_addr));
  if (connectResult < 0){
    perror("(CLIENT) Connect error"); 
    exit(EXIT_FAILURE);
  }
  service.serverFD = serverFD;

  sendHelloMessage();
  sendProbeMessages();
  sendByeMessage();

  return 0;
}


/* Send bye message to the server and receive its response */
void sendByeMessage(){
  char receivedData[MAX_BUFFER_SIZE]="\0";
  send(service.serverFD, "b", 1, 0);
  printf("(CLIENT) Message sent: b\n");
  recv(service.serverFD, receivedData, MAX_BUFFER_SIZE, 0);
  printf("(CLIENT) Message received: %s\n", receivedData);
  if(strcmp(receivedData, "200 OK - Closing") != 0){
    printf("(CLIENT) Error: server reject bye message\n");
    exit(EXIT_FAILURE);
  } 
}

/* Send probe message to the server and receive its response */
void sendProbeMessages(){
  char payload[MAX_BUFFER_SIZE];
  int i;
  
  // Create appropriate payload
  for(i = 0; i < service.messageSize; i++){
    payload[i] = PAYLOAD_CHARACTER;
  }
  payload[i]='\0'; /* I don't know why it works */
  // Manage probe messages
  for(i = 0; i < service.nProbes; i++){
    // Create probe message
    char completeMessage[MAX_BUFFER_SIZE] = "m ";
    char sequenceNumber[MAX_BUFFER_SIZE];
    sprintf(sequenceNumber, "%d ", i+1);
    strcat(completeMessage, sequenceNumber);
    strcat(completeMessage, payload);
    // Send probe message and check its responde
    char receivedData[MAX_BUFFER_SIZE]="\0";
    send(service.serverFD, completeMessage, strlen(completeMessage), 0);
    printf("(CLIENT) Message sent: %s\n", completeMessage);
    recv(service.serverFD, receivedData, MAX_BUFFER_SIZE, 0);
    printf("(CLIENT) Message received: %s\n", receivedData);
    if(strcmp(receivedData, completeMessage) != 0){
      printf("(CLIENT) Error: server reject probe message\n");
      exit(EXIT_FAILURE);
    }   
  }
}

/* Send hello message to the server and receive its response */
void sendHelloMessage(){
  char receivedData [MAX_BUFFER_SIZE]; // Data to be received
  char buffer[MAX_BUFFER_SIZE];
  FILE * fileFD;

  // Read hello message by file
  fileFD = fopen(INPUT_FILE_NAME, "r");
  if(fileFD == NULL){
    perror("(CLIENT) Opening file error\n");
    exit(EXIT_FAILURE);
  }
  fgets(buffer, MAX_BUFFER_SIZE, fileFD);
  buffer[strcspn(buffer, "\n")] = ' ';
  fclose(fileFD);
  // Check hello message and parse it in the data structure
  if(checkHelloMessage(buffer)){
    parseHelloMessage(buffer);
  }else{
    perror("(CLIENT) Error: Hello message format isn't correct \n");
    exit(EXIT_FAILURE);
  }
  // Send hello message to the server and check its response
  send(service.serverFD, buffer, strlen(buffer), 0);
  printf("(CLIENT) Message sent: %s\n", buffer);
  recv(service.serverFD, receivedData, MAX_BUFFER_SIZE, 0);
  printf("(CLIENT) Message received: %s\n", receivedData);
  if(strncmp(receivedData, "200 OK - Ready", strlen(receivedData)) != 0){
      printf("(CLIENT) Error: server reject hello message\n");
      exit(EXIT_FAILURE);
  }
}

/* Extract informations by hello message */
BOOL checkHelloMessage(char mess[]){
    char * token;
    char message[strlen(mess)];

    strcpy(message, mess);
    /* Check protocol phase character */
    token = strtok(message, STRING_SPLITTER);
    if(token == NULL || strcmp(token, "h") != 0){
        return FALSE;
    }
    /* Check measure type characters */
    token = strtok(NULL, STRING_SPLITTER);
    if(token == NULL || (strcmp(token, "rtt") != 0 && strcmp(token, "thput") != 0) ){
        return FALSE;
    }
    /* Check number of probes characters */
    token = strtok(NULL, STRING_SPLITTER);
    if(token == NULL || !isNumber(token) || getNumber(token)<0){
        return FALSE;
    }
    /* Check message size characters */
    token = strtok(NULL, STRING_SPLITTER);
    if(token == NULL || !isNumber(token) || getNumber(token)<0){
        return FALSE;
    }
    /* Check server delay characters */
    token = strtok(NULL, STRING_SPLITTER);
    if(token == NULL || !isNumber(token) || getNumber(token)<0){
        return FALSE;
    }
    return TRUE;
} 

/* Extract informations by hello message */
void parseHelloMessage(char mess[]){
    char * token;
    char message[strlen(mess)];

    strcpy(message, mess);
    /* Check protocol phase character */
    token = strtok(message, STRING_SPLITTER);
    strcpy(service.protocolPhase, token);
    /* Check measure type characters */
    token = strtok(NULL, STRING_SPLITTER);
    strcpy(service.measureType, token);
    /* Check number of probes characters */
    token = strtok(NULL, STRING_SPLITTER);
    service.nProbes = getNumber(token);
    /* Check message size characters */
    token = strtok(NULL, STRING_SPLITTER);
    service.messageSize = getNumber(token);
    /* Check server delay characters */
    token = strtok(NULL, STRING_SPLITTER);
    service.serverDelay = getNumber(token);
} 

/* Check if a string is a number */
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

