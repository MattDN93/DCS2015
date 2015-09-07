/* tftp_client.c
 * modified from code for example client program that uses TCP
 * Original author:  D. Comer
 * Modifications by R. Levow
 *
 * Ver 1.01, 11 November 1999
 *    Corrected defect in passing argument for ACK packet in last sendto()
 */

#include "cs_tftp.h"

#ifndef __GNUC__
#include <windows.h>
#include <winsock.h>
#else
#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <stdio.h>
#include <string.h>

#define PROTOPORT       69              /* tftp protocol port number */

extern  int             errno;
char    localhost[] =   "localhost";    /* default host name            */
/*------------------------------------------------------------------------
 * Program:   client
 *
 * Purpose:   allocate a udp socket, connect to a tftp server, 
 *                 send read request for file f1 and receive first
 *                 data packet
 *
 * Syntax:    client [ host ]
 *
 *               host  - name of a computer on which server is executing
 *
 *------------------------------------------------------------------------
 */
main(argc, argv)
int     argc;
char    *argv[];
{
        struct  hostent  *ptrh;  /* pointer to a host table entry       */
        struct  protoent *ptrp;  /* pointer to a protocol table entry   */
        struct  sockaddr_in sad; /* structure to hold an IP address     */
        int     sd;              /* socket descriptor                   */
        int     port;            /* protocol port number                */
        char    *host;           /* pointer to host name                */
        int     n;               /* number of characters read           */
        char    buf[1000];       /* buffer for data from the server     */

       	generic_packet rrq;      /* request packet */
       	char* prrq = (char*)&rrq;  /* pointer to rrq packet */
       	data_packet *data = (data_packet*)buf;  /* data packet in buffer */
	ack_packet *pack = (ack_packet*)buf;     /* ack packet in buffer  */

	struct sockaddr fskt;    /* for recvfrom */
	int fskt_len = sizeof(fskt);

#ifdef WIN32
        WSADATA wsaData;
        WSAStartup(0x0101, &wsaData);
#endif
        memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
        sad.sin_family = AF_INET;           /* set family to Internet   */

        /* Set port number for ftfp                                     */

        port = PROTOPORT;
	sad.sin_port = htons((u_short)port);

        /* Check host argument and assign host name. */

        if (argc > 1) {
                host = argv[1];         /* if host argument specified   */
        } else {
                host = localhost;
        }

        /* Convert host name to equivalent IP address and copy to sad. */

        ptrh = gethostbyname(host);
        if ( ((char *)ptrh) == NULL ) {
                fprintf(stderr,"invalid host: %s\n", host);
                exit(1);
        }
        memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

        /* Map UDP transport protocol name to protocol number. */

        if ( ((int)(ptrp = getprotobyname("udp"))) == 0) {
                fprintf(stderr, "cannot map \"udp\" to protocol number");
                exit(1);
        }

        /* Create a datagram socket; do not connect. */

        sd = socket(PF_INET, SOCK_DGRAM, ptrp->p_proto);
        if (sd < 0) {
                fprintf(stderr, "socket creation failed\n");
                exit(1);
        }

	/* construct rrq packet */
	rrq.opcode = htons((u_short)RRQ);
	sprintf((char *)&(rrq.info), "%s%c%s%c", "picFile", '\0', "octet", '\0');

	/* send rrq packet to server */
	n = sendto(sd, (void*)&rrq, 24, 0, 
	       (struct sockaddr *)&sad, sizeof(sad));  /* send wrq */

	printf("sent packet %X %X %s %s\nWaiting for reply\n",
	       prrq[0], prrq[1], prrq+2, prrq+5);

	/* get response, should be first data block of file, and print it */
        n = recvfrom(sd, buf, sizeof(buf), 0, &fskt, &fskt_len);
        while (n > 0) {
	n = recvfrom(sd, buf, sizeof(buf), 0, &fskt, &fskt_len);
	        data->data[n-4] = '\0';
                printf("Received packet type %d, block %d, data: %s\n",
		       ntohs(data->opcode), ntohs(data->block_number),
		       data->data);

		/* build ack packet */
		pack->opcode = htons((u_short)ACK);
		pack->block_number = data->block_number;

		/* send ack packet to server on new port */
		n = sendto(sd, (void*)pack, 4, 0, 
		       (struct sockaddr *)&fskt, fskt_len);
		if (n > 0){
		       printf("ACK sent\n");
		}
		else
		       printf("Unable to send ACK\n");
        }
	
	        printf("No response from server\n");

        /* Close the socket. */

        closesocket(sd);

        /* Terminate the client program gracefully. */

        exit(0);
}
