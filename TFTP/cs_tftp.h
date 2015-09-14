/* tftp.h
 *
 * Header file containing various declarations in support of
 * Trivial File Transfer Protocol (Revision 2), RFC 1350
 *
 *
 * Assumptions:
 *    Requires 32-bit int, 16-bit short, 8-bit char
 *
 * Author:  Roy Levow
 * Date:    20 October 1999
 * Revision History:
 *
 */

#ifndef _TFTP_H
#define _TFTP_H

enum packet_type {RRQ=1, WRQ=2, DATA=3, ACK=4, ERROR=5};

enum filemode {NETASCII, OCTET, MAIL};
static char *filemode[] = {
    "netascii",
    "octet",
    "mail"
};

enum error_codes {NOTDEFERR, NOTFOUNDERR, ACCESSERR, DISKFULLERR, 
		  ILLEGALOPERR, UNKNOWNIDERR, FILEEXISTSERR, 
		  UNKNOWNUSERERR};

static char *errormsg[] = {
    "Not defined, see error message",
    "File not found",
    "Access violation",
    "Disk full or allocation exceeded",
    "Illegal TFTP operation",
    "Unknown transfer ID",
    "File already exists",
    "No such user"
};

typedef
    struct generic_packet {
	 short unsigned opcode;
	 char info[514];
     } generic_packet;

typedef
     struct data_packet {
         short unsigned opcode;  /* 3 */
         short unsigned block_number;
         char data[512];
     } data_packet;

typedef
     struct ack_packet {
         short unsigned opcode;  /* 4 */
         short unsigned block_number;
     } ack_packet;

typedef
     struct error_packet {
         short unsigned opcode;  /* 5 */
         short unsigned error_code;
         char error_msg[512];
     } error_packet;

#endif

