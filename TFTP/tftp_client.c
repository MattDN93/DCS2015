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

#define TFTP_PORT "69"					//well known port for TFTP

	/*sockets method*/
	//void startSockets(char *,char * , char *,struct addrinfo, struct addrinfo*);
	
	/*socket destination variables*/
	char *dest;					//destination hostname
	char *destport;					//destination port
	char ipstr[INET6_ADDRSTRLEN];			//array of length of an ipv6 addr

	/*socket properties*/
	int s,c;					//socket descriptor/ connect result
	char *inettype;					//friendly names for getaddrinfo params
	char *socktype;
	char *prottype;	
	
	/* for sending data these fields are needed */
	char *msg_tosend;				//string of message to send
	int sndmsg_len;					//length of string above
	int bytes_sent;					//counter for bytes sent

	/* for receiving data from a server*/
	char *msg_received;				//string for received message
	int recvmsg_len;				//received length
	int bytes_recv;					//buffer to receive
	int new_sd;					//receiving socket descriptor

	char *arg0;
	char *arg1;
	char *arg2;
int main(int argc, char *argv[])			//argv[] are args passed from user in terminal
{
	char *inarg;	
	char *arg[3];					//accepts input args from user

		
	char *saveptr;	
	int cont=0;					//var to indicate continuing execution

	struct addrinfo hints, *res, *p;		//declares a struct of type 
	int status,rv;					//integer for return flags of functions
	memset(&hints, 0, sizeof hints);		//create memory for hints
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
	arg0 = strtok_r(inarg, " ",&saveptr);
	arg1 = strtok_r(NULL, " ",&saveptr);
	arg2 = strtok_r(NULL, " ",&saveptr);	
	
	if(arg0!= NULL && arg1!=NULL && arg2!=NULL){ 	
	printf("%s\n",arg0);
	printf("%s\n",arg1);
	printf("%s\n",arg2);
	}

	if (arg2 == NULL){			//must have 3 args before checing validity
	printf("usage: <hostname to connect to> GET <filename>. Try again.\n");
	cont = 1;}
	
	if(arg2!=NULL){			//this only runs if we have all 3 args
		if ((strstr(arg1,"GET")==NULL)&&(strstr(arg1,"get")==NULL)){
		printf("GET command missing. Try again.\n");
		cont = 1;}
	}

	if(arg2!=NULL && arg1!=NULL){
		if (arg0 == "\n\0"){			//only runs if 3 args != NULL
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


	hints.ai_family = AF_UNSPEC; 			// AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_DGRAM;			//type of socket--we want a UDP socket for TFTP
	//hints.ai_flags = AI_PASSIVE;			//keep this DISABLED to use connect()
	hints.ai_protocol = 0;				//any protocol accepted

	//GETTING ADDRESS INFO FROM HOST SERVER	
	printf("Again AFTER MEMSET: %s\n",arg1);
	/*get address info for host 	
	ARGUMENTS:	1st: IP address of host to get info
			2nd: Port, fixed at 69 for TFTP
			3rd: hints struct with socket/protocol we require
			4th: the actual socket/protocol data the server has
	PURPOSE:	passes the hints struct with connection requirements
			to server. We want datagram and UDP service
			if the reqs match what the server has, we're OK*/
	
	if ((rv = getaddrinfo(arg0 , TFTP_PORT, &hints, &res)) != 0) //errorcheck getaddrinfo
	{
		fprintf(stderr, "Connection params failed: %s\n", gai_strerror(status));
		return 2;
	}

	//CALL SOCKETS
	printf("Showing arg: %s",(char *)arg2);
	printf("we're here!");

	
	dest = arg1;				//assign input args to dest h/n
	destport = TFTP_PORT;				//...or port respectively

	printf("\n%s and %s\n",arg1,arg2);
		
	printf("Opening socket for %s on dest. port: %s\n\n", arg0, TFTP_PORT);		//else show socket

	if(res->ai_family == AF_INET6){inettype = "IPv6";}else{inettype = "IPv4";}
	if(res->ai_socktype == SOCK_DGRAM){socktype = "Datagram";}else{socktype = "Stream";}
	if(res->ai_protocol == 6){prottype = "TCP";}else{prottype = "UDP";}
	
	printf("%d\n",res->ai_family);
	printf("%d\n",res->ai_socktype);
	printf("%d\n",res->ai_protocol);
	printf("family: %s \n socket type: %s \n protocol: %s \n",inettype,socktype,prottype);
	
	/*action section - create socket and connect*/
	s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);	//tries to pass to socket()
	c = connect(s, res->ai_addr, res->ai_addrlen);

	/* for receiving socket setup we need storage for structs */
	struct sockaddr_storage incoming_addr;		//struct for incoming
	socklen_t incoming_addr_size;				//incoming size

	if (s==-1){
		printf("Socket Creation Failed. Reason:\n");		//if socket failed
		if (res->ai_flags == 0){printf("The remote socket hasn't been setup yet OR incorrect port specified.");}
		else{
		fprintf(stderr,"\n '%s.' \n",gai_strerror(s));	//prints error message
		}	
	}
	else{
		printf("Created socket successfully using %s, in %s mode using the %s protocol.\n\n",inettype,socktype,prottype);
		printf("\nSending chargen command automatically!");

		/*issues send command to server*/
		msg_tosend = "GET picFile";					//msg to send to server		
		sndmsg_len = strlen(msg_tosend);			//define the message length
		send(s,msg_tosend,sndmsg_len,0);			//s is the stored socket descriptor for this connection*/

		listen(s,1);						//listen on socket server is using for msgs from it
		
		//...when connection incoming...
		incoming_addr_size = sizeof(incoming_addr);		//make space in mem for struct
		new_sd = accept(s, (struct sockaddr *)&incoming_addr, &incoming_addr_size);
		
		/*while socket connection remains open, receive chargen from server*/
		while (bytes_recv >= 0){				//as long as server messages non-empty, receive them
			bytes_recv = recv(new_sd,msg_received,recvmsg_len,0);
			//printf("%s",msg_received);				//get data and print out received msg
		bytes_recv = 1;
		}
		if (msg_received == NULL)
		{
			printf("\nServer response error. Message not received.\n");
		}
	}

	if(c == -1){
		printf("\nconnection failed. Reason: %s\n",gai_strerror(c));
	}else{ 
		printf("Socket connected to %s on port %s successfully\n", dest, destport);
	}
}



