#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "myfunction.h"

#define STRING_HELLO_RESPOSE_OK        "200 OK - Ready"
#define STRING_HELLO_RESPONE_NOT_OK    "404 ERROR - Invalid Hello Message"
#define STRING_BYE_RESPONSE_OK         "200 OK - Closing"
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define MAX_BUFFER_SIZE 32768           /* Maximum size of TCP messages */
#define INPUT_FILE_NAME "init.conf"     /* Name of the file where the hello message is conteined */
#define PAYLOAD_CHARACTER 'a'           /* Character of payload of messages that has to be sent */
#define HELLO_PHASE 1                   /* We have to send hello message and receive its response */
#define PROBE_PHASE 2                   /* We have to send probe message and receive its response */
#define BYE_PHASE 3                     /* We have to send bye message and receive its response */
#define STRING_SPLITTER " "           
#define BOOL int 
#define TRUE 1
#define FALSE 0

BOOL checkInputString(char mess[]);     /* Check if the string passed it'is a valid hello message */
void parseInputString(char mess[]);     /* Extract params by hello message string and insert them inside the data structure*/
void sendHelloMessage();                /* Send hello message to the server and receive its response*/
void sendProbeMessages();               /* Send a sequence of probe message to the server and receive their responses */
void sendByeMessage();                  /* Send a bye message to the server and receive its response */
float evaluateRTT(float rttOfProbes[]); /* Bye a sequence of RTT, it extract an avarage value */ 
BOOL isNumber(char * token);            /* Check if a string is a number */
int getNumber(char * token);            /* Extract a number by a string (1 if it's not a number) */
char * receiveMessage();           /* Allows to received the completed next message */ 

/* Service structure definition goes here */
typedef struct node {
	char protocolPhase[2];
	char measureType[4];
	int nProbes;
	int messageSize;
	int serverDelay;
  int serverFD;
  int phaseNumber;  /* 1: Hello message
                       2: Probe message
                       3: Bye message */
} serviceNode;
serviceNode service;

int main(int argc, char *argv[]){
  struct sockaddr_in server_addr; /* Struct containing server address information */
  struct sockaddr_in client_addr; /* Struct containing client address information */
  int serverFD;                   /* Server socket file descriptor */
  int connectResult;              /* Connect result */
  ssize_t byteRecv;               /* Number of bytes received */
  ssize_t byteSent;               /* Number of bytes sent */
  socklen_t serv_size;

  /* Check if number of params passed is correct */
  if (argc != 3) {
		printf("\nErrore numero errato di parametri\n");
		printf("\n%s <server IP (dotted notation)> <server port>\n", argv[0]);
		exit(EXIT_FAILURE);
  }
  /* Open connection with the tcp server */
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
  /* Send of all messages and receive of their respective answers */
  sendHelloMessage();
  service.phaseNumber = 2;
  sendProbeMessages();
  service.phaseNumber = 3;
  sendByeMessage();
  return 0;
}

BOOL checkInputString(char mess[]){
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

void parseInputString(char mess[]){
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
  service.phaseNumber = 1;
} 

void sendHelloMessage(){
  char sendData[MAX_BUFFER_SIZE];        
  FILE * fileFD;

  /* Read hello message by file */
  fileFD = fopen(INPUT_FILE_NAME, "r");
  if(fileFD == NULL){
    perror("(CLIENT) Opening file error\n");
    exit(EXIT_FAILURE);
  }
  fgets(sendData, MAX_BUFFER_SIZE, fileFD);
  sendData[strcspn(sendData, "\n")] = ' ';
  fclose(fileFD);
  /* Check hello message and, if it is valide, parse it in the data structure */
  if(checkInputString(sendData)){
    parseInputString(sendData);
  } else{
    perror("(CLIENT) Error: Hello message format isn't correct \n");
    exit(EXIT_FAILURE);
  }
  strcat(sendData, "\n");
  /* Send hello message to the server and check its response */
  send(service.serverFD, sendData, strlen(sendData), 0);
  printf("(CLIENT) Message sent: %s", sendData);
  char * completeReceivedMessage = receiveMessage();
  printf("(CLIENT) Message received: %s\n", completeReceivedMessage);
  if(strncmp(completeReceivedMessage, STRING_HELLO_RESPOSE_OK, strlen(completeReceivedMessage)) != 0){
      free(completeReceivedMessage);
      printf("(CLIENT) Error: server reject hello message\n");
      exit(EXIT_FAILURE);
  }
  free(completeReceivedMessage);
}

void sendProbeMessages(){
  char payload[service.messageSize + 1];            /* Contains the payload */
  float rttOfProbes[service.nProbes];               /* RTT probes values */
  int numberOfBits = 0;                           /* Size of the probe message in bit */
  struct timespec start, end;                       /* Variabiles usefull to take the RTT */
  BOOL messageIsComplete = FALSE; 
  int i;
  
  /* Create appropriate payload */
  for(i = 0; i < service.messageSize; i++){
    payload[i] = PAYLOAD_CHARACTER;
  }
  payload[i]='\0'; 
  /* Send and receive all probe messages */
  for(i = 0; i < service.nProbes; i++){
    /* Create probe message */
    char finalMessageToSend[MAX_BUFFER_SIZE] = "m ";
    char sequenceNumber[MAX_BUFFER_SIZE];
    sprintf(sequenceNumber, "%d ", i+1);
    strcat(finalMessageToSend, sequenceNumber);
    strcat(finalMessageToSend, payload);
    strcat(finalMessageToSend, "\n");
    numberOfBits=strlen(finalMessageToSend)*8;
    /* Send probe message and check its response */
    send(service.serverFD, finalMessageToSend, strlen(finalMessageToSend), 0);
    clock_gettime(CLOCK_REALTIME, &start);
    printf("(CLIENT) Message sent: %s", finalMessageToSend);
    /* Receive probe message response */
    char * completeMessageReceived = receiveMessage();
    clock_gettime(CLOCK_REALTIME, &end);
    if(strcmp(finalMessageToSend, completeMessageReceived) != 0){
      printf("(CLIENT) Error: server reject probe message\n");
      exit(EXIT_FAILURE);
    }
    printf("(CLIENT) Message received: %s", completeMessageReceived); 
    free(completeMessageReceived);
    /* Calculate RTT of probe message */
    rttOfProbes[i] = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000; // RTT in microseconds
    printf("(CLIENT) RTT of probe message number %i - %f milliseconds\n\n", i+1, rttOfProbes[i]/1000);
  }
  /* Show response to the request of service, that could be RTT or THROUGHPUT */
  if(strcmp(service.measureType, "rtt") == 0){
    printf("Average RTT: %f milliseconds\n\n", evaluateRTT(rttOfProbes)/1000);
  } else{
    printf("Byte nr: %d\n", numberOfBits/8);
    printf("Average RTT: %f milliseconds\n", (evaluateRTT(rttOfProbes)/1000));
    printf("Throughtput: %f kilobits/seconds\n\n", (((float)numberOfBits)/1000) /(evaluateRTT(rttOfProbes)/1000000));
  }
}

void sendByeMessage(){
  char byemessage[3]="b\n\0";

  send(service.serverFD, byemessage, strlen(byemessage), 0);
  printf("(CLIENT) Message sent: %s", byemessage);
  char * completeReceivedMessage = receiveMessage();
  printf("(CLIENT) Message received: %s\n", completeReceivedMessage);
  if(strcmp(completeReceivedMessage, STRING_BYE_RESPONSE_OK) != 0){
    free(completeReceivedMessage);
    printf("(CLIENT) Error: server reject bye message\n");
    exit(EXIT_FAILURE);
  } 
  free(completeReceivedMessage);
}

char * receiveMessage(){
  char * completeMessageReceived = (char *)calloc(MAX_BUFFER_SIZE, sizeof(char));    /* Complete message */
  BOOL messageIsComplete = FALSE;                                                    /* Check if message is complete */
  for(;messageIsComplete==FALSE;){
    char * receivedData = (char *)calloc(MAX_BUFFER_SIZE, sizeof(char));
    recv(service.serverFD, receivedData, MAX_BUFFER_SIZE, 0);
    /** In this phase I need to check if the message just arrived is complete:
        this becouse it's possible that it doesnt' arrive in just one message,
        but in multiple message **/
    strcat(completeMessageReceived, receivedData);
    free(receivedData);
    /* Check if the message received at the moment is complete (has \n) */
    if(service.phaseNumber == HELLO_PHASE) {
      int messLengh = strlen(completeMessageReceived);
      int respOkLen = strlen(STRING_HELLO_RESPOSE_OK);
      int respNotOkLen = strlen(STRING_HELLO_RESPONE_NOT_OK);
      if(strncmp(STRING_HELLO_RESPOSE_OK, completeMessageReceived, min(respOkLen, messLengh)) == 0 || 
         strncmp(STRING_HELLO_RESPONE_NOT_OK, completeMessageReceived, min(respNotOkLen, messLengh)) == 0){
        messageIsComplete=TRUE;
      }
    } else if(service.phaseNumber == PROBE_PHASE && completeMessageReceived[strlen(completeMessageReceived) - 1] == '\n'){
      messageIsComplete=TRUE;
    } else if(service.phaseNumber == BYE_PHASE) {
      int messLengh = strlen(completeMessageReceived);
      int respOkLen = strlen(STRING_BYE_RESPONSE_OK);
      if(strncmp(STRING_BYE_RESPONSE_OK, completeMessageReceived, min(respOkLen, messLengh)) == 0){
        messageIsComplete=TRUE;
      }
    }
  }
  return completeMessageReceived;
}

float evaluateRTT(float rttOfProbes[]){
  float total = 0;
  for(int i = 0; i < service.nProbes; i++){
    total += rttOfProbes[i];
  }
  return total/service.nProbes;
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

