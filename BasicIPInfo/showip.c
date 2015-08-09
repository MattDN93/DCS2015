#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
	struct addrinfo hints, *res, *p;		//declares a struct of type addrinfo 
	int status;					//integer for return flags of functions
	char ipstr[INET6_ADDRSTRLEN];			//array of length of an ipv6 addr
	
	if (argc != 2) {				//if there are <> 2 args supplied 
	fprintf(stderr,"usage: showip hostname\n");
	return 1;
}

	memset(&hints, 0, sizeof hints);			//create memory for hints
	hints.ai_family = AF_UNSPEC; 			// AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;
	
	if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) //errorcheck getaddrinfo
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	printf("IP addresses for %s:\n\n", argv[1]);		//else show IPs
	
	for(p = res;p != NULL; p = p->ai_next) 			//for each item in the 'res' linked list (the addresses returned from the hostname)
	{
	void *addr;
	char *ipver;
	char *socktype;
	// get the pointer to the address itself,
	// different fields in IPv4 and IPv6:

	if (p->ai_family == AF_INET) 
	{ // IPv4
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
		addr = &(ipv4->sin_addr);
		ipver = "IPv4";
	} else { // IPv6
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
		addr = &(ipv6->sin6_addr);
		ipver = "IPv6";
	}

	if (p->ai_socktype == SOCK_STREAM)
	{
		socktype = "Stream (TCP)";
	}else if (p->ai_socktype == SOCK_DGRAM)
	{
		socktype = "Datagram (UDP)";
	}

	// convert the IP to a string and print it:
	inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);	//network->presentation (bin->dd)
	printf(" %s: %s\n", ipver, ipstr);
	printf("Type: %s\n", socktype); 

	}
freeaddrinfo(res); // free the linked list
return 0;
}
