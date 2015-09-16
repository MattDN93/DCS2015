/* Program:	
		CLIENT APP
	ARGUMENTS:	pass <server hostname> <portnumber> when executing
				
	FEATURES:	*connects to any server at port 2804
			*Automatically sends required CHARGEN option to server
			*Receives welcome text from server first
			*Shows details of connection and data bytes send/received
			*Uses TCP socket connections, credit to Beej's guide for framework
   Module:	DCS 2015 ENEL4CC
   Name:	Matthew de Neef
   Stu. Num.	212503024			*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/* to explain sockets, only these are explicitly needed */
#include <sys/types.h>
#include <sys/socket.h>

#define MAX_BUFFER_SIZE 200


int main(int argc, char *argv[])			//argv[] are args passed from user in terminal
{
	//-------------------------INITIALIZATION-------------------------------------
	struct addrinfo hints, *res, *p;		//declares a struct of type addrinfo
	int status;					//integer for return flags of functions
	int s,c;					//socket descriptor/ connect result
	char *inettype;					//friendly names for getaddrinfo params
	char *socktype;
	char *prottype;

	char *dest;					//destination hostname
	char *destport;					//destination port
	char ipstr[INET6_ADDRSTRLEN];			//array of length of an ipv6 addr

	/* for receiving socket setup we need storage for structs */
	struct sockaddr_storage incoming_addr;		//struct for incoming
	socklen_t incoming_addr_size;			//incoming size
	int rv;						//return value for getaddrinfo

	/* for sending data these fields are needed */
	char *msg_tosend;				//string of message to send
	int sndmsg_len;					//length of string above
	int bytes_sent;					//counter for bytes sent

	/* for receiving data from a server*/
	char buffer[MAX_BUFFER_SIZE];			//buffer for input data
	char *msg_received;				//string for received message
	int recvmsg_len;				//received length
	int bytes_recv=1;					//buffer to receive
	int new_sd;					//receiving socket descriptor

	//-------------------------GET ADDRESS PARAMS-------------------------------------
	/* mandatory - fill in the hints structure
		       specifies which filters to use when fetching address info
			-NOTE: this is an addrinfo struct so it has:
			ai_flags
			ai_family
			ai_socktype
			ai_protocol
			ai_addrlen (struct)
			*ai_addr
			*ai_canonname

	To use BIND() set 1st arg for getaddrinfo = NULL, and set hints.ai_flags = AI_PASSIVE
	To use CONNECT set 1st arg for getaddrinfo != NULL, and disable hints.ai_flags = AI_PASSIVE*/

	memset(&hints, 0, sizeof hints);		//create memory for hints
	hints.ai_family = AF_UNSPEC; 			// AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;		//type of socket we want TCP
	//hints.ai_flags = AI_PASSIVE;			//keep this DISABLED to use connect()
	hints.ai_protocol = 0;				//any protocol accepted

	if (argv[1] == NULL || argv[2] == NULL){printf("usage: <hostname to connect to> <port>. \nTry again.\n"); return 1;}	//check for null input on both args


	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) //errorcheck getaddrinfo
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	//-------------------------SOCKET ACTIONS------------------------------------

	dest = argv[1];					//assign input args to dest h/n
	destport = argv[2];				//...or port respectively

	printf("Opening socket for %s on dest. port: %s\n\n", argv[1], argv[2]);		//else show socket

	if(res->ai_family == AF_INET6){inettype = "IPv6";}else{inettype = "IPv4";}
	if(res->ai_socktype == SOCK_DGRAM){socktype = "Datagram";}else{socktype = "Stream";}
	if(res->ai_protocol == 6){prottype = "TCP";}else{prottype = "UDP";}

	printf("family: %s \n socket type: %s \n protocol: %s \n",inettype,socktype,prottype);

	/* action section*/
	s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);	//tries to pass to socket()
	c = connect(s, res->ai_addr, res->ai_addrlen);
	/* end action section*/

	//-------------------------ERROR CHECKING-------------------------------------
	if (s==-1){
		printf("Socket Creation Failed. Reason:\n");		//if socket failed
		if (res->ai_flags == 0){printf("The remote socket hasn't been setup yet OR incorrect port specified.");}
		else{
		fprintf(stderr,"\n '%s.' \n",gai_strerror(s));	//prints error message
		}
	}
	//-------------------------DATA SEND/RECIEVE-------------------------------------
	else{
		printf("Created socket successfully using %s, in %s mode using the %s protocol.\n\n",inettype,socktype,prottype);
		printf("->Sending chargen command automatically!");

		//...when connection incoming...
		incoming_addr_size = sizeof(incoming_addr);		//make space in mem for struct

		/*while socket connection remains open, receive chargen from server*/
		while (bytes_recv >= 0){				//as long as server messages non-empty, receive them
            if ((bytes_recv = recv(s, buffer, MAX_BUFFER_SIZE-1, 0)) == -1) {
                perror("Recieve error:");
                exit(1);
            }
			buffer[bytes_recv] = '\0';
			printf("%s",buffer);				//get data and print out received msg
            if(strstr((char *)buffer,"chargen")){
                //issues chargen command to server
                msg_tosend = "chargen\r\n";					//msg to send to server
                sndmsg_len = strlen(msg_tosend);			//define the message length
                send(s,msg_tosend,sndmsg_len,0);			//s is the stored socket descriptor for this connection*/
            }
		}
		close(new_sd);
		close(s);
		exit(0);
	}

	if(c == -1){
		printf("\nconnection failed. Reason: %s\n",gai_strerror(c));
	}else{
		printf("Socket connected to %s on port %s successfully\n", dest, destport);
	}



}
