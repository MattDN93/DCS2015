/* Program:	SIMPLE CLIENT FOR CONNECTION
		C stub to test creation of sockets
		Connects to a port at a remote address
   Module:	DCS 2015 ENEL4CC
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


int main(int argc, char *argv[])			//argv[] are args passed from user in terminal
{
	struct addrinfo hints, *res, *p;		//declares a struct of type addrinfo 
	int status;					//integer for return flags of functions
	int s,c;					//socket descriptor/ connect result
	char *inettype;					//friendly names for getaddrinfo params
	char *socktype;
	char *prottype;	
	
	char *dest;					//destination hostname
	char *destport;					//destination port
	char ipstr[INET6_ADDRSTRLEN];			//array of length of an ipv6 addr
	
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
	hints.ai_socktype = 0;				//type of socket CL or CO
	//hints.ai_flags = AI_PASSIVE;			//keep this DISABLED to use connect()
	hints.ai_protocol = 0;				//any protocol accepted
	
	if (argv[1] == NULL || argv[2] == NULL){printf("usage: <hostname to connect to> <port>. \nTry again.\n"); return 1;}	//check for null input on both args


	if ((getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) //errorcheck getaddrinfo
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	//Debug only: getaddrinfo("www.example.com", "http", &hints, &res);
	
	dest = argv[1];					//assign input args to dest h/n
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
	
	/* action section*/
	s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);	//tries to pass to socket()
	c = connect(s, res->ai_addr, res->ai_addrlen);
	/* end action section*/

	//binds printf("%d\n",s);
	// Debug only: s = socket(AF_INET6, SOCK_STREAM, 3);	//tries to pass to socket()

	if (s==-1){
		printf("Socket Creation Failed. Reason:\n");		//if socket failed
		fprintf(stderr,"\n '%s.' \n",gai_strerror(s));	//prints error message
	}
	else{
		printf("Created socket successfully using %s, in %s mode using the %s protocol.\n\n",inettype,socktype,prottype);
	}

	if(c == -1){
		printf("\nconnection failed. Reason: %s\n",gai_strerror(c));
	}else{ 
		printf("Socket connected to %s on port %s successfully\n", dest, destport);
	}

}
