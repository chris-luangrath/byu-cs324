// Replace PUT_USERID_HERE with your actual BYU CS user id, which you can find
// by running `id -u` on a CS lab machine.
#define USERID 1823700709
// #define USERID 12345
// #define BUF_SIZE 8
// #define BYTE_SIZE 2
// #define ID_SIZE 8
// #define SEED_SIZE 4
#define SEND_SIZE 8
#define REC_SIZE 256
#define TREASURE_SIZE 1024
#define BYTE_SIZE 1
#define ID_SIZE 4
#define SEED_SIZE 2

#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int verbose = 1;

// hongkong:32400



void print_bytes(unsigned char *bytes, int byteslen);

int main(int argc, char *argv[]) {
	// printf("hey1");
	int sfd, s;
	// struct sockaddr_storage remote_addr;
	int nread;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	socklen_t remote_addr_len;

	int af;
	struct sockaddr_in ipv4addr_remote;
	struct sockaddr_in6 ipv6addr_remote;


	// socklen_t local_addr_len, remote_addr_len;


	// printf("hey1");

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;    /* Allow IPv4, IPv6, or both, depending on
				    what was specified on the command line. */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;  /* Any protocol */

	unsigned int server = htons(atoi(argv[1]));
	unsigned int port = htons(atoi(argv[2]));
	char * port_c = argv[2]; 
	// unsigned int level = 0;
	unsigned int level = atoi(argv[3]);
	printf("leve=%d\n",level);
	level = htons(atoi(argv[3]));
	printf("leve=%d\n",level);
	unsigned int seed = htons(atoi(argv[4]));

	// unsigned int id = htons(USERID);
	unsigned int id = htonl(USERID);

	unsigned char send_buf[SEND_SIZE];
	bzero(send_buf,SEND_SIZE);

	unsigned char rec_buf[REC_SIZE];
	bzero(rec_buf,REC_SIZE);

	// printf("hey1\n");
	memcpy(&send_buf[1], &level, BYTE_SIZE);
	// printf("hey2\n");
	memcpy(&send_buf[2], &id, ID_SIZE);
	// printf("%d\n",level);
	memcpy(&send_buf[6], &seed, SEED_SIZE);
	// printf("hey4\n");
	// 
	print_bytes(send_buf,SEND_SIZE);

	// pre-socket

	s = getaddrinfo(argv[1], argv[2], &hints, &result);
	// s = getaddrinfo(server, port_c, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	if (result == NULL) {   /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	// pre-socket
	sfd = socket(result->ai_family, result->ai_socktype,
				result->ai_protocol);
	
	size_t len = SEND_SIZE;

	// found in the struct addrinfo from getaddrinfo()
	af = result->ai_family;
	if (af == AF_INET) {
		ipv4addr_remote = *(struct sockaddr_in *)result->ai_addr;
	} else {
		ipv6addr_remote = *(struct sockaddr_in6 *)result->ai_addr;
	}

	// // updating port
	// ipv4addr_remote.sin_port = htons(port); // specific port
	// ipv6addr.sin6_port = htons(port); // specific port
	

	remote_addr_len = sizeof(struct sockaddr_in);
	struct sockaddr_in remote_addr;

	// if (sendto(sfd, send_buf, SEND_SIZE, 0,
	// 				(result->ai_addr),
	// 				// (struct sockaddr *) &(rp->ai_addr),
	// 				// (struct sockaddr *) &remote_addr,
	// 				remote_addr_len) < 0){
	// 		fprintf(stderr, "Error sending response\n");
	// 		exit(EXIT_FAILURE);
	// 		}


	// unsigned char nonce[4];
	// bzero(rec_buf,4);

	// unsigned short val = 0x0000000000000000;
	int i = 0;
	int n = 1;
	int op = 0;
	int par = 0;
	char treasure[TREASURE_SIZE];
	bzero(treasure,TREASURE_SIZE);
	// char* nonce[4];
	int nonce = 0;
	int start = 1;
	// char * n[1];

	while(n != 0){
		switch(op){
			case 0:
				if(start == 1){
					if (sendto(sfd, send_buf, SEND_SIZE, 0,
					(result->ai_addr),remote_addr_len) < 0){
						fprintf(stderr, "Error sending response\n");
						exit(EXIT_FAILURE);
					}
					start = 0;
					// printf("started\n");
				} else {
					if (sendto(sfd, &nonce, 4, 0,
						(result->ai_addr),
						remote_addr_len) < 0){
					fprintf(stderr, "Error sending response\n");
					exit(EXIT_FAILURE);
					}
				}
				break;
			case 1:
				// Communicate with the server using a new remote (server-side) port designated by the server.
				// // updating port
				// ipv4addr_remote.sin_port = htons(port); // specific port
				// ipv6addr.sin6_port = htons(port); // specific port
				if (af == AF_INET) {
					printf("here\n");
					ipv4addr_remote.sin_port = htons(par);
					if (sendto(sfd, &nonce, 4, 0, (struct sockaddr *) &ipv4addr_remote,
						remote_addr_len) < 0) {
						perror("sendto()");
					}
				} else {
					// ipv6addr.sin6_port = htons(par);
					ipv6addr_remote.sin6_port = htons(par);
					if (sendto(sfd, &nonce, 4, 0, (struct sockaddr *) &ipv4addr_remote,
						remote_addr_len) < 0) {
						perror("sendto()");
					}
				}
				break;
			case 2:
				//  Communicate with the server using a new local (client-side) port designated by the server.
				// // updating port
				// ipv4addr_remote.sin_port = htons(port); // specific port
				// ipv6addr.sin6_port = htons(port); // specific port
				break;
			case 3:
				// Same as op-code 0, but instead of sending a nonce that is provided by the server, 
				// derive the nonce by adding the remote ports associated with the m communications sent by the server.
				break;
			case 4:
				// Communicate with the server using a new address family, IPv4 or IPv6--whichever is not currently being used.
				break;
		}
		// i += nread;
		// remote_addr_len = sizeof(struct sockaddr_storage);
		nread = recvfrom(sfd, rec_buf, REC_SIZE, 0,
					(struct sockaddr *) &remote_addr, &remote_addr_len);
		if (nread == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}
		if (verbose)
			print_bytes(rec_buf,nread);
		
		memcpy(&n,&rec_buf[0], 1);
		if (verbose)
			printf("n=%d\n",n);
		
		// memcpy(&treasure[i],&rec_buf[1],n);
		// i += n;

		memcpy(&treasure[i],&rec_buf[1],n);
		i += n;
		// printf("i=%d\n",i);
		// if (verbose)
		// 	print_bytes(treasure,i);

		memcpy(&op,&rec_buf[n+1], 1);
		if (verbose)
			printf("op=%d\n",op);

		memcpy(&par,&rec_buf[n+2], 2);
		if (verbose)
			printf("par=%d\n",par);

		// bzero(buf, BUFSIZE);
		// printf("nonce=");
		memcpy(&nonce,&rec_buf[n+4], 4);
		nonce = htonl(ntohl(nonce) + 1);

		// sleep(1);
	}
	// printf("loop finished\n");
	// print_bytes(treasure,i);
	printf("%s\n",treasure);



}

void print_bytes(unsigned char *bytes, int byteslen) {
	int i, j, byteslen_adjusted;

	if (byteslen % 8) {
		byteslen_adjusted = ((byteslen / 8) + 1) * 8;
	} else {
		byteslen_adjusted = byteslen;
	}
	for (i = 0; i < byteslen_adjusted + 1; i++) {
		if (!(i % 8)) {
			if (i > 0) {
				for (j = i - 8; j < i; j++) {
					if (j >= byteslen_adjusted) {
						printf("  ");
					} else if (j >= byteslen) {
						printf("  ");
					} else if (bytes[j] >= '!' && bytes[j] <= '~') {
						printf(" %c", bytes[j]);
					} else {
						printf(" .");
					}
				}
			}
			if (i < byteslen_adjusted) {
				printf("\n%02X: ", i);
			}
		} else if (!(i % 4)) {
			printf(" ");
		}
		if (i >= byteslen_adjusted) {
			continue;
		} else if (i >= byteslen) {
			printf("   ");
		} else {
			printf("%02X ", bytes[i]);
		}
	}
	printf("\n");
}
