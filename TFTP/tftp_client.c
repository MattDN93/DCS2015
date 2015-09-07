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
#include <errno.h>

/* to explain sockets, only these are explicitly needed */
#include <sys/types.h>
#include <sys/socket.h>
#include "cs_tftp.h"					//standardised declarations for TFTP

#define TFTP_PORT "69"					//well known port for TFTP
#define TFTP_BUFFER_LEN 512				//length opf data packets in TFTP
#define WRQ 2
#define ERR 3
	/*sockets method*/
	//void startSockets(char *,char * , char *,struct addrinfo, struct addrinfo*);
	

	int n;
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
	//RRQ PACKET
	generic_packet rrq;      			// request packet
       	char* prrq = (char*)&rrq;  			// pointer to rrq packet 	
	
	//GENERIC DATA PACKET
	struct sockaddr_in dgram_dest;			//datagram destination
	char *msg_tosend;				//string of message to send
	int sndmsg_len;					//length of string above
	int bytes_sent;					//counter for bytes sent

	/* for receiving data from a server*/
	struct sockaddr_storage incoming_addr;		//storage for incoming address	
	char buffer[TFTP_BUFFER_LEN];				//string for received message
	int recvmsg_len;				//received length
	int bytes_recv;					//buffer to receive
	int new_sd;					//receiving socket descriptor

	data_packet *data = (data_packet*)buffer;		//data from buffer
	ack_packet *pack = (ack_packet*)buffer;     	//for returning ACKs for data received

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

	int errno;					//error code
	
	//Welcome user with a prompt'
	printf("TFTP Client | Matthew de Neef | 212503024\n");
	printf("usage: <hostname to connect to> GET <filename>.\n e.g. 146.230.193.160 GET picFile \n"); 	

do{	
	cont = 0;
	printf("tftp>");
	fgets(inarg,50,stdin);				//get line from keyboard
	
	/* TOKENIZE INPUT INFO:
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
	hints.ai_flags = AI_PASSIVE;			//keep this DISABLED to use connect()
	hints.ai_protocol = 0;				//any protocol accepted

	//GETTING ADDRESS INFO FROM HOST SERVER	
	//printf("Again AFTER MEMSET: %s\n",arg1);
	/*get address info for host 	
	ARGUMENTS:	1st: IP address of host to get info
			2nd: Port, fixed at 69 for TFTP
			3rd: hints struct with socket/protocol we require
			4th: the actual socket/protocol data the server has
	PURPOSE:	passes the hints struct with connection requirements
			to server. We want datagram and UDP service
			if the reqs match what the server has, we're OK*/
	
	if ((rv = getaddrinfo("146.230.193.160", TFTP_PORT, &hints, &res)) != 0) //errorcheck getaddrinfo
	{
		fprintf(stderr, "Connection params failed: %s\n", gai_strerror(status));
		return 2;
	}

	
	//-----------------------------------------------------------------------------------------------
	//CALL SOCKETS
	
	dest = arg0;				//assign input args to dest h/n
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
	//s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);	//tries to pass to socket()

	
	//LOOPING THROUGH RESULTS TO MAKE A SOCKET
	// loop through all the results and make a socket
	for(p = res; p != NULL; p = p->ai_next) {
		if ((s = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
		{
		perror("socket:creation");
		continue;
		}
		break;		//as soon as first socket made, quit loop
	}


	
		//if p isn't assigned after the loop it means so sockets bound
		if (res== NULL){
		fprintf(stderr, "Error. Socket binding failed.\n");	
		}else{
		printf("Socket for %s on port %s ready.\n",arg0,TFTP_PORT);
		}

	if (s==-1){
		printf("Socket Creation Failed. Reason:\n");		//if socket failed
		if (res->ai_flags == 0){printf("The remote socket hasn't been setup yet OR incorrect port specified.");}
		else{
		fprintf(stderr,"\n '%s.' \n",gai_strerror(s));	//prints error message
		}	
	}
	else{
		printf("Created socket successfully using %s, in %s mode using the %s protocol.\n\n",inettype,socktype,prottype);

		//-----------------------------------------------------------------------------------------------		
		/*DATAGRAM SEND
		using sendto() needs extra vars from p addrinfo stuct
		allows UDP packet to have dest IP addr and defines its length.
		in this case we only send the server GET <filename>
		
		RRQ form (sprintf)	
		[opcode=1][filename (arg2)][0][mode][0]
		
		s is the stored socket descriptor for this connection

		OUTPUT: # bytes sent; error condition if fail		
		*/

		//----------------------------------
		//REMOVE TRAILING crlf on filename
		arg2[strcspn(arg2, "\n")] = 0;
		//----------------------------------

		rrq.opcode = htons(RRQ);	//opcode = 1 (RRQ) use host-to-network!!
		sprintf((char *)&(rrq.info), "%s%c%s%c", arg2, '\0', "octet", '\0');
		printf("%s",rrq.info);	
		
		if((n = sendto(s,&rrq,24,0,res->ai_addr, res->ai_addrlen))==-1){
			perror("Sending command to server failed.");	//if send fails, quit send process	
			exit(1);
		}

		//if no error, show stats
		printf("sent %d bytes to %s\n",bytes_sent,arg0);
		//freeaddrinfo(res);					//we're done with server port/addrinfo
		//-----------------------------------------------------------------------------------------------
		/*DATAGRAM RECEIVE & ACK
		using recvfrom(), we expect the first packets to come through
		as long as there is still data arriving, construct and ACK and return it
		ACK form (sprintf)	
		[opcode=4][block number]
		
		s is the stored socket descriptor for this connection

		OUTPUT: sends return ACKs as each block received	
		*/


				
		/* for receiving socket setup we need storage for structs */
		struct sockaddr_storage incoming_addr;			//struct for incoming
		socklen_t incoming_addr_len;				//incoming size
		incoming_addr_len = sizeof(incoming_addr);		//make space in mem for struct
		
		bytes_recv = recvfrom(s,buffer,sizeof(buffer),0,(struct sockaddr *)&incoming_addr, &incoming_addr_len);
		//continuous process for all data blocks
		while (bytes_recv > 0){				//as long as server messages non-empty, receive them
		bytes_recv = recvfrom(s,buffer,sizeof(buffer),0,(struct sockaddr *)&incoming_addr, &incoming_addr_len);
		n = bytes_recv;
		 data->data[bytes_recv-4] = '\0';
		//DEBUG output stats
		printf("Received packet type %d, block %d, data: %s\n",
		       ntohs(data->opcode), ntohs(data->block_number),
		       data->data);
		
		//build ACK for current block and send	
		pack->opcode = htons((u_short)ACK);
		pack->block_number = data->block_number;		

		bytes_sent = sendto(s, (void*)pack, 4, 0,(struct sockaddr *)&incoming_addr, incoming_addr_len);
		
		//check ACK validity
		if (bytes_sent>0){
			printf("ACK sent.\n");			
		}else printf("ACK failed.\n");
		
		//check if connection is ready to be closed
		//if datalength <512 bytes (+46 header), close the socket.
		if(n < 512){
			printf("File received. Closing socket");
			close(s);
			exit(0);}	
	}//end loop to receive

		if (bytes_recv == -1)
		{
			printf("\nServer response error: %s\n",gai_strerror(errno));
		}else{
			printf("\n File received.\n");
			//close(s);}
			}
	}

	if(c == -1){
		printf("\nconnection failed. Reason: %s\n",gai_strerror(c));
	}
}



