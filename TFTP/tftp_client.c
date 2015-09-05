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


int main(int argc, char *argv[])			//argv[] are args passed from user in terminal
{
	char *inarg;	
	char *arg[3];					//accepts input args from user
	char *saveptr;	
	int cont=1;					//var to indicate continuing execution

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
	hints.ai_socktype = SOCK_DGRAM;			//type of socket--we want a UDP socket for TFTP
	//hints.ai_flags = AI_PASSIVE;			//keep this DISABLED to use connect()
	hints.ai_protocol = 0;				//any protocol accepted

	//Welcome user with a prompt'
	printf("TFTP Client | Matthew de Neef | 212503024\n");
	printf("usage: <hostname to connect to> GET <filename>.\n e.g. 146.230.193.160 GET picFile \n"); 	
  do{	
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
	printf("%s\n",arg[2]);}

	if (arg[2] == NULL){			//must have 3 args before checing validity
	printf("usage: <hostname to connect to> GET <filename>. Try again.\n");}
	
	if(arg[2]!=NULL){			//this only runs if we have all 3 args
	if ((strstr(arg[1],"GET")==NULL)&&(strstr(arg[1],"get")==NULL)){
	printf("GET command missing. Try again.\n");}}

	if(arg[2]!=NULL && arg[1]!=NULL){
	if (arg[0] == "\n\0"){			//only runs if 3 args != NULL
	printf("\nSome errors encountered: \nNo hostname entered. Try again.\n");}}

 }while(cont == 1);
		
	

}
