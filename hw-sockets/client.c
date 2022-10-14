#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 500
#define IN_SIZE 4096
#define CHUNK_SIZE 512
#define MAX_READ 16384

int main(int argc, char *argv[]) {
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s, j;
	size_t len;
	ssize_t nread;
	char buf[BUF_SIZE];
	int hostindex;
	int af;

	if (argc < 3 ||
		((strcmp(argv[1], "-4") == 0 || strcmp(argv[1], "-6") == 0) &&
			argc < 4)) {
		fprintf(stderr, "Usage: %s [ -4 | -6 ] host port msg...\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Use only IPv4 (AF_INET) if -4 is specified;
	 * Use only IPv6 (AF_INET6) if -6 is specified;
	 * Try both if neither is specified. */
	af = AF_UNSPEC;
	if (strcmp(argv[1], "-4") == 0 ||
			strcmp(argv[1], "-6") == 0) {
		if (strcmp(argv[1], "-6") == 0) {
			af = AF_INET6;
		} else { // (strcmp(argv[1], "-4") == 0) {
			af = AF_INET;
		}
		hostindex = 2;
	} else {
		hostindex = 1;
	}

	/* Obtain address(es) matching host/port */

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = af;    /* Allow IPv4, IPv6, or both, depending on
				    what was specified on the command line. */
	// hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;  /* Any protocol */

	/* SECTION A - pre-socket setup; getaddrinfo() */

	s = getaddrinfo(argv[hostindex], argv[hostindex + 1], &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
	   Try each address until we successfully connect(2).
	   If socket(2) (or connect(2)) fails, we (close the socket
	   and) try the next address. */

	/* SECTION B - pre-socket setup; getaddrinfo() */

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
		// sfd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
		
		if (sfd == -1)
			continue;

		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;
			// if (bind(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
				// break;  /* Success */

		close(sfd);
	}

	if (rp == NULL) {   /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);   /* No longer needed */

	// Part 3
	char* buffer[IN_SIZE];
	char* buffer2[IN_SIZE];
	char* buffer3[MAX_READ];
	char* p = buffer;
	int readed;
	int wrote;
	int totalRead = 0;
	int totalWrote = 0;
	int oldRead = 0;
	int oldWrote = 0;
	int amountToSend;
	while((readed = fread(p,1,CHUNK_SIZE,stdin)) > 0 && totalRead < IN_SIZE){
		if(readed < 0){
			fprintf(stderr, "partial/failed read\n");
			exit(EXIT_FAILURE);
		}
		p += readed;
		oldRead = totalRead;
		totalRead += readed;
		// read = fread(buffer,sizeof(char),512,stdin);	
	}
	p = buffer;
	// totalWrote = totalRead;
	while (totalWrote < totalRead) {
		amountToSend = totalRead - totalWrote > CHUNK_SIZE ? CHUNK_SIZE : totalRead - totalWrote; 

		if((wrote = write(sfd, p, amountToSend)) < 0){
			fprintf(stderr, "partial/failed write\n");
			exit(EXIT_FAILURE);
		}
		p += wrote;
		oldWrote = totalWrote;
		totalWrote += wrote;
		
	}
	
	if(totalRead != totalWrote){
		fprintf(stderr, "did not read/write all data\n");
		fprintf(stderr, "total_Read: %d\n",totalRead);
		fprintf(stderr, "total_Wrote: %d\n",totalWrote);
		printf("oldWrote: %d\n",oldWrote);
		printf("wrote: %d\n",wrote);
		printf("oldread: %d\n",oldRead);
		printf("read: %d\n",readed);
		exit(EXIT_FAILURE);
	}

	p = buffer3;
	totalRead = 0;
	while((readed = read(sfd, p,CHUNK_SIZE)) > 0 && totalRead < MAX_READ){
		p += readed;
		totalRead += readed;
	}

	p = buffer3;
	totalWrote = 0;
	while (totalWrote < totalRead) {
		amountToSend = totalRead - totalWrote; 
		// amountToSend = totalRead - totalWrote > CHUNK_SIZE ? CHUNK_SIZE : totalRead - totalWrote; 
		if((wrote = fwrite(p,1,amountToSend,stdout)) < 0){
			fprintf(stderr, "partial/failed write\n");
			exit(EXIT_FAILURE);
		}
		p += wrote;
		totalWrote += wrote;
	}
	fprintf(stderr, "total_Read: %d\n",totalRead);
	fprintf(stderr, "total_Wrote: %d\n",totalWrote);
	if(totalRead != totalWrote){
		fprintf(stderr, "did not read/write all data\n");
		fprintf(stderr, "total_Read: %d\n",totalRead);
		fprintf(stderr, "total_Wrote: %d\n",totalWrote);
		printf("oldWrote: %d\n",oldWrote);
		printf("wrote: %d\n",wrote);
		printf("oldread: %d\n",oldRead);
		printf("read: %d\n",readed);
		exit(EXIT_FAILURE);
	}
	printf("\n");
	

	/* SECTION C - interact with server; send, receive, print messages */

	/* Send remaining command-line arguments as separate
	   datagrams, and read responses from server */

	// sleep(30);
	for (j = hostindex + 2; j < argc; j++) {
		len = strlen(argv[j]) + 1;
		/* +1 for terminating null byte */

		if (len + 1 > BUF_SIZE) {
			fprintf(stderr,
					"Ignoring long message in argument %d\n", j);
			continue;
		}

		if (write(sfd, argv[j], len) != len) {
			fprintf(stderr, "partial/failed write\n");
			exit(EXIT_FAILURE);
		}

		// nread = read(sfd, buf, BUF_SIZE);
		if (nread == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		// printf("Received %zd bytes: %s\n", nread, buf);

	}

	exit(EXIT_SUCCESS);
}
