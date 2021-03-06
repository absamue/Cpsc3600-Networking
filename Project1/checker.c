/* Andrew Samuels
 * CPSC 3600
 * Project 1 - Case inversion server
 *
 * This file contains the implemetation for running the checker client. The
 * client can be run by compiling and then running the executable as
 * follows:
 * ./checker -p <portno> -s <server IP/name> -m <message>
 *
 * <portno> is the port that the client will attempt to connect to at the
 * server. The portno must be the same as the one that was used with the -p
 * flag when running caseInverter.
 *
 * <server IP/name> is the IP address or name of the server. This should be
 * the IP address of the physical machine that is running the server. The
 * name of the server machine can be used if the client machine is networked to
 * the server machine.
 *
 * <message> is the message that will be sent to the server to be inverted.
 *
 * Upon execution, the client will make the connection to the server, send
 * the message, and then send the recieved message once more to confirm
 * that the server is correct. If the original message is the same as the
 * second returned message then we have confirmed that the server is
 * operating correctly. 
 * 
 * The program will then finally display the following information:
 *
 * Connection attempts, time elapsed, original message, returned message,
 * verifvication status.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>

int main(int argc, char *argv[]){
	clock_t tic = clock(); //start timer
	
	int sock, n, portno;
	int att = 0; //attempts
	unsigned int length;
	struct sockaddr_in serv, from; //address holders
	struct hostent *hp; //hsot information

	char orig[256]; //original message from -m flag
	char ret[256]; //first returned message from server
	char check[256]; //second returned message


	//grab the command line flags
	int f;
	while((f = getopt(argc, argv, "p:s:m:")) != -1){
		switch(f){
			//port number
			case 'p': portno = atoi(optarg);
						 break;
			//server ip
			case 's': hp = gethostbyname(optarg);
						 if(hp == 0){
							 fprintf(stderr,"Error, host not found.\n");
							 exit(0);
						 }
						 break;
			//message
			case 'm': strcpy(orig, optarg);
						 break;
			//error message
			default: fprintf(stderr, "Usage: yeller -s <serverIP> -p <serverPort> -m <message>\n");
						break;
		}
	}

	//create a datagram socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0){
		printf("Error opening socket\n");
		exit(0);
	}

	//set up socket parameters
	serv.sin_family = AF_INET;
	bcopy((char *)hp->h_addr, (char *)&serv.sin_addr, hp->h_length);
	serv.sin_port = htons(portno);
	length = sizeof(struct sockaddr_in);

	//send message to socket
	n = sendto(sock, orig, strlen(orig),0,(const struct sockaddr *)&serv, length);
	if(n < 0){
		printf("Error on sendto\n");
		exit(0);
	}
	//successful attempt
	else
		att++;

	//get returned message
	bzero(ret,256);
	n = recvfrom(sock,ret,strlen(orig),0,(struct sockaddr *)&from, &length);
	if(n < 0){
		printf("Error on recieve\n");
		exit(0);
	}
	
	//send message to socket, again
	n = sendto(sock, ret, strlen(ret),0,(const struct sockaddr *)&serv, length);
	if(n < 0){
		printf("Error on sendto\n");
		exit(0);
	}
	else
		att++;

	//get returned message, again
	bzero(check, 256);
	n = recvfrom(sock,check,strlen(ret),0,(struct sockaddr *)&from, &length);
	if(n < 0){
		printf("Error on recieve\n");
		exit(0);
	}
	
	//close socket
	close(sock);

	//stop timer
	clock_t toc = clock();
	double time = (double)(toc - tic) / CLOCKS_PER_SEC;
	printf("%d	%f	%s	%s", att, time, orig, ret);
	
	//confrim inversion worked
	if(strcmp(orig, check) == 0){
		printf("	Verified!\n");
	}
	else{
		printf("\nVERIFICATION FAILED\n");
		printf("Original: %s	Double Inverted: %s\n", orig, check);
	}

	return 0;
}
