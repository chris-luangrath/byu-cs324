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
#define NONCE_SIZE 1024
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

int verbose = 0;

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
	unsigned int level = htons(atoi(argv[3]));
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
	// printf("hey3\n");
	memcpy(&send_buf[6], &seed, SEED_SIZE);
	// printf("hey4\n");
	// 
	// print_bytes(send_buf,SEND_SIZE);

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
	

	remote_addr_len = sizeof(struct sockaddr_in);
	if (sendto(sfd, send_buf, SEND_SIZE, 0,
					(result->ai_addr),
					// (struct sockaddr *) &(rp->ai_addr),
					// (struct sockaddr *) &remote_addr,
					remote_addr_len) < 0){
			fprintf(stderr, "Error sending response\n");
			exit(EXIT_FAILURE);
			}

	// remote_addr_len = sizeof(struct sockaddr_storage);
	struct sockaddr_in remote_addr;
	nread = recvfrom(sfd, rec_buf, REC_SIZE, 0,
				(struct sockaddr *) &remote_addr, &remote_addr_len);
	if (nread == -1) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	print_bytes(rec_buf,nread);


	// unsigned char nonce[4];
	// bzero(rec_buf,4);

	// unsigned short val = 0x0000000000000000;
	int i = 0;
	int n = 0;
	int op = 1;
	int par = 2;
	char* full_nonce[NONCE_SIZE];
	// char* nonce[4];
	int nonce = 0;
	// char * n[1];
	// printf("hey1\n");
	// printf("hey2\n");
	memcpy(&n,&rec_buf[0], 1);
	printf("n=%d\n",n);
	// n += 1;

	memcpy(&op,&rec_buf[n+1], 1);
	printf("op=%d\n",op);

	memcpy(&par,&rec_buf[n+2], 2);
	printf("par=%d\n",par);

	// bzero(buf, BUFSIZE);
	memcpy(&nonce,&rec_buf[n+4], 4);
	memcpy(&full_nonce[i],&nonce, 4);
	i += 4;
	// print_bytes((unsigned char *)&nonce,4);
	nonce = ntohl(nonce);
	// print_bytes((unsigned char *)&nonce,4);
	nonce += 1;
	// print_bytes((unsigned char *)&nonce,4);
	nonce = htonl(nonce);
	// print_bytes((unsigned char *)&nonce,4);
	// nonce = (char*)ntohs(nonce);
	// memcpy(&nonce,&ntohs(nonce), 4);
	// printf("hey3\n");
	printf("nonce=%x\n", nonce);
	// printf("nonce=%x\n", ntohs(nonce));
	// for (i = 0; i < 4; i++) {
	// 	printf("%x ", nonce[i]);
	// }
	while(n != 0){
		switch(op){
			case 0:
				if (sendto(sfd, &nonce, 4, 0,
						(result->ai_addr),
						remote_addr_len) < 0){
					fprintf(stderr, "Error sending response\n");
					exit(EXIT_FAILURE);
				}
				nread = recvfrom(sfd, rec_buf, REC_SIZE, 0,
					(struct sockaddr *) &remote_addr, &remote_addr_len);
				if (nread == -1) {
					perror("read");
					exit(EXIT_FAILURE);
				}
				// memcpy(&full_nonce[i],&rec_buf[0], nread);
				memcpy(&full_nonce[i],&rec_buf[0], nread);
				i += nread;
				print_bytes(rec_buf,nread);
				sleep(1);
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;
		}
	}



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
