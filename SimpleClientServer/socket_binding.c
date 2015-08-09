/* Program:	C stub to test creation of sockets
		Binds socket to port
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
	int s,b;						//return of socket()/bind() call
	char *inettype;					//friendly names for getaddrinfo params
	char *socktype;
	char *prottype;	
	int portnum = 2849;	
size_t nbytes = 6;	
	char ipstr[INET6_ADDRSTRLEN];			//array of length of an ipv6 addr
	
	/* mandatory - fill in the hints structure */
	memset(&hints, 0, sizeof hints);		//create memory for hints
	hints.ai_family = AF_UNSPEC; 			// AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;		//type of socket CL or CO
	hints.ai_flags = AI_PASSIVE;			//fill in IP automagically xD

	// Debug only: argv[1] = "www.example.com";
	
	if (argv[1] == NULL){printf("usage: <hostname>. \nTry again.\n"); return 1;}	//check for null input

	//we can't pass a host arg here, just the port!
	if ((getaddrinfo(NULL, "2849", &hints, &res)) != 0) //errorcheck getaddrinfo
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	//Debug only: getaddrinfo("www.example.com", "http", &hints, &res);

		
	printf("Opening socket for %s:\n\n", argv[1]);		//else show socket

	if(res->ai_family == AF_INET6){inettype = "IPv6";}else{inettype = "IPv4";}
	if(res->ai_socktype == SOCK_DGRAM){socktype = "Datagram";}else{socktype = "Stream";}
	if(res->ai_protocol == 6){prottype = "TCP";}else{prottype = "UDP";}
	
	printf("%d\n",res->ai_family);
	printf("%d\n",res->ai_socktype);
	printf("%d\n",res->ai_protocol);
	printf("family: %s \n socket type: %s \n protocol: %s \n",inettype,socktype,prottype);
	
	s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);	//tries to pass to socket()
	b = bind(s,res->ai_addr,res->ai_addrlen);	
	//binds printf("%d\n",s);
	// Debug only: s = socket(AF_INET6, SOCK_STREAM, 3);	//tries to pass to socket()

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
		printf("Socket bound to port %d successfully\n", portnum);
	}

}
