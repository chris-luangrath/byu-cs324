// Replace PUT_USERID_HERE with your actual BYU CS user id, which you can find
// by running `id -u` on a CS lab machine.
#define USERID 1823700709
// #define BUF_SIZE 8
// #define BYTE_SIZE 2
// #define ID_SIZE 8
// #define SEED_SIZE 4
#define SEND_SIZE 8
#define REC_SIZE 256
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
	int sfd, s, j;
	struct sockaddr_storage remote_addr;
	int nread;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	socklen_t remote_addr_len;
	// socklen_t local_addr_len, remote_addr_len;



	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = af;    /* Allow IPv4, IPv6, or both, depending on
				    what was specified on the command line. */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;  /* Any protocol */

	int af = AF_INET;

	int server = atoi(argv[1]);
	int port = atoi(argv[2]);
	char * port_c = argv[2]; 
	int level = atoi(argv[3]);
	int seed = atoi(argv[4]);

	int id = USERID;

	unsigned char send_buf[SEND_SIZE];
	bzero(send_buf,SEND_SIZE);

	unsigned char rec_buf[REC_SIZE];
	bzero(rec_buf,REC_SIZE);

	memcpy(&send_buf[1], &level, BYTE_SIZE);
	memcpy(&send_buf[2], &id, ID_SIZE);
	memcpy(&send_buf[6], &seed, SEED_SIZE);
	// 
	// print_bytes(buf,BUF_SIZE);

	// pre-socket

	s = getaddrinfo(&server, &port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	// pre-socket
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;  /* Success */

		close(sfd);
	}

	if (rp == NULL) {   /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	// sending/recieving message
	size_t len;
	len = strlen(seed) + 1;
	if (write(sfd, seed, len) != len) {
		fprintf(stderr, "partial/failed write\n");
		exit(EXIT_FAILURE);
	}

	remote_addr_len = sizeof(struct sockaddr_storage);
	// nread = read(sfd, buf, 2);
	nread = recvfrom(sfd, rec_buf, REC_SIZE, 0,
				(struct sockaddr *) &remote_addr, &remote_addr_len);
	if (nread == -1) {
		perror("read");
		exit(EXIT_FAILURE);
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
