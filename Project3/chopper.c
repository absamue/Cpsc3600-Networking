/* Andrew Samuels - absamue
	Nicole Michaud - nmichau
	Parth Patel - pspatel
	Stuart Jackson - sljacks

	CPSC Project 3 - HTTP Server and clients
	Client 1 - Chopper

	The first client is intended to send HTTP/1.1 add and view requests. The
	add functionality will take the parameter of the -a flag and send it as
	an add request to the server. The view functionality will request for
	the server to return the contents of the servers buffer as a payload in
	the response.

	Before terminating, the program will display the number of sent
	requests, the time taken in seconds, the input string, the returned HTTP
	status code, and the response payload (if a view request was sent)
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
#include <stdbool.h>

int main(int argc, char *argv[]){
	clock_t tic = clock(); //start timer

	int sock, n, portno;
	int att = 0;
	unsigned int length;
	struct sockaddr_in serv, from; //address holders
	struct hostent *hp; //host information

	char message[256]; //orignial message from flags
	char ret[1256]; //returned message from server
	char *action = NULL; //add or view
	char *data; //data paramater from add or view
	char *status; //response from server
	char *header = NULL; //header if specified
	char body[1000]; //holder for view request data
	bool view = false;

	//grab the command line flags
	int f;
	while((f = getopt(argc, argv, "p:s:a:h:v")) != -1){
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
			case 'a': if(strlen(optarg) > 80){
							 printf("Max message length is 80 characters");
							 exit(0);
						 }
						 else{
							 //set up the add action
							 action = malloc(sizeof("/add?") + sizeof(optarg));
							 sprintf(action, "/add?%s", optarg);
							 
							 //save the data parameter
							 data = malloc(sizeof(optarg));
							 data = optarg;
						 }
						 break;
			//header
			case 'h': header = optarg;
						 break;
			//view request
			case 'v': view = true;
						 break;
			//error message
			default: fprintf(stderr, "Usage: bellower -s <serverIP> -p <serverPort> -m <message>\n");
						break;
		}
	}

	//view request found, overrite potential add request
	if(view == true){
		if(action != NULL)
			free(action);
		action = malloc(sizeof("/view?"));
		sprintf(action, "/view?");
	}

	//create a datagram socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		printf("Error opening socket\n");
		exit(0);
	}

	//check header
	if(header == NULL)
		header = hp->h_name;

	//set up socket parameters
	serv.sin_family = AF_INET;
	bcopy((char *)hp->h_addr, (char *)&serv.sin_addr, hp->h_length);
	serv.sin_port = htons(portno);
	length = sizeof(struct sockaddr_in);

	//attempt to make the connection
	if(connect(sock, (struct sockaddr *) &serv, length) < 0){
		printf("error on connect\n");
		exit(0);
	}

	sprintf(message, "GET %s HTTP/1.1\nHost: %s\n", action, header);

	//write our message to the socket
	n = write(sock, message, sizeof(message));
	if(n < 0){
		printf("error on write\n");
		exit(0);
	}
	//increment attempts
	else
		att++;

	//read returned message from the socket
	n = read(sock, ret, sizeof(ret));
	if(n < 0){
		printf("error on read\n");
		exit(0);
	}
	//get info we need out of response
	else{
		//get http status
		char *token = strtok(ret, "\n");	
		status = token;

		//luckily we know what the response will be, so skip through
		//unrelated info and get body
		memset(body, 0, sizeof(body));
		for(int i=0; i<7; i++){
			token = strtok(NULL, "\n");
		}
		while(token != NULL){
			strcat(body, token);
			strcat(body, "\n");
			token = strtok(NULL, "\n");
		}
	}
	//stop clock and print what we got
	clock_t toc = clock();
	double time = (double)(toc - tic) / CLOCKS_PER_SEC;
	printf("%d	%f	%s	%s	%s\n", att, time, data, status, body);

	return 0;
}
