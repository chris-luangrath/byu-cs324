// #include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* Recommended max cache and object sizes */
#define MAXEVENTS 64
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

// static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:97.0) Gecko/20100101 Firefox/97.0";

int all_headers_received(char *);
int parse_request(char *, char *, char *, char *, char *, char *);
void test_parser();
void print_bytes(unsigned char *, int);
int open_sfd(char*, char*);
void handle_new_clients(int sfd);

struct client_info {
	int fd;
	char desc[1024];
};

// int main()
int main(int argc, char* argv[])
{
	// test_parser();
	// printf("%s\n", user_agent_hdr);

	// Create an epoll instance with epoll_create1().
	struct epoll_event event;
	struct epoll_event *events;

	struct client_info *listener;
	

	int sfd;
	int efd;

	if ((efd = epoll_create1(0)) < 0) {
		fprintf(stderr, "error creating epoll fd\n");
		exit(1);
	}

	// Call open_sfd() to get your listening socket.
	
	sfd = open_sfd(NULL,argv[1]);

	listener = malloc(sizeof(struct client_info));
	listener->fd = sfd;

	// Register your listen socket with the epoll instance that you created, for reading and for edge-triggered monitoring (i.e., EPOLLIN | EPOLLET).
	event.data.ptr = listener;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event) < 0) { // sfd??
		fprintf(stderr, "error adding event\n");
		exit(1);
	}
	// Create a while(1) loop that does the following:
	while(1){
		// Calls epoll_wait() loop with a timeout of 1 second.
		size_t n;
		n = epoll_wait(efd, events, MAXEVENTS, 1);
		
		// If the result was a timeout (i.e., return value from epoll_wait() is 0), 
		if(n == 0){
			// check if a global flag has been set by a signal handler and, if so, break out of the loop; otherwise, continue.
			// fprintf(stderr, "timeout in epoll_wait\n");
			// exit(1);
		} else if(n < 0){
		// If the result was an error (i.e., return value from epoll_wait() is less than 0), 
			// then handle the error appropriately (see the man page for epoll_wait() for more).
			if (errno == EBADF) {
				// epfd is not a valid file descriptor.
				fprintf(stderr, "epfd is not a valid file descriptor\n");
				exit(1);
			} else if(errno = EFAULT) {
				// The memory area pointed to by events is not accessible with write permissions.
				fprintf(stderr, "The memory area pointed to by events is not accessible with write permissions.\n");
				exit(1);
			} else if(errno = EINTR) {
				// The call was interrupted by a signal handler before either 
				// (1) any of the requested events occurred or (2) the timeout expired; see signal(7).
				fprintf(stderr, "The call was interrupted by a signal handler\n");
				exit(1);
			} else if(errno = EINVAL) {
				// epfd is not an epoll file descriptor, or maxevents is less than or equal to zero.
				perror("epfd is not an epoll file descriptor, or maxevents is less than or equal to zero\n");
				exit(1);
			}
		} 
		// If there was no error, 

		// you should loop through all the events and handle each appropriately. 
		// For now, just start with handling new clients. 
		// We will implement the handling of existing clients later. 

		struct client_info *active_client;
		// int i;
		for (int i = 0; i < n; i++) {
			// grab the data structure from the event, and cast it
			// (appropriately) to a struct client_info *.
			active_client = (struct client_info *)(events[i].data.ptr);
			printf("New event for %s\n", active_client->desc);

			if ((events[i].events & EPOLLERR) ||
					(events[i].events & EPOLLHUP) ||
					(events[i].events & EPOLLRDHUP)) {
				/* An error has occured on this fd */
				fprintf(stderr, "epoll error on %s\n", active_client->desc);
				close(active_client->fd);
				free(active_client);
				continue;
			}

			// If the event corresponds to the listening file descriptor, then call handle_new_clients().
			if (sfd == active_client->fd) {
				handle_new_clients(sfd);
			}

			// After the epoll_wait() while(1) loop, you should clean up any resources (e.g., freeing malloc()'d memory), and exit.

		}
		
	}

	return 0;
}

int all_headers_received(char *request) {
	if (strcmp(&request[strlen(request)-4],"\r\n\r\n") == 0){
		// printf("nice\n");
		return 1;
	} else {
		// printf("not nice\n");
		return 0;
	}
}

int parse_request(char *request, char *method,
char *hostname, char *port, char *path, char *headers) {
	if(!all_headers_received(request)){
		return 0;
	}
	char* ret;
	char* h = "Host: ";
	int newport = 1;

	//// Method
	// printf("method\n");
	int i = 0;
	int j = 0;
	
	ret = request;
	while(*ret != ' '){
		i++;
		ret++;
	}
	ret -= i;
	memcpy(method,ret,i);
	if(method == NULL){
		return 0;
	}
	// printf("method=%s\n",method);


	//// Host
	// printf("host\n");
	ret = strstr(request,h);
	
	while(*ret != ' '){
		// printf("skipping host=%c\n",*ret);
		ret++;
	}
	// sleep(1);
	ret++;
	// printf("host skipped=%s\n",ret);

	i = 0;
	while(*ret != '\r' && *ret != ':'){
		// printf("c=%c\n",*ret);
		// printf("d=%d\n",*ret);
		ret++;
		i++;
		// sleep(1);
	}
	if(*ret == ':'){
		ret++;
		j = 0;
		while(*ret != '\r'){
			j++;
			ret++;
		}
		ret -= j;
		memcpy(port,ret,j);
		ret--;
	} else {
		newport = 0;
		strcpy(port,"80");
		// strcpy(port,defaultport);
	}
	// printf("port=%s\n",port);

	ret -= i;
	memcpy(hostname,ret,i);
	if(hostname == NULL){
		return 0;
	}
	// printf("hostname=%s\n",hostname);


	//// Path
	// printf("path\n");
	ret = strstr(request,hostname);
	ret += strlen(hostname);
	// ret++;
	if(newport){
		ret += strlen(port) + 1;
	}
	// printf("to path=%s\n",ret);
	i = 0;
	while(*ret != ' '){
		// printf("pat=%c\n",*ret);
		i++;
		ret++;
	}
	ret -= i;
	// printf("path=%s\n",path);
	memcpy(path,ret,i);
	if(path == NULL){
		return 0;
	}
	// printf("path=%s\n",path);
	
	//// Headers
	// printf("headers\n");
	ret = strstr(request,"\r\n");
	ret += 2;
	memcpy(headers,ret,strlen(ret));
	if(headers == NULL){
		return 0;
	}

	// printf("headers=%s\n",headers);

	return 1;
}

int open_sfd(char* hostname, char* port) {
	struct addrinfo hints;
	struct addrinfo *result;
	int optval = 1;
	int sfd = 0;


	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;    /* Allow IPv4, IPv6, or both, depending on
				    what was specified on the command line. */
	hints.ai_socktype = SOCK_STREAM; /* socket stream */
	hints.ai_flags = AI_PASSIVE;
	// hints.ai_flags = 0;
	hints.ai_protocol = 0;  /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	int s = getaddrinfo(hostname, port, &hints, &result);
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

	setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	
	if (bind(sfd, result->ai_addr, result->ai_addrlen) < 0) {	
		perror("Could not bind");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(result);
	listen(sfd,100);
	// printf("sfd=%d\n",sfd);
	// printf("sfd=%d\n",sfd);
	return sfd;
}

// int handle_new_clients(int sfd){
void handle_new_clients(int sfd){
	// 	Loop to accept() any and all client connections. 
	// For each new file descriptor (i.e., corresponding to a new client) returned, 
	while(1){
		int connfd;
		// int clientsfd, connfd;
		int efd;
		struct epoll_event event;
		// struct epoll_event *events;
		struct sockaddr_storage clientaddr;
		socklen_t clientlen;
		clientlen = sizeof(struct sockaddr_storage); 
		
		connfd = accept(sfd, (struct sockaddr *)&clientaddr, &clientlen);
		// connfd = accept(active_client->fd, (struct sockaddr *)&clientaddr, &clientlen);

		if (connfd < 0) {
			// You should only break out of your loop and stop calling accept() when it returns a value less than 0, in which case:
			// If errno is set to EAGAIN or EWOULDBLOCK, then that is an indicator that there are no more clients currently pending;
			if (errno == EWOULDBLOCK ||
					errno == EAGAIN) {
				// no more clients ready to accept
				break;
			} else {
				// If errno is anything else, this is an error. It actually be best to have your proxy exit at this point.
				perror("accept");
				exit(EXIT_FAILURE);
			}
		}

		// configure it to use non-blocking I/O (see the man page for fcntl() for how to do this), 
		// set client file descriptor non-blocking
		if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
			fprintf(stderr, "error setting socket option\n");
			exit(1);
		}

		// and register each returned client socket with the epoll instance that you created for reading, 
		// using edge-triggered monitoring (i.e., EPOLLIN | EPOLLET). 

		if ((efd = epoll_create1(0)) < 0) {
			fprintf(stderr, "error creating epoll fd\n");
			exit(1);
		}

		struct client_info *listener;
		listener = malloc(sizeof(struct client_info));
		listener->fd = sfd;

		// register the listening file descriptor for incoming events using
		// edge-triggered monitoring
		event.data.ptr = listener;
		event.events = EPOLLIN | EPOLLET;
		if (epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event) < 0) {
			fprintf(stderr, "error adding event\n");
			exit(1);
		}

		// Have your proxy print the newly created file descriptor associated with any new clients. 
		// You can remove this later, but it will be good for you to see now that they are being created.
		printf("new file descriptor: %d\n",efd);

		/* Buffer where events are returned */
		// events = calloc(MAXEVENTS, sizeof(struct epoll_event));

		// You will need to pass your epoll file descriptor as an argument, 
		// so you can register the new file descriptor with the epoll instance.


		// if ((clientsfd = accept(sfd, NULL, NULL)) < 0) {
		// 	perror("Could not accept");	
		// 	exit(EXIT_FAILURE);
		// } else {
		// 	// configure it to use non-blocking I/O (see the man page for fcntl() for how to do this)
		// 	fcntl(clientsfd, O_NONBLOCK);
		// }
		// 	// handle_client(clientsfd);

	}

	
	

	
}

void test_parser() {
	int i;
	char method[16], hostname[64], port[8], path[64], headers[1024];

       	char *reqs[] = {
		"GET http://www.example.com/index.html HTTP/1.0\r\n"
		"Host: www.example.com\r\n"
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n\r\n",

		"GET http://www.example.com:8080/index.html?foo=1&bar=2 HTTP/1.0\r\n"
		"Host: www.example.com:8080\r\n"
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n\r\n",

		"GET http://localhost:1234/home.html HTTP/1.0\r\n"
		"Host: localhost:1234\r\n"
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n\r\n",

		"GET http://www.example.com:8080/index.html HTTP/1.0\r\n",

		NULL
	};
	
	for (i = 0; reqs[i] != NULL; i++) {
		printf("Testing %s\n", reqs[i]);
		if (parse_request(reqs[i], method, hostname, port, path, headers)) {
			printf("METHOD: %s\n", method);
			printf("HOSTNAME: %s\n", hostname);
			printf("PORT: %s\n", port);
			printf("HEADERS: %s\n", headers);
		} else {
			printf("REQUEST INCOMPLETE\n");
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
