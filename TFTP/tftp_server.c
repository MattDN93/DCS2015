/* Program:	
		SERVER STUB
		uses send commands to return
				
		*C stub to test creation of sockets
		*Listens for connection on port 2804
		*Use "socket_connect localhost 2804" in another terminal
		to test it!
		*NEW: telnet to this server on 2804 and see reply! 
   Module:	DCS 2015 ENEL4CC
   Name:	Matthew de Neef
   Stu. Num.	212503024			*/
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

/* to explain sockets, only these are explicitly needed */
#include <sys/types.h>
#include <sys/socket.h>
#include "cs_tftp.h"					//standardised declarations for TFTP

#define TFTP_SERVER_PORT "2804"					//well known port for TFTP
#define TFTP_BUFFER_LEN 512				//length opf data packets in TFTP
#define WRQ 2
#define ERR 5

#define TEXT 0
#define BINARY 1
#define BACKLOG 5					//number of waiting backed up connections

int n;
	/*socket destination variables*/
	char *dest;					//destination hostname
	char *destport;					//destination port
	char ipstr[INET6_ADDRSTRLEN];			//array of length of an ipv6 addr

	/*socket properties*/
	int s,c,b;					//socket descriptor/ connect result
	char *inettype;					//friendly names for getaddrinfo params
	char *socktype;
	char *prottype;	
	
	/* the server does not send R/WRQs but structs exist to receive them */
	//RRQ PACKET
	generic_packet rrq;      			// request packet
       	char* prrq = (char*)&rrq;  			// pointer to rrq packet

	//WRQ PACKET
	generic_packet wrq;      			// request packet
       	char* pwrq = (char*)&wrq;  			// pointer to rrq packet		

	//GENERIC DATA PACKET
	struct sockaddr_in dgram_dest;			//datagram destination
	char *msg_tosend;				//string of message to send
	int sndmsg_len;					//length of string above
	int bytes_sent;					//counter for bytes sent

	/* for receiving data packets from a client*/
	struct sockaddr_storage incoming_addr;		//storage for incoming address	
	char buffer[TFTP_BUFFER_LEN];				//string for received message
	int recvmsg_len;				//received length
	int bytes_recv;					//buffer to receive
	int new_sd;					//receiving socket descriptor
	int cont_recv=1;				//flag to set if data must still be received
	int total_size;				//total Rx size
	
	/*using header struct definitions for predefined packets */
	data_packet *data = (data_packet*)buffer;		//data from buffer
	ack_packet *pack = (ack_packet*)buffer;     		//for returning ACKs for data received
	error_packet *error = (error_packet*)buffer;		//for handling errors to client		
	
	/*file management */
	FILE *filePtr;					///pointer for file access
	int filetype;					//0=text, 1=binary



int main(int argc, char *argv[])			//argv[] are args passed from user in terminal
{

	int cont=0;					//var to indicate continuing execution

	struct addrinfo hints, *res, *p;		//declares a struct of type 
	int status,rv;					//integer for return flags of functions
	memset(&hints, 0, sizeof hints);		//create memory for hints

	int errno;					//error code
	
	//Welcome user with a prompt'
	printf("TFTP Server | Matthew de Neef | 212503024\n");
	
	/*	Getting address info and hints
		-----------------------------------------------------*/
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
	hints.ai_socktype = SOCK_DGRAM;			//type of socket-we want UDP!
	hints.ai_flags = AI_PASSIVE;			//use current protocol address
	//hints.ai_protocol = 0;			//any protocol accepted
	
	//GETTING ADDRESS INFO 

	/*get address info for host 	
	ARGUMENTS:	1st: IP address of host to get info (NULL as a server)
			2nd: Port, fixed at 69 for TFTP (use 2804 for custom server)
			3rd: hints struct with socket/protocol we require
			4th: the actual socket/protocol we expect the client to have
	PURPOSE:	passes the hints struct with connection requirements
			to network. We want datagram and UDP service
			if the reqs match what thTFTP_BUFFER_LENe network has, we're OK*/
	

	if ((rv = getaddrinfo(NULL, TFTP_SERVER_PORT, &hints, &res)) != 0) //errorcheck getaddrinfo
	{
		fprintf(stderr, "Connection params failed: %s\n", gai_strerror(status));
		return 2;
	}
	//dest = argv[1];					//assign input args to dest h/n
	//destport = argv[2];				//...or port respectively
	
	/*-----------------------------------------------------*/
	/*---------------------------------------------------- 
		socket creation & error handling	*/

	/* loop through all the results and bind to the first we can
	we want to bind so that the socket is accessible by clients
	binding allows data to be retrieved from the socket file descriptor*/

    	for(p = res; p != NULL; p = p->ai_next) {
        if ((s = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("Socket error.");
            continue;
        }

        if ((b = bind(s, p->ai_addr, p->ai_addrlen)) == -1) {
            close(s);
            perror("Bind error.");
            continue;
        }

        break;
    }
	if (s==-1){
		printf("Socket Creation Failed. Reason:\n");		//if socket failed
		fprintf(stderr,"\n '%s.' \n",gai_strerror(s));	//prints error message
		close(s);	
		exit(1);
	}

	if(b == -1){
		printf("\nPort binding failed. Reason: %s\n",gai_strerror(b));
		close(s);
		exit(1);
	}else{ 
		printf("Socket bound to port %s successfully\n", TFTP_SERVER_PORT);
	}	

	freeaddrinfo(res);
	
	/*-----------------------------------------------------*/
	/*-----------------------------------------------------
		Diagnostic and debug information		*/
	printf("Opening socket to listen on port: %s\n\n", TFTP_SERVER_PORT);		//else show socket

	if(res->ai_family == AF_INET6){inettype = "IPv6";}else{inettype = "IPv4";}
	if(res->ai_socktype == SOCK_DGRAM){socktype = "Datagram";}else{socktype = "Stream";}
	if(res->ai_protocol == 6){prottype = "TCP";}else{prottype = "UDP";}
	
	printf("Created socket successfully using %s, in %s mode using the %s protocol.\n\n",inettype,socktype,prottype);
	printf("\nWaiting for incoming...");
	/*-----------------------------------------------------
		RECEIVE ROUTINES
	*we expect the first incoming data packet to be a RRQ or a WRQ
	*If it is neither, simply drop the packet
	*Packet format:
		RRQ form (sprintf)	
		[opcode=1][filename][0][mode=octet][0]

		WRQ form (sprintf)	
		[opcode=2][filename][0][mode=octet][0]

	s is the stored socket descriptor for this connection

							*/
	struct sockaddr_storage incoming_addr;			//struct for incoming
	socklen_t incoming_addr_len;				//incoming size
	incoming_addr_len = sizeof(incoming_addr);

	//DEBUG BASIC CODE
	if ((bytes_recv = recvfrom(s, buffer, TFTP_BUFFER_LEN-1 , 0,
        (struct sockaddr *)&incoming_addr, &incoming_addr_len)) == -1) {
        perror("Receive error");
        exit(1);
    	}
	
	printf("Server: packet is %d bytes long\n", bytes_recv);
    	//buffer[bytes_recv] = '\0';
    	printf("Server: packet contains \"%s\"\n", buffer);

/*	ALL PREVIOUS ROUTINES!------------------------------------
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
		//TODO: type of connection RRQ or WRQ

		/* wait for client to respond 
		   NOTE: we use the NEW socket file descriptor to communicate*/
/*		while (bytes_recv != 0){
			bytes_recv = recv(new_sd,msg_received,recvmsg_len,0);
			
			
/*			//recvmsg_len = strlen(msg_received);			
			
			if ( bytes_recv ==0){printf("\nClient has disconnected!");close(new_sd);break;}
			else if (bytes_recv == -1){printf("\nError has occurred.");close(new_sd);break;}
			else {
			printf("\n------------INCOMING-----------");			
			printf("\nThis msg: %d bytes received.",bytes_recv - 2);}
			printf("\n Received: %s",msg_received);
			printf("\n------------END MESSAGE-----------");
	
			/*respond to client requests*/

			
/*			if(strstr(msg_received,"about")!=NULL) //about found in request
			{
			msg_tosend = "\nA basic socket server.\nMatthew de Neef\n 212503024 \n DCS2015\n\n";	//msg to send to user
			sndmsg_len = strlen(msg_tosend);			//set msg length
			bytes_sent = send(new_sd,msg_tosend,sndmsg_len,0);
			}

			if(strstr(msg_received,"chargen")!=NULL) //about found in request
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
		//msg_received = "";		
		//bytes_recv = 1;
/*	}//endwhile

	end ALL PREVIOUS ROUTINES!------------------------------------*/		
	printf("\nBacklog limit reached, closing.\n");
	/* close all socket descriptors*/	
	close(new_sd);
	close(s);
	
	/*if(c == -1){
		printf("\nconnection failed. Reason: %s\n",gai_strerror(c));
	}else{ 
		printf("Socket connected to %s on port %s successfully\n", dest, destport);
	}*/
	
}


