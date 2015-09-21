/* Program:	SERVER FOR TFTP CONNECTION
		PURPOSE:    Provides server to read/write files in its directory to a client
                    Allows for transferring of files to a remote client
                    Also receiving files from a remote client
                    Works on port 2804, passive open
                    Uses TFTP packet protocol (WRQ, RRQ)
                    Includes error checking and file output
        PREREQ:     File "cs_tftp.h" must exist to compile
                    Runs on Ubuntu 14.04+
                    Compiled with gcc and debugged in Code::Blocks
                    If sending files to client, file must be in same directory as server
        SYNTAX:     <hostname> GET|PUT <filename>
        GITHUB:     http://github.com/MattDN93/DCS2015/TFTP
        CREDIT:     *Roy Levow, Florida Atlantic University CS
                        >"cs_tftp.h" modified from his original work
                        >RRQ, WRQ packet construction credit his TFTP course
                    *Beej's Guide to Network Programming
                        >Socket layout
                        >UDP routines
                        >getaddrlen() details

   Module:	DCS 2015 ENEL4CC
   Name:	Matthew de Neef
   Stu. Num.	212503024			*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

/*-----------TFTP VARIABLE DEFINES---------------------------
-Gives port data, andbuffer lengths, may be modified as needed
-Don't reduce TFTP buffer <512, changes protocol
-Opcodes are standard for TFTP, don't modify
-Text/binary define write/read mode for files
-get/put allows correct routine to run based on user input
----------------------------------------------------------- */
#include <sys/types.h>
#include <sys/socket.h>
#include "cs_tftp.h"					//standardised declarations for TFTP

#define TFTP_SERVER_PORT "2804"					//well known port for TFTP
#define TFTP_BUFFER_LEN 558				//length opf data packets in TFTP
#define FBUFFER_LEN 10000000				//max buffer size

#define RRQ 1                       //opcode # for a RRQ
#define WRQ 2                       //opcode for write req
#define ACK 4                       //opcode for ack
#define ERR 5                       //opcode for error

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

	error_packet errpack;
	char* perr = (char*)&errpack;

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
	char *fbuffer;			//file buffer
    char filename_req[TFTP_BUFFER_LEN];
    char filename_dest[TFTP_BUFFER_LEN];
	/*mode of operation*/
	int server_mode;

/*stub to append to strings */
int  append(char*s, size_t size, char c) {
     if(strlen(s) + 1 >= size) {
          return 1;
     }
     int len = strlen(s);
     s[len] = c;
     s[len+1] = '\0';
     return 0;
}

// from Beej's guide - getting the IP address of client
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

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

/*-----------SOCKET METHODS---------------------------
-All the methods to set up a sockeet for communication
-Process:
    >addressinfo details shown to user, properties of connection
    >socket created, file descriptor stored for comms
    >data exchanged on socket once confirmedby server

CLIENT (2804)----------->Initial Setup-------------------->(2804)SERVER
CLIENT (ephemeral)<------Return ACK<-----------------------(2804)SERVER
CLIENT (ephemeral)<----->Data Tx/Rx<--------------(new ephemeral)SERVER
*/

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

	//freeaddrinfo(res);
    // MRDDN 2015 21253024
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

/*	---------------------------------------------------------
	INFORMATIONAL: Shows info about the type of packet received
		Purpose: 	Shows the type of TFTP packet (WRQ, RRQ, ERR)
				Shows IP address of client
				Filename to read/write
									*/

	//receive the first packet from the client & address details
	if ((bytes_recv = recvfrom(s, buffer, TFTP_BUFFER_LEN-1 , 0,
        (struct sockaddr *)&incoming_addr, &incoming_addr_len)) == -1) {
        	perror("Receive error");
        	exit(1);
    	}

	//Allows server to display IP address of client
	inet_ntop(incoming_addr.ss_family, get_in_addr((struct sockaddr *)&incoming_addr),ipstr, sizeof(ipstr));
        printf("Server: Connection from: %s\n", ipstr);

	//Displays size of the packet
	printf("Server: packet is %d bytes long\n", bytes_recv);
    	//buffer[bytes_recv] = '\0';

    /*-----------PACKET DETECTION---------------------------
    -The server must know what the client wants from it
    - A read request means the server must send the client a file (RRQ)
    - A write request means the server must receive a file from client (WRQ)
    - packets segmented with opcode and data                                    */

	printf("---------Packet Information-------\n");
	int opcode;
	opcode = buffer[1] ;
	printf("Opcode:\t\t%d\n",buffer[1]);

	if(opcode == 1){			    //we have a RRQ so store it
		rrq.opcode = htons(RRQ);
		printf("Type:\t\tRRQ\n");
		server_mode = RRQ;
	}
	else if (opcode == 2){			//we have a WRQ so store it
		wrq.opcode = htons(WRQ);
		printf("Type:\t\tWRQ\n");
		server_mode = WRQ;
	}else{					        //error occurred
		errpack.opcode = htons(ERR);
		printf("Type:\t\tERROR\n");
		server_mode = ERR;
	}

	/* FRAME: [opcode][filename][0][mode][0] */
	int i=2;			//set pointers where each element starts
	printf("Filename:\t");			//display filename

	//buffer the filename into its own char array
	while((i < sizeof(buffer)) && buffer[i] != '\0' ){
	printf("%c",buffer[i]);
	filename_req[i-2] = buffer[i];
	i++;
	}
	filename_req[i-1] = '\0';		//truncate filename string

	printf("\nMode:\t\t");			//display mode
	i+=1;					//advance pointer
	while((i < sizeof(buffer)) && buffer[i] != '\0' ){
	printf("%c",buffer[i]);
	i++;
	}


/*	--------------------------------------------------------- */
// 212503024 Matthew de Neef
/*	---------------------------------------------------------
	GET ROUTINES
	Catagorizes the packet based on request type, then respond
	If RRQ:	Accept filename, try find file, if not found, assemble
		error packet and return it. Use incoming_addr to get correct port
		If found, split data into 512 byte packets and send. Only send
		next packet if the ACK for it has been received.
	*/


	if (server_mode == RRQ){

		//---------Search for the file requested------//
		if(((filePtr = fopen(filename_req,"rb"))==NULL)){
			//assemble the error package for the client
			printf("\nFile not found, sending error");

			errpack.opcode = htons(ERR);
			errpack.error_code = htons(NOTFOUNDERR);
			sprintf((char *)&(errpack.error_msg), "%s%c",errormsg[NOTFOUNDERR],'\0');

			if((n = sendto(s,&errpack,24,0,(struct sockaddr *)&incoming_addr, incoming_addr_len))==-1){
			perror("Sending failed");			//if send fails, quit send process
			exit(1);}
		}else{
			//prepare to send the file!
			//read file into buffer
			printf("\nFile found, sending data");

			if ( filePtr != NULL )
  			{
    			fseek(filePtr, 0L, SEEK_END);
    				long s = ftell(filePtr);
    				rewind(filePtr);
    				fbuffer = malloc(s);
    				if ( fbuffer != NULL )
    				{
      					fread(fbuffer, s, 1, filePtr);
     					// we can now close the file
      					fclose(filePtr); filePtr = NULL;
				}
			}

			//now file is in buffer, segment it in 512 bytes
			//assemble data packet!
			int i,j,blkcount;
			i=0;j=0;			//marks pointer within block
			blkcount=0;			//number of data blocks
			int eofmrk=0;			//marks if EOF reached

		while(eofmrk == 0){			//as long as EOF not reached
			data->opcode = htons(DATA);		//sets data opcode to DATA
			data->block_number = htons(blkcount);	//sets block #
			printf("\n----Sending block %d,",blkcount+1);

			for(i=0;i<512;i++){		//clear data array
				data->data[i]='0';
			}

			//populate this data block with file data
			for(i=0;i<512;i++)
			{
                	//do a check for the last block -> if endln char
				if(fbuffer[(512*blkcount)+i] == '\0')
				{
				eofmrk=1;
				data->data[i] = fbuffer[(512*blkcount)+i];
				data->data[i+1] = '\0';                     //terminate block early!
				break;
				}

				data->data[i] = fbuffer[(512*blkcount)+i];  //else just do a normal block
			}

			//send the data to the client, with error checking
			//46 bytes of encapsulated header included (512 + 46 = 558)
			printf(" with size: %li.----\n",strlen(data->data));
            printf(data->data);
			if(eofmrk == 0)
			{        //use full-size packet if it's not the last one
                		if((bytes_sent = sendto(s, (void*)data, 558, 0,(struct sockaddr *)&incoming_addr, incoming_addr_len))==-1)
                		{
				perror("Data send failed");
				exit(1);
                		}
			}
			else if (eofmrk == 1)
			{  //else use truncated packet length!
                sndmsg_len = strlen(data->data);             //send modified packet size + header
                sndmsg_len += 46;                           //consider header length
                		if((bytes_sent = sendto(s, (void*)data, sndmsg_len, 0,(struct sockaddr *)&incoming_addr, incoming_addr_len))==-1)
                		{
				perror("Data send failed");
				exit(1);
                		}

			}

			//await ACK from client before sending next block
			bytes_recv = recvfrom(s,buffer,sizeof(buffer),0,(struct sockaddr *)&incoming_addr, &incoming_addr_len);
			opcode = buffer[1];
			printf("\nReceived ACK opcode: %d",opcode);

			//only start sending the next block if the ACK is valid!
			if(opcode == ACK)
			{
				if(pack->block_number == data->block_number)
				{
					data->block_number += 1;
					blkcount++;
                    			total_size +=(bytes_sent - 46);
				}
			}


        }//eof noted!

		printf("\n file sent to client.");
		printf("\n Total size %d.",total_size);
		}

	}else if (server_mode == WRQ){
    // 212503024 Matthew de Neef
        //before receiving first block, send an ACK to bein data transfer
         pack->opcode = htons((u_short)ACK);				//ACK the file not found!
         pack->block_number = data->block_number;

         //send ACK
                bytes_sent = sendto(s, (void*)pack, 4, 0,(struct sockaddr *)&incoming_addr, incoming_addr_len);

         if (bytes_sent>0)
                {
                    printf("\n\nACK sent.\n");
                }else {printf("ACK failed.\n"); exit(1);}

                //the file can now be created for writing to
                filename_req[strcspn(filename_req, "\n")] = 0;
	//----------------------------------

                if((strstr(filename_req,"txt")!=NULL)){			//the filename is a text file!
                sprintf(filename_dest, "%s%s","Srv_text_rx_",filename_req);
                filePtr = fopen(filename_dest,"w");			//creates file to write to
                if (filePtr == NULL){ printf("\nFile I/O error.");}
                filetype = TEXT;					//TEXT FILE
                }

                else if ((strstr(filename_req,"pic")!=NULL)){			//picture file
                sprintf(filename_dest, "%s%s","Srv_pic_rx_",filename_req);
                filePtr = fopen(filename_dest,"wb");			//binary write mode!
                filetype = BINARY;					//BINARY FILE
                if (filePtr == NULL) {printf("\nFile I/O error.");}
                }

                else if ((strstr(filename_req,"movie")!=NULL)){			//video file
                sprintf(filename_dest, "%s%s","Srv_mov_rx_",filename_req);
                filePtr = fopen(filename_dest,"wb");			//binary write mode!
                filetype = BINARY;					//BINARY FILE
                    if (filePtr == NULL){ printf("\nFile I/O error.");}
                }else{
                sprintf(filename_dest, "%s%s","Srv_rx_",filename_req);
                filePtr = fopen(filename_dest,"wb");     //default file case
                filetype = BINARY;
                    if (filePtr == NULL){ printf("\nFile I/O error.");
                    exit(1);
                        }
                }
	//-----------------------------------------------------------------------------------------------
		/*DATAGRAM RECEIVE & ACK        // MRDDN 2015 21253024
		using recvfrom(), we expect the first packets to come through
		as long as there is still data arriving, construct and ACK and return it
		ACK form (sprintf)
		[opcode=4][block number]

		s is the stored socket descriptor for this connection

		OUTPUT: sends return ACKs as each block received
		*/

		//continuous process for all data blocks

	//RECEVING DATA PROCESS--------------------------------------------------------
            while (cont_recv > 0)
            {				//as long as server messages non-empty, receive them
            //receive data!
                int i=0;
                for(i=0;i<512;i++)
                {		//clear data array
                    data->data[i]='0';
                }

                bytes_recv = recvfrom(s,buffer,TFTP_BUFFER_LEN,0,(struct sockaddr *)&incoming_addr, &incoming_addr_len);


                //error check - does file exist?
                if((ntohs(data->opcode)) == ERR)
                {
                printf("Error: %s\n",data->data);
                cont_recv = 0;							//exit loop for Rx
                fclose(filePtr);						//close file I/O

                pack->opcode = htons((u_short)ACK);				//ACK the file not found!
                pack->block_number = data->block_number;

                close(s);							//close socket
                exit(0);
                }


                n = bytes_recv - 46;               //account for header overhead
                data->data[bytes_recv-4] = '\0';

                //add packet size to total
                total_size += (bytes_recv - 46);   //again header overhead

                //DEBUG output stats
                printf("\n-----------------------------------------\n");
                printf("Received packet type %d, block %d, data:\n %s\n",
                   ntohs(data->opcode), ntohs(data->block_number),
                   data->data);
                printf("\n-----------------------------------------\n");

            /*write to text/binary file here*/
                if(filetype == TEXT)
                {
                    fputs(data->data, filePtr);		//write to file
                }else if (filetype == BINARY)
                {
                    fputs(data->data, filePtr);
                }

                //build ACK for current block and send
                pack->opcode = htons((u_short)ACK);
                pack->block_number = data->block_number;

                //send ACK
                bytes_sent = sendto(s, (void*)pack, 4, 0,(struct sockaddr *)&incoming_addr, incoming_addr_len);

                //check ACK validity
                if (bytes_sent>0)
                {
                    printf("ACK sent.\n");
                }else printf("ACK failed.\n");

                //check if connection is ready to be closed
                //if datalength <512 bytes (+46 header), close the socket.
                //note subtract the last ACK header for total filesize
                if(n < 512)
                {
                    printf("\nFile size: %d bytes\n",total_size ); 			//display total size of file
                    printf("File received from client. Check folder for contents. Closing socket\n");
                    cont_recv = 0;							//exit loop for Rx
                    fclose(filePtr);						//close file I/O
                    close(s);							//close socket
                    exit(0);
                }							//quit program
            }//end while loop to receive
    //end WRQ
    }else if (server_mode == ERR){

	}

	printf("\nBacklog limit reached, closing.\n");
	/* close all socket descriptors*/
	close(new_sd);
	close(s);
    // MRDDN 2015 21253024

}


