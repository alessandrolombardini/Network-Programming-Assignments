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

#define STRING_HELLO_RESPOSE_OK        "200 OK - Ready\n"                           /* Positive response awaited from server after send the hello message */
#define STRING_BYE_RESPONSE_OK         "200 OK - Closing\n"                         /* Positive response awaited from server after send the bye message */
#define STRING_HELLO_RESPONE_NOT_OK    "404 ERROR - Invalid Hello message\n"        /* Negative response awaited from server after send the hello message */
#define STRING_PROBE_RESPONSE_NOT_OK   "404 ERROR - Invalid Measurement message\n"  /* Negative response awaited from server after send a probe message */
#define STRING_BYE_RESPONSE_NOT_OK     "404 ERROR - Invalid Bye message\n"          /* Negative response awaited from server after send the bye message */ 
#define max(a,b) (((a) > (b)) ? (a) : (b))                        
#define BUF_SIZE 1024                         /* Maximum buffer size */
#define INPUT_FILE_NAME "init.conf.txt"           /* Name of the file where the hello message is conteined */
#define PAYLOAD_CHARACTER 'x'                 /* Character of payload of messages that has to be sent */
#define WAIT_HELLO_MESSAGE_RESPONSE  1        /* Phase: we have to send hello message and receive its response */
#define WAIT_PROBE_MESSAGE_RESPONSE  2        /* Phase: we have to send probe message and receive its response */
#define WAIT_BYE_MESSAGE_RESPONSE    3        /* Phase: we have to send bye message and receive its response */
#define STRING_SPLITTER " "                   /* String splitter of the hello message */
#define MAX_BYTE_OF_HEADER           100      /* Max number of bytes of probe message header */
#define BOOL int 
#define TRUE 1
#define FALSE 0

BOOL checkInputString(char mess[]);     /* Check if the string read by file contains valid values */
void parseInputString(char mess[]);     /* Extract params by the string read by file and insert them inside the data structure*/
void sendHelloMessage();                /* Send hello message to the server and receive its response*/
void sendProbeMessages();               /* Send a sequence of probe message to the server and receive their responses */
void sendByeMessage();                  /* Send a bye message to the server and receive its response */
float evaluateRTT(float rttOfProbes[]); /* By a sequence of RTT, it extract an avarage value */ 
BOOL isNumber(char * token);            /* Check if a string is a number */
int getNumber(char * token);            /* Extract a number by a string */
char * receiveMessage();                /* Allows to received the completed next message */ 
BOOL readFile();                        /* Read init.conf, check if values inside are valid and load them into the data structure */

/* Service structure definition */
typedef struct node {
	char measureType[6];    /* Service requested - RTT or THPUT */
	int nProbes;            /* Number of probe messages to send */
	int messageSize;        /* Size of probe message's payload */
	int serverDelay;        /* Delay of server probe message echo (in milliseconds) */
  int serverFD;           /* File descriptor of socket */
  int phaseNumber;        /* 1: Hello phase
                             2: Probe phase
                             3: Bye phase */
} serviceNode;
serviceNode service;

int main(int argc, char *argv[]){
  struct sockaddr_in server_addr; /* Struct containing server address information */
  struct sockaddr_in client_addr; /* Struct containing client address information */
  int socketFD;                   /* Socket file descriptor */
  int connectResult;              /* Connect result */
  ssize_t byteRecv;               /* Number of bytes received */
  ssize_t byteSent;               /* Number of bytes sent */

  /* Check if number of params passed is correct */
  if (argc != 3) {
		printf("\n(CLIENT) Error: wrong number of parameters\n");
		printf("\n%s <server IP (dotted notation)> <server port>\n", argv[0]);
		exit(EXIT_FAILURE);
  }
  /* Read init.conf file */
  if(!readFile()){
    printf("(CLIENT) Error: hello message format isn't correct \n");
    exit(EXIT_FAILURE);
  }
  service.phaseNumber = 1;
  /* Open connection with the tcp server */
  socketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socketFD < 0){
  	printf("(CLIENT) Error: socket error"); 
  	exit(EXIT_FAILURE);
  }
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(atoi(argv[2]));
  server_addr.sin_addr.s_addr = inet_addr(argv[1]);
  connectResult = connect(socketFD, (struct sockaddr *) &server_addr, sizeof(server_addr));
  if (connectResult < 0){
    perror("(CLIENT) Error: connection error"); 
    exit(EXIT_FAILURE);
  }
  service.serverFD = socketFD;
  /** Send all messages and receive their response **/
  /* Hello phase */
  sendHelloMessage();
  service.phaseNumber = 2;
  /* Probe phase */
  sendProbeMessages();
  service.phaseNumber = 3;
  /* Bye phase */
  sendByeMessage();
  return 0;
}

BOOL readFile(){
  FILE * fileFD;
  char readData[BUF_SIZE];
  char * res;

  /* Read hello message by file */
  fileFD = fopen(INPUT_FILE_NAME, "r");
  if(fileFD == NULL){
    printf("(CLIENT) Error: opening file error\n");
    close(service.serverFD);
    exit(EXIT_FAILURE);
  }
  res = fgets(readData, BUF_SIZE, fileFD);
  if (res == NULL){
    printf("(CLIENT) Error: reading file\n");
    close(service.serverFD);
    exit(EXIT_FAILURE);
  }
  fclose(fileFD);
  /* Check hello message and, if it is valide, parse it in the data structure */
  if(checkInputString(readData)){
    parseInputString(readData);
    return TRUE;
  } 
  return FALSE;
}

BOOL checkInputString(char mess[]){
  char * token;
  char message[strlen(mess)];

  strcpy(message, mess);
  /* Check measure type characters */
  token = strtok(message, STRING_SPLITTER);
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
  /* Extract measure type characters */
  token = strtok(message, STRING_SPLITTER);
  strcpy(service.measureType, token);
  /* Extract number of probes characters */
  token = strtok(NULL, STRING_SPLITTER);
  service.nProbes = getNumber(token);
  /* Extract message size characters */
  token = strtok(NULL, STRING_SPLITTER);
  service.messageSize = getNumber(token);
  /* Extract server delay characters */
  token = strtok(NULL, STRING_SPLITTER);
  service.serverDelay = getNumber(token);
} 

void sendHelloMessage(){
  char sendData[BUF_SIZE];        
  char * completeReceivedMessage;

  /* Send hello message to the server and check its response */
  sprintf(sendData, "h %s %d %d %d\n", service.measureType, service.nProbes, service.messageSize, service.serverDelay);
  send(service.serverFD, sendData, strlen(sendData), 0);
  printf("(CLIENT) Message sent: %s", sendData);
  completeReceivedMessage = receiveMessage();
  printf("(CLIENT) Message received: %s\n\n", completeReceivedMessage);
  if(strncmp(completeReceivedMessage, STRING_HELLO_RESPOSE_OK, strlen(completeReceivedMessage)) != 0){
      printf("\n(CLIENT) Error: server reject hello message\n");
      free(completeReceivedMessage);
      close(service.serverFD);
      exit(EXIT_FAILURE);
  }
  free(completeReceivedMessage);
}

void sendProbeMessages(){
  char payload[service.messageSize + 1];    /* Contains the payload */
  float rttOfProbes[service.nProbes];       /* RTT probes values */
  int numberOfBits = 0;                     /* Size of the probe message in bit */
  struct timespec start, end;               /* Variabiles usefull to take the RTT */
  int i;
  
  /* Create appropriate payload */
  for(i = 0; i < service.messageSize; i++){
    payload[i] = PAYLOAD_CHARACTER;
  }
  payload[i]='\0'; 
  /* Send and receive all probe messages */
  for(i = 0; i < service.nProbes; i++){
    /* Create probe message */
    char * finalMessageToSend = (char *)calloc(service.messageSize + MAX_BYTE_OF_HEADER, sizeof(char)); 
    if(finalMessageToSend==NULL){
      printf("(CLIENT) Error: an error occurred while calling calloc.\n");
      close(service.serverFD);
      exit(EXIT_FAILURE);
    }
    sprintf(finalMessageToSend, "%c ", 'm');
    char sequenceNumber[BUF_SIZE];
    sprintf(sequenceNumber, "%d ", i+1);
    strcat(finalMessageToSend, sequenceNumber);
    strcat(finalMessageToSend, payload);
    strcat(finalMessageToSend, "\n");
    numberOfBits = strlen(finalMessageToSend) * 8;     /* Bytes of message * 8 = bits of message */
    printf("(CLIENT) Message sent: %s", finalMessageToSend);
    /* Send probe message and check its response */
    send(service.serverFD, finalMessageToSend, strlen(finalMessageToSend), 0);
    clock_gettime(CLOCK_REALTIME, &start);
    /* Receive probe message response */
    char * completeMessageReceived = receiveMessage();
    clock_gettime(CLOCK_REALTIME, &end);
    printf("(CLIENT) Message received: %s", completeMessageReceived);
    if(strcmp(finalMessageToSend, completeMessageReceived) != 0){
      printf("\n(CLIENT) Error: server reject probe message\n");
      free(finalMessageToSend);
      free(completeMessageReceived);
      close(service.serverFD);
      exit(EXIT_FAILURE);
    }
    free(finalMessageToSend);
    free(completeMessageReceived);
    /* Calculate RTT of probe message */
    rttOfProbes[i] = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;        /* RTT in microseconds */
    printf("(CLIENT) RTT of probe message number %i - %f milliseconds\n\n", i+1, rttOfProbes[i]/1000);  
  }
  /* Show response to the request of service, that could be RTT or THROUGHPUT */
  if(strcmp(service.measureType, "rtt") == 0){
    printf("Average RTT: %f milliseconds\n\n", evaluateRTT(rttOfProbes)/1000);
  } else{
    /* printf("Number of bytes sent for each message: %d\n", numberOfBits/8); 
    printf("Average RTT: %f milliseconds\n", (evaluateRTT(rttOfProbes)/1000)); */
    printf("Avarage THPUT: %f kilobits/seconds\n\n", 
          (((float)numberOfBits)/1000) / (evaluateRTT(rttOfProbes)/1000000));
  }

  // printf("List of RTT:\n");
  // for(int i = 0; i < service.nProbes; i++){
  //   printf("RTT of message %i: %f\n", i, rttOfProbes[i]/1000);
  // }
  // puts("\n");
}

void sendByeMessage(){
  char byemessage[3]="b\n\0";
  char * completeReceivedMessage;

  /* Send bye message to the server and check its response */
  send(service.serverFD, byemessage, strlen(byemessage), 0);
  printf("(CLIENT) Message sent: %s", byemessage);
  completeReceivedMessage = receiveMessage();
  printf("(CLIENT) Message received: %s\n", completeReceivedMessage);
  if(strcmp(completeReceivedMessage, STRING_BYE_RESPONSE_OK) != 0){
    printf("\n(CLIENT) Error: server reject bye message\n");
    free(completeReceivedMessage);
    close(service.serverFD);
    exit(EXIT_FAILURE);
  } 
  printf("\n(CLIENT) Service completed.\n");
  close(service.serverFD);
  free(completeReceivedMessage);
}

char * receiveMessage(){
  BOOL messageIsComplete = FALSE;   /* Check if message is complete */
  char * completeMessageReceived;   /* Complete message that has been received: it is composed by multiple reading */
  int respOkLen = 0;                /* Length of OK response message exptected in the actual phase */
  int respNotOkLen = 0;             /* Length of NOT_OK response message exptected in the actual phase */
  int byteRecv = 0;                 /* Number of bytes received in one buffer read */
  int totalByteReceived = 0;        /* Total bytes read of one message */
  int bufferMultiplier = 1;         /* Multiplier of message buffer: it has to be increased if the space is not enought */

  completeMessageReceived = (char *)calloc(BUF_SIZE*bufferMultiplier, sizeof(char));
  if(completeMessageReceived==NULL){
    printf("(CLIENT) Error: an error occurred while calling calloc.\n");
    close(service.serverFD);
    exit(EXIT_FAILURE);
  }
  for(;messageIsComplete==FALSE;){
    char * receivedData = (char *)calloc(BUF_SIZE, sizeof(char));   /* Part of all message that has been read */
    if(receivedData==NULL){
      printf("(CLIENT) Error: an error occurred while calling calloc.\n");
      free(completeMessageReceived);
      close(service.serverFD);
      exit(EXIT_FAILURE);
    }
    byteRecv =  recv(service.serverFD, receivedData, BUF_SIZE, 0);
    if (byteRecv < 0){
        printf("(CLIENT) Error: recive error");
        free(receivedData);
        free(completeMessageReceived);
        close(service.serverFD);
        exit(EXIT_FAILURE);
    }
    /* If the initial buffer is not enough big to containt all message, it is reallocated */
    totalByteReceived += byteRecv;
    if(totalByteReceived > BUF_SIZE * bufferMultiplier -1){
        bufferMultiplier += 1;
        completeMessageReceived = (char *) realloc(completeMessageReceived, BUF_SIZE * bufferMultiplier * sizeof(char));
    }
    strcat(completeMessageReceived, receivedData);
    free(receivedData);
    /** In this phase I need to check if the message just arrived is complete:
        this becouse it's possible that it doesnt' arrive in just one message,
        but in multiple message **/
    if(completeMessageReceived[strlen(completeMessageReceived) - 1] == '\n'){
      messageIsComplete=TRUE;
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

