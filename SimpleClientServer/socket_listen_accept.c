/* Program:	C stub to test creation of sockets
		Listens for connection on port 2804
		Use "socket_connect localhost 2804" in another terminal
		to test it!
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

/* for servers listening */
#define LOCALPORT "2804"				//users will connect to this port
#define BACKLOG 5					//number of waiting backed up connections



int main(int argc, char *argv[])			//argv[] are args passed from user in terminal
{
	struct addrinfo hints, *res, *p;		//declares a struct of type addrinfo 
	int status;					//integer for return flags of functions
	int s,b,c,a;					//socket descriptor/ connect result
	char *inettype;					//friendly names for getaddrinfo params
	char *socktype;
	char *prottype;	
	int portnum;
	char ipstr[INET6_ADDRSTRLEN];			//array of length of an ipv6 addr

	/* for receiving data we need storage for structs */
	struct sockaddr_storage incoming_addr;		//struct for incoming
	socklen_t incoming_addr_size;				//incoming size
	int new_sd;					//new socket descripter after accept()
	int bl=0;
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
	hints.ai_flags = AI_PASSIVE;			//keep this DISABLED to use connect()
	hints.ai_protocol = 0;				//any protocol accepted
	
	//WE don't need address info, and we specify a local port to listen on

	if ((getaddrinfo(NULL, LOCALPORT, &hints, &res)) != 0) //errorcheck getaddrinfo
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	//Debug only: getaddrinfo("www.example.com", "http", &hints, &res);
	
	//dest = argv[1];					//assign input args to dest h/n
	//destport = argv[2];				//...or port respectively

	
	printf("Opening socket to listen on port: %s\n\n", LOCALPORT);		//else show socket

	if(res->ai_family == AF_INET6){inettype = "IPv6";}else{inettype = "IPv4";}
	if(res->ai_socktype == SOCK_DGRAM){socktype = "Datagram";}else{socktype = "Stream";}
	if(res->ai_protocol == 6){prottype = "TCP";}else{prottype = "UDP";}
	
	//printf("%d\n",res->ai_family);
	//printf("%d\n",res->ai_socktype);
	//printf("%d\n",res->ai_protocol);
	printf("family: %s \n socket type: %s \n protocol: %s \n",inettype,socktype,prottype);
	
	/* action section (socket decriptor = s)*/
	s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);	//tries to pass to socket()
	b = bind(s,res->ai_addr,res->ai_addrlen);	
		
	//c = connect(s, res->ai_addr, res->ai_addrlen);
	/* end action section*/

	if (s==-1){
		printf("Socket Creation Failed. Reason:\n");		//if socket failed
		fprintf(stderr,"\n '%s.' \n",gai_strerror(s));	//prints error message
	}
	else{
		printf("Created socket successfully using %s, in %s mode using the %s protocol.\n\n",inettype,socktype,prottype);
	}

	if(b == -1){
		printf("\nPort binding failed. Reason: %s\n",gai_strerror(b));
	}else{ 
		printf("Socket bound to port %s successfully\n", LOCALPORT);
	}

	/*now listen for incoming...*/
	
	
	while (bl <5)
	{	
		printf("\nnow listening on port...\n");	
		listen(s,BACKLOG);					//listen on socket we created 
	
		//...when connection incoming...
		incoming_addr_size = sizeof(incoming_addr);
		new_sd = accept(s, (struct sockaddr *)&incoming_addr, &incoming_addr_size);
		bl++;
	
		printf("\nConnection detected! Forking socket descriptor now...");
		printf("\nNew connection ready on descriptor %d",new_sd);
		printf("\nThat's %d connections so far.",bl);
		printf("\n-------------------------------------");
	}

	printf("\nBacklog limit reached, closing.\n");
	/*if(c == -1){
		printf("\nconnection failed. Reason: %s\n",gai_strerror(c));
	}else{ 
		printf("Socket connected to %s on port %s successfully\n", dest, destport);
	}*/

}
