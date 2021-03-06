/* Program:	
		SERVER STUB
		uses send commands to return
				
	FEATURES:	*Allows clients to connect on port 2804
			*Not necessary to supply runtime args
			*forks appropriate socket descriptor
			*Sends client options [only if using built-in TELNET]:
				-client sending server 'about' returns an about menu
				-client sending server 'chargen' returns a chargen response
			*Server shows all received data (anything typed in client)
			*Shows details of connection and data bytes send/received
			*Uses TCP socket connections, credit to Beej's guide for framework
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

/* for servers listening - pick an open port if <> localhost */
#define LOCALPORT "2804"				//users will connect to this port
#define BACKLOG 5					//number of waiting backed up connections



int main(int argc, char *argv[])			//argv[] are args passed from user in terminal
{
	//-------------------------INITIALIZATION-------------------------------------
	struct addrinfo hints, *res, *p;		//declares a struct of type addrinfo 
	int status;					//integer for return flags of functions
	int s,b,c,a;					//socket descriptor/ connect result
	char *inettype;					//friendly names for getaddrinfo params
	char *socktype;
	char *prottype;	
	int portnum;
	char ipstr[INET6_ADDRSTRLEN];			//array of length of an ipv6 addr

	/* for receiving socket setup we need storage for structs */
	struct sockaddr_storage incoming_addr;		//struct for incoming
	socklen_t incoming_addr_size;				//incoming size
	int new_sd;					//new socket descripter after accept()
	int bl=0;

	/* for sending data these fields are needed */
	char *msg_tosend;				//string of message to send
	int sndmsg_len;					//length of string above
	int bytes_sent;					//counter for bytes sent

	/* for receiving data from a clinet*/
	char *msg_received;				//string for received message
	int recvmsg_len;				//received length
	int bytes_recv;					//buffer to receive
	

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
	hints.ai_socktype = 0;				//type of socket CL or CO
	hints.ai_flags = AI_PASSIVE;			//keep this DISABLED to use connect()
	hints.ai_protocol = 0;				//any protocol accepted
	
	//WE don't need address info, and we specify a local port to listen on

	if ((getaddrinfo(NULL, LOCALPORT, &hints, &res)) != 0) //errorcheck getaddrinfo
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}
	
	//-------------------------INFORMATIONAL-------------------------------------	
	printf("Opening socket to listen on port: %s\n\n", LOCALPORT);		//else show socket

	if(res->ai_family == AF_INET6){inettype = "IPv6";}else{inettype = "IPv4";}
	if(res->ai_socktype == SOCK_DGRAM){socktype = "Datagram";}else{socktype = "Stream";}
	if(res->ai_protocol == 6){prottype = "TCP";}else{prottype = "UDP";}
	
	printf("family: %s \n socket type: %s \n protocol: %s \n",inettype,socktype,prottype);
	
	//-------------------------SOCKET CONENCTION-------------------------------------	
	// Creates a socket with populated addresses from getaddrinfo
	// Binds the socket to port 2804
	s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);	//tries to pass to socket()
	b = bind(s,res->ai_addr,res->ai_addrlen);	
		

	//-------------------------ERROR CHECKING-------------------------------------
	if (s==-1){
		printf("Socket Creation Failed. Reason:\n");		//if socket failed
		fprintf(stderr,"\n '%s.' \n",gai_strerror(s));	//prints error message
		close(s);
	}
	else{
		printf("Created socket successfully using %s, in %s mode using the %s protocol.\n\n",inettype,socktype,prottype);
	}

	if(b == -1){
		printf("\nPort binding failed. Reason: %s\n",gai_strerror(b));
		close(s);
	}else{ 
		printf("Socket bound to port %s successfully\n", LOCALPORT);
	}

	/*now listen for incoming...*/
	
	//-------------------------CONNECTION SEND/RECIEVE-------------------------------------
	while (bl <5 && b>=0 && s>=0)
	{	
		printf("\nnow listening on port...\n");	
		listen(s,BACKLOG);					//listen on socket we created 
	
		//...when connection incoming...
		incoming_addr_size = sizeof(incoming_addr);		//make space in mem for struct
		new_sd = accept(s, (struct sockaddr *)&incoming_addr, &incoming_addr_size);
		bl++;							//accept connection with given parameters
	
		printf("\nConnection detected! Forking socket descriptor now...");
		printf("\nNew connection ready on descriptor %d",new_sd);
		printf("\nSending response to user on fd: %d", new_sd);

		/*send welcome to client*/
		
		msg_tosend = "\nDCS 2015 Sockets Tut 1; Hello World!!";	//msg to send to user
		sndmsg_len = strlen(msg_tosend);			//set msg length
		bytes_sent = send(new_sd,msg_tosend,sndmsg_len,0);
		msg_tosend = "\nType 'about' to get details about this server.\n";	//send it!
		sndmsg_len = strlen(msg_tosend);		
		bytes_sent += send(new_sd,msg_tosend,sndmsg_len,0);
		msg_tosend = "\nType 'chargen' to get a chargen response.\n";	//send it!
		sndmsg_len = strlen(msg_tosend);		
		bytes_sent += send(new_sd,msg_tosend,sndmsg_len,0);
		msg_tosend = "\nNOTE: If running the custom client, this doesn't apply. Chargen is starting now.\n";	//send it!
		sndmsg_len = strlen(msg_tosend);		
		bytes_sent += send(new_sd,msg_tosend,sndmsg_len,0);

		//-------------------------DETAILS OF CONENCTION-------------------------------------
		// This displays details about anything the client is sending 
		// Returns info messages based on client request. Sends required chargen response if 
		// client requests it.		
		while (bytes_recv != 0){
			bytes_recv = recv(new_sd,msg_received,recvmsg_len,0);
			
			
			//recvmsg_len = strlen(msg_received);			
			
			if ( bytes_recv ==0){printf("\nClient has disconnected!");close(new_sd);break;}
			else if (bytes_recv == -1){printf("\nError has occurred.");close(new_sd);break;}
			else {
			printf("\n------------INCOMING-----------");			
			printf("\nThis msg: %d bytes received.",bytes_recv - 2);}
			printf("\n Received: %s",msg_received);
			printf("\n------------END MESSAGE-----------");
	
			/*respond to client requests*/

			
			if(strstr(msg_received,"about")!=NULL) //about found in request
			{
			msg_tosend = "\nA basic socket server.\nMatthew de Neef\n 212503024 \n DCS2015\n\n";	//msg to send to user
			sndmsg_len = strlen(msg_tosend);			//set msg length
			bytes_sent = send(new_sd,msg_tosend,sndmsg_len,0);
			}

			if(strstr(msg_received,"chargen")!=NULL) //CHARGEN found in request
			{
			printf("\nsending chargen response!");
			while(1)
			{
				msg_tosend = "!@#$%^&*()_+<>:{}|<>? abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890\n";	//msg to send to user
				sndmsg_len = strlen(msg_tosend);			//set msg length
				bytes_sent = send(new_sd,msg_tosend,sndmsg_len,0);
			}			
			}

			msg_received[0] = "\0";		//clear the rx buffer

			bytes_recv = 1;		
		}
		
		
		printf("\nA total of %d bytes sent.",bytes_sent);
		printf("\nThat's %d connections so far.",bl);
		printf("\n-------------------------------------");
		
		/* Reset received buffer and bytecount. Bytecount must
		   be nonzero to prevent connection closing */
		msg_received = "";		
		bytes_recv = 1;
		//Close socket descriptors in case user ^C out, else socket won't close
		close(new_sd);
		close(s);
		//New sock file descriptors will be created every connection, so can close this session
	}

	printf("\nBacklog limit reached, closing.\n");
	/* close all socket descriptors*/	
	close(new_sd);
	close(s);
	

	
}
