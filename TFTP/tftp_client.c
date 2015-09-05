/* Program:	CLIENT FOR TFTP CONNECTION
		Connects to a TFTP server to GET files
		*accepts filename as argument
		*syntax <host> GET <filename>
		*TFTP port 69 used internally to connect
   Module:	IE 2015 ENEL4IE
   Name:	Matthew de Neef
   Stu. Num.	212503024			*/
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/* to explain sockets, only these are explicitly needed */
#include <sys/types.h>
#include <sys/socket.h>

#define TFTP_PORT "2804"					//well known port for TFTP

	/*sockets method*/
void startSockets(struct addrinfo, struct addrinfo*);

int main(int argc, char *argv[])			//argv[] are args passed from user in terminal
{
	char *inarg;	
	char *arg[3];					//accepts input args from user
	char *arg0;
		
	char *saveptr;	
	int cont=0;					//var to indicate continuing execution

	struct addrinfo hints, *res, *p;		//declares a struct of type 
	int status,rv;					//integer for return flags of functions
	int s,c;					//socket descriptor/ connect result
	char *inettype;					//friendly names for getaddrinfo params
	char *socktype;
	char *prottype;	
	
	char *dest;					//destination hostname
	char *destport;					//destination port
	char ipstr[INET6_ADDRSTRLEN];			//array of length of an ipv6 addr

	/* for receiving socket setup we need storage for structs */
	struct sockaddr_storage incoming_addr;		//struct for incoming
	socklen_t incoming_addr_size;				//incoming size

	/* for sending data these fields are needed */
	char *msg_tosend;				//string of message to send
	int sndmsg_len;					//length of string above
	int bytes_sent;					//counter for bytes sent

	/* for receiving data from a server*/
	char *msg_received;				//string for received message
	int recvmsg_len;				//received length
	int bytes_recv;					//buffer to receive
	int new_sd;					//receiving socket descriptor
	

	//Welcome user with a prompt'
	printf("TFTP Client | Matthew de Neef | 212503024\n");
	printf("usage: <hostname to connect to> GET <filename>.\n e.g. 146.230.193.160 GET picFile \n"); 	
  do{	
	cont = 0;
	printf("tftp>");
	fgets(inarg,50,stdin);				//get line from keyboard
	/* tokenise input into:
	arg0 = hostname
	arg1 = "GET" for now
	arg2 = filename
	*/ 
	int i=0;
	arg[0] = strtok_r(inarg, " ",&saveptr);
	arg[1] = strtok_r(NULL, " ",&saveptr);
	arg[2] = strtok_r(NULL, " ",&saveptr);	
	
	if(arg[0]!= NULL && arg[1]!=NULL && arg[2]!=NULL){ 	
	printf("%s\n",arg[0]);
	printf("%s\n",arg[1]);
	printf("%s\n",arg[2]);
	}

	if (arg[2] == NULL){			//must have 3 args before checing validity
	printf("usage: <hostname to connect to> GET <filename>. Try again.\n");
	cont = 1;}
	
	if(arg[2]!=NULL){			//this only runs if we have all 3 args
		if ((strstr(arg[1],"GET")==NULL)&&(strstr(arg[1],"get")==NULL)){
		printf("GET command missing. Try again.\n");
		cont = 1;}
	}

	if(arg[2]!=NULL && arg[1]!=NULL){
		if (arg[0] == "\n\0"){			//only runs if 3 args != NULL
		printf("\nSome errors encountered: \nNo hostname entered. Try again.\n");
		cont = 1;}
	}


 }while(cont == 1);		//break out of this loop if the syntax is correct!

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
	hints.ai_socktype = 0;			//type of socket--we want a UDP socket for TFTP
	//hints.ai_flags = AI_PASSIVE;			//keep this DISABLED to use connect()
	hints.ai_protocol = 0;				//any protocol accepted

	//GETTING ADDRESS INFO FROM HOST SERVER	
	
	/*get address info for host 	
	ARGUMENTS:	1st: IP address of host to get info
			2nd: Port, fixed at 69 for TFTP
			3rd: hints struct with socket/protocol we require
			4th: the actual socket/protocol data the server has
	PURPOSE:	passes the hints struct with connection requirements
			to server. We want datagram and UDP service
			if the reqs match what the server has, we're OK*/
		
	argv[1] = *arg[0];
	if ((rv = getaddrinfo(argv[1], TFTP_PORT, &hints, &res)) != 0) //errorcheck getaddrinfo
	{
		fprintf(stderr, "Connection params failed: %s\n", gai_strerror(status));
		return 2;
	}

	//CALL SOCKETS
	startSockets(hints,res);

}

void startSockets(struct addrinfo hints, struct addrinfo *res)
{
	printf("we're here!");

	/*dest = argv[1];					//assign input args to dest h/n
	destport = argv[2];				//...or port respectively

	printf("\n%s and %s\n",argv[1],argv[2]);
		
	printf("Opening socket for %s on dest. port: %s\n\n", argv[1], argv[2]);		//else show socket

	if(res->ai_family == AF_INET6){inettype = "IPv6";}else{inettype = "IPv4";}
	if(res->ai_socktype == SOCK_DGRAM){socktype = "Datagram";}else{socktype = "Stream";}
	if(res->ai_protocol == 6){prottype = "TCP";}else{prottype = "UDP";}
	
	printf("%d\n",res->ai_family);
	printf("%d\n",res->ai_socktype);
	printf("%d\n",res->ai_protocol);
	printf("family: %s \n socket type: %s \n protocol: %s \n",inettype,socktype,prottype);
	
	/* action section
	s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);	//tries to pass to socket()
	c = connect(s, res->ai_addr, res->ai_addrlen);*/

}
