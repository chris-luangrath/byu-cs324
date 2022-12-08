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

#define READ_REQUEST 0
#define SEND_REQUEST 1
#define READ_RESPONSE 2
#define SEND_RESPONSE 3

static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:97.0) Gecko/20100101 Firefox/97.0";

int verbose = 1;
// if(verbose)

// struct client_info
// {
// 	int fd;
// 	char desc[1024];
// };

// typedef struct {
// // 	the socket corresponding to the requesting client
// 	int soc_cli;
// // the socket corresponding to the connection to the Web server
// 	int soc_ser;
// // the current state of the request (see Client Request States).
// 	int state;
// // the buffer(s) to read into and write from
// 	char rec_buf[MAX_OBJECT_SIZE];
// 	// unsigned char rec_buf[MAX_OBJECT_SIZE]; //
// // the total number of bytes read from the client
// 	int bytes_read_cli;
// // the total number of bytes to write to the server
// 	int bytes_to_write_ser;
// // the total number of bytes written to the server
// 	int bytes_written_ser;
// // the total number of bytes read from the server
// 	int bytes_read_ser;
// // the total number of bytes written to the client
// 	int bytes_written_cli;
// } request_info;

struct request_info {
	int fd;
	char desc[1024];

	// int soc_lis; // is this correct

// 	the socket corresponding to the requesting client
	int soc_cli;
// the socket corresponding to the connection to the Web server
	int soc_ser;
// the current state of the request (see Client Request States).
	int state;
// the buffer(s) to read into and write from
	char rec_buf[MAX_OBJECT_SIZE];
	// unsigned char rec_buf[MAX_OBJECT_SIZE]; //
// the total number of bytes read from the client
	int bytes_read_cli;
// the total number of bytes to write to the server
	int bytes_to_write_ser;
// the total number of bytes written to the server
	int bytes_written_ser;
// the total number of bytes read from the server
	int bytes_read_ser;
// the total number of bytes written to the client
	int bytes_written_cli;
};

int all_headers_received(char *);
int parse_request(char *, char *, char *, char *, char *, char *);
void test_parser();
void print_bytes(unsigned char *, int);
int open_sfd(char *, char *);
void handle_new_clients(int sfd);
void handle_client(struct request_info*);




int efd;
struct epoll_event event;
struct epoll_event *events;

// int main()
int main(int argc, char *argv[]) {
	// test_parser();
	// printf("%s\n", user_agent_hdr);

	// Create an epoll instance with epoll_create1().
	
	
	// struct client_info *new_client;
	// struct client_info *active_client;

	int sfd;

	if ((efd = epoll_create1(0)) < 0) {
		fprintf(stderr, "error creating epoll fd\n");
		exit(1);
	}

	// Call open_sfd() to get your listening socket.

	sfd = open_sfd(NULL, argv[1]);

	struct request_info *listener;
	// struct client_info *listener;
	listener = malloc(sizeof(struct request_info));
	// listener = malloc(sizeof(struct client_info));
	// listener->fd = sfd;
	// listener->soc_lis = sfd;
	listener->soc_cli = sfd;
	sprintf(listener->desc, "Listening socket");
	event.data.ptr = listener;
	event.events = EPOLLIN | EPOLLET;

	// Register your listen socket with the epoll instance that you created, for reading and for edge-triggered monitoring (i.e., EPOLLIN | EPOLLET).
	// event.data.ptr = listener;
	// event.events = EPOLLIN | EPOLLET;
	
	if (epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event) < 0) { // sfd??
		fprintf(stderr, "error adding event\n");
		exit(1);
	}
	// Create a while(1) loop that does the following:
	while (1) {
		// printf("main_while_loop\n");
		// Calls epoll_wait() loop with a timeout of 1 second.
		size_t n;
		events = calloc(MAXEVENTS, sizeof(struct epoll_event));
		n = epoll_wait(efd, events, MAXEVENTS, 1);
		

		// If the result was a timeout (i.e., return value from epoll_wait() is 0),
		if (n == 0) {
			// check if a global flag has been set by a signal handler and, if so, break out of the loop; otherwise, continue.
			// printf("timeout\n");
			// fprintf(stderr, "timeout in epoll_wait\n");
			// exit(1);
		}
		else if (n < 0) {
			// If the result was an error (i.e., return value from epoll_wait() is less than 0),
			// then handle the error appropriately (see the man page for epoll_wait() for more).
			if (errno == EBADF) {
				// epfd is not a valid file descriptor.
				fprintf(stderr, "epfd is not a valid file descriptor\n");
				exit(1);
			}
			else if (errno = EFAULT) {
				// The memory area pointed to by events is not accessible with write permissions.
				fprintf(stderr, "The memory area pointed to by events is not accessible with write permissions.\n");
				exit(1);
			}
			else if (errno = EINTR) {
				// The call was interrupted by a signal handler before either
				// (1) any of the requested events occurred or (2) the timeout expired; see signal(7).
				fprintf(stderr, "The call was interrupted by a signal handler\n");
				exit(1);
			}
			else if (errno = EINVAL) {
				// epfd is not an epoll file descriptor, or maxevents is less than or equal to zero.
				perror("epfd is not an epoll file descriptor, or maxevents is less than or equal to zero\n");
				exit(1);
			}
		}
		// If there was no error,

		// you should loop through all the events and handle each appropriately.
		// For now, just start with handling new clients.
		// We will implement the handling of existing clients later.

		struct request_info *active_client;
		// struct client_info *active_client;
		// int i;
		for (int i = 0; i < n; i++) {
			// printf("look here--------------------------------------------------\n");
	
			// grab the data structure from the event, and cast it
			// (appropriately) to a struct client_info *.
			// active_client = (struct client_info *)(events[i].data.ptr);
			active_client = (struct request_info *)(events[i].data.ptr);
			printf("New event for %s\n", active_client->desc);
			// uh...... we don't even use client_info for our request_info, what's the point of this then?

			if ((events[i].events & EPOLLERR) ||
				(events[i].events & EPOLLHUP) ||
				(events[i].events & EPOLLRDHUP)) {
				/* An error has occured on this fd */
				fprintf(stderr, "epoll error on %s\n", active_client->desc);
				// close(active_client->fd);
				close(active_client->soc_cli);
				free(active_client);
				continue;
			}

			// If the event corresponds to the listening file descriptor, then call handle_new_clients().
			// printf("look here--------------------------------------------------\n");
			// if (sfd == active_client->fd) {
			// if (listener->fd == active_client->fd) { // can't tell if it already exists----------------------------------------------------------------------------------------
			if (listener->soc_cli == active_client->soc_cli) { // can't tell if it already exists----------------------------------------------------------------------------------------
				handle_new_clients(active_client->soc_cli);
			} else {
				printf("-active client fd: %d\n", active_client->soc_cli);
				// printf("ITS HERE AND IT WORKS\n;"); // it does 
				handle_client((struct request_info*) active_client); // casting so it stops yelling at me 
			}

			// After the epoll_wait() while(1) loop, you should clean up any resources (e.g., freeing malloc()'d memory), and exit.
		}
	}

	return 0;
}

int all_headers_received(char *request) {
	if (strcmp(&request[strlen(request) - 4], "\r\n\r\n") == 0) {
		// printf("nice\n");
		return 1;
	}
	else
	{
		// printf("not nice\n");
		return 0;
	}
}

int parse_request(char *request, char *method,
				  char *hostname, char *port, char *path, char *headers) {
	if (!all_headers_received(request)) {
		return 0;
	}
	char *ret;
	char *h = "Host: ";
	int newport = 1;

	//// Method
	// printf("method\n");
	int i = 0;
	int j = 0;

	ret = request;
	while (*ret != ' ') {
		i++;
		ret++;
	}
	ret -= i;
	memcpy(method, ret, i);
	if (method == NULL) {
		return 0;
	}
	// printf("method=%s\n",method);

	//// Host
	// printf("host\n");
	ret = strstr(request, h);

	while (*ret != ' ') {
		// printf("skipping host=%c\n",*ret);
		ret++;
	}
	// sleep(1);
	ret++;
	// printf("host skipped=%s\n",ret);

	i = 0;
	while (*ret != '\r' && *ret != ':') {
		// printf("c=%c\n",*ret);
		// printf("d=%d\n",*ret);
		ret++;
		i++;
		// sleep(1);
	}
	if (*ret == ':') {
		ret++;
		j = 0;
		while (*ret != '\r') {
			j++;
			ret++;
		}
		ret -= j;
		memcpy(port, ret, j);
		ret--;
	}
	else
	{
		newport = 0;
		strcpy(port, "80");
		// strcpy(port,defaultport);
	}
	// printf("port=%s\n",port);

	ret -= i;
	memcpy(hostname, ret, i);
	if (hostname == NULL) {
		return 0;
	}
	// printf("hostname=%s\n",hostname);

	//// Path
	// printf("path\n");
	ret = strstr(request, hostname);
	ret += strlen(hostname);
	// ret++;
	if (newport) {
		ret += strlen(port) + 1;
	}
	// printf("to path=%s\n",ret);
	i = 0;
	while (*ret != ' ') {
		// printf("pat=%c\n",*ret);
		i++;
		ret++;
	}
	ret -= i;
	// printf("path=%s\n",path);
	memcpy(path, ret, i);
	if (path == NULL) {
		return 0;
	}
	// printf("path=%s\n",path);

	//// Headers
	// printf("headers\n");
	ret = strstr(request, "\r\n");
	ret += 2;
	memcpy(headers, ret, strlen(ret));
	if (headers == NULL) {
		return 0;
	}

	// printf("headers=%s\n",headers);

	return 1;
}

int open_sfd(char *hostname, char *port) {
	struct addrinfo hints;
	struct addrinfo *result;
	int optval = 1;
	int sfd = 0;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;		 /* Allow IPv4, IPv6, or both, depending on
					   what was specified on the command line. */
	hints.ai_socktype = SOCK_STREAM; /* socket stream */
	hints.ai_flags = AI_PASSIVE;
	// hints.ai_flags = 0;
	hints.ai_protocol = 0; /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	int s = getaddrinfo(hostname, port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	if (result == NULL) { /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	// pre-socket
	sfd = socket(result->ai_family, result->ai_socktype,
				 result->ai_protocol);

	setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

	if (fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
			fprintf(stderr, "error setting socket option\n");
			exit(1);
		}

	if (bind(sfd, result->ai_addr, result->ai_addrlen) < 0) {
		perror("Could not bind");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(result);
	listen(sfd, 100);
	// printf("sfd=%d\n",sfd);
	// printf("sfd=%d\n",sfd);
	return sfd;
}

// int handle_new_clients(int sfd){
void handle_new_clients(int sfd) {
	if(verbose)
		printf("handle_new_clients\n");
	// 	Loop to accept() any and all client connections.
	// For each new file descriptor (i.e., corresponding to a new client) returned,
	
	int connfd;
	struct sockaddr_storage clientaddr;
	socklen_t clientlen;
	clientlen = sizeof(struct sockaddr_storage);
	// printf("look here--------------------------------------------------\n");
	while (1) {
		if(verbose)
			printf("inside while\n");
		// int clientsfd, connfd;
		
		// struct epoll_event *events;
		if(verbose)
			printf("before accept\n");
		connfd = accept(sfd, (struct sockaddr *)&clientaddr, &clientlen);
		if(verbose)
			printf("after accept\n");
		// connfd = accept(active_client->fd, (struct sockaddr *)&clientaddr, &clientlen);

		if (connfd < 0) {
			// You should only break out of your loop and stop calling accept() when it returns a value less than 0, in which case:
			// If errno is set to EAGAIN or EWOULDBLOCK, then that is an indicator that there are no more clients currently pending;
			if (errno == EWOULDBLOCK ||
				errno == EAGAIN) {
					if(verbose)
						perror("timeout or block or something");
						// printf("before accept\n");
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
		// struct client_info *client;
		struct request_info *client;
		struct epoll_event event;
		// client = malloc(sizeof(struct client_info));
		client = malloc(sizeof(struct request_info));
		client->soc_cli = connfd;
		event.data.ptr = client;
		event.events = EPOLLIN | EPOLLET;
		sprintf(client->desc, "returned client socket");
		if (epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event) < 0) {
			fprintf(stderr, "error adding event\n");
			exit(1);
		}




		// Have your proxy print the newly created file descriptor associated with any new clients.
		// You can remove this later, but it will be good for you to see now that they are being created.4
		if(verbose)
			printf("new file descriptor: %d\n", efd);

		/* Buffer where events are returned */
		// events = calloc(MAXEVENTS, sizeof(struct epoll_event));


		// You will need to pass your epoll file descriptor as an argument,
		// so you can register the new file descriptor with the epoll instance.

		// struct request_info request;
		struct request_info *request;
		// request = malloc(sizeof(struct request_info));
		request = malloc(sizeof(struct request_info));
		request->bytes_read_cli = 0;
		request->bytes_read_ser = 0;
		request->bytes_to_write_ser = 0;
		request->bytes_written_cli = 0;
		request->bytes_written_ser = 0;
		request->state = READ_REQUEST;
		bzero(request->rec_buf, MAX_OBJECT_SIZE);

		// DO I HAVE THESE CORRECT
		request->soc_cli = connfd;
		// request->soc_ser = sfd;

		handle_client(request);
	}
}

void handle_client(struct request_info* request) {
	if(verbose)
		printf("entered handle_client--------------------------------------------------\n");
	// that takes a pointer to a client request,
	// determines what state it is in,
	// and performs the actions associated with that state
	// (i.e., picks up where it left off. See Client Request States for more information.
	// For now, just implement the READ_REQUEST state.

	
	
	// unsigned char rec_buf[MAX_OBJECT_SIZE];
	
	socklen_t remote_addr_len;
	struct sockaddr_storage remote_addr;
	remote_addr_len = sizeof(struct sockaddr_storage);
	// struct epoll_event event;
	// event.data.ptr = listener;
	// event.events = EPOLLIN | EPOLLET;
	printf("request state: %d\n",request->state);

	if (request->state == READ_REQUEST) {
		if(verbose)
			printf("--read request--\n");
		// This is the start state for every new client request.
		// You should initialize every new client request to be in this state.
		int nread = 0;

		// In this state, read from the client socket in a loop until one of the following happens:
		while (1) {
			// if (all_headers_received(recieved_request)) {
			// 		headers_recieved = 1;
			// 		printf("done receiving\n");
			// 	}
			char* p = request->rec_buf;
			p += request->bytes_read_cli;

			nread = recvfrom(request->soc_cli, p, MAX_OBJECT_SIZE, 0,
								(struct sockaddr *)&remote_addr, &remote_addr_len);
			if(nread > 0){
				request->bytes_read_cli += nread;
				p += nread;
			}
			if(verbose)
				printf("just read %d bytes\n",nread);
			if (all_headers_received((request->rec_buf))) { 
				// you have read the entire HTTP request from the client. If this is the case: ----------------------------------------------------------
				char method[16], hostname[64], port[8], path[64], headers[1024];
				memset(method, 0, 16);
				memset(hostname, 0, 64);
				memset(port, 0, 8);
				memset(path, 0, 64);
				memset(headers, 0, 1024);

				if (parse_request(request->rec_buf, method, hostname, port, path, headers)) {
					printf("METHOD: %s\n", method);
					printf("HOSTNAME: %s\n", hostname);
					printf("PORT: %s\n", port);
					printf("PATH: %s\n", path); // I ADDED THIS ONE. IT WASNT HERE BEFORE
					printf("HEADERS: %s\n", headers);
					printf("%s\n", user_agent_hdr);
				} else {
					printf("REQUEST INCOMPLETE\n");
					exit(1);
				}


				// parse the client request and create the request that you will send to the server. ----------------------------------------------------------
				char *connection = "close";
				char *proxyconnection = "close";

				// char newrequest[MAX_OBJECT_SIZE];
				char *newrequest = malloc(MAX_OBJECT_SIZE);
				char *p = newrequest;
				bzero(newrequest, MAX_OBJECT_SIZE);
				char buf[MAX_OBJECT_SIZE];
				bzero(buf, MAX_OBJECT_SIZE);
				sprintf(buf, "%s %s HTTP/1.0\r\n", method, path);
				// printf("method=%s\n",method);
				// printf("path=%s\n",path);
				// printf("buf=%s\n",buf);

				memcpy(p, &buf, strlen(buf));
				p += strlen(buf);

				bzero(buf, MAX_OBJECT_SIZE);
				sprintf(buf, "Host: %s:%s\r\n", hostname, port);
				memcpy(p, &buf, strlen(buf));
				p += strlen(buf);

				bzero(buf, MAX_OBJECT_SIZE);
				sprintf(buf, "%s\r\n", user_agent_hdr);
				memcpy(p, &buf, strlen(buf));
				p += strlen(buf);

				bzero(buf, MAX_OBJECT_SIZE);
				sprintf(buf, "Connection: %s\r\nProxy-Connection: %s\r\n\r\n", connection, proxyconnection);
				memcpy(p, &buf, strlen(buf));
				p += strlen(buf);

				// printf("HEY IT'S HERE ----------------------\n");
				// if(verbose)
				// printf("%s\n",newrequest);
				if(verbose)
					printf("new request:\n%s\n", newrequest);


				// create a new socket and connect to the HTTP server. ----------------------------------------------------------
				struct addrinfo hints;
				// struct addrinfo *result;
				struct addrinfo *result, *rp;
				memset(&hints, 0, sizeof(struct addrinfo));
				hints.ai_family = AF_INET;    /* Allow IPv4, IPv6, or both, depending on
								what was specified on the command line. */
				hints.ai_socktype = SOCK_STREAM; /* socket stream*/
				hints.ai_flags = 0;
				hints.ai_protocol = 0;  /* Any protocol */
				hints.ai_canonname = NULL;
				hints.ai_addr = NULL;
				hints.ai_next = NULL;

				// printf("1--------------------------------------------\n");
				int s;
				int serversfd;
				s = getaddrinfo(hostname,port,&hints,&result);
				if (s != 0) {
					fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
					exit(EXIT_FAILURE);
				}

				for (rp = result; rp != NULL; rp = rp->ai_next) {
					serversfd = socket(rp->ai_family, rp->ai_socktype,
							rp->ai_protocol);
					if (serversfd == -1)
						continue;

					if (connect(serversfd, rp->ai_addr, rp->ai_addrlen) != -1){
						// printf("successfully connected\n");
						break;  /* Success */
					}
					if(verbose)
						printf("success?\n");
					close(serversfd);
				}
				// printf("2--------------------------------------------\n");

				if (rp == NULL) {   /* No address succeeded */
					fprintf(stderr, "Could not connect\n");
					exit(EXIT_FAILURE);
				}

				freeaddrinfo(result);

				// configure the new socket as non-blocking. ----------------------------------------------------------
				if (fcntl(serversfd, F_SETFL, fcntl(serversfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
					fprintf(stderr, "error setting socket option\n");
					exit(1);
				}

				// change state to SEND_REQUEST.
				request->state = SEND_REQUEST;
				request->soc_ser = serversfd;
				// request->bytes_to_write_ser = request->bytes_read_cli;
				// bzero(request->rec_buf, MAX_OBJECT_SIZE);
				// request->rec_buf = *newrequest;
				sprintf(request->rec_buf,"%s",newrequest);
				// memcpy(request->rec_buf, newrequest, strlen(newrequest));
				request->bytes_to_write_ser = strlen(newrequest);
				// request->bytes_to_write_ser = strlen(request->rec_buf);

				if(verbose)
					printf("bytes read: %d\n", request->bytes_read_cli);

				if(verbose){
					printf("size of new request: %ld\n", strlen(newrequest));
					printf("rec buf: %s\n", request->rec_buf);
					printf("bytes to write: %d\n", request->bytes_to_write_ser);

				}

				struct epoll_event event;
				event.data.ptr = request;
				event.events = EPOLLOUT | EPOLLET;


				// register the socket with the epoll instance for writing. ----------------------------------------------------------
				if (epoll_ctl(efd, EPOLL_CTL_ADD, serversfd, &event) < 0) {
					if(verbose)
						printf("-----------------weird\n");
					if (epoll_ctl(efd, EPOLL_CTL_MOD, request->soc_ser, &event) < 0) {
						return;
					}
					fprintf(stderr, "error adding event\n");
					exit(1);
				}

		
				

				if(verbose)
					printf("--Read Request finished--\n");
				return;
			} else if (nread < 0) { 
				// read() (or recv()) returns a value less than 0.
				// If errno is EAGAIN or EWOULDBLOCK, it just means that there is no more data ready to be read; 
				if (errno == EWOULDBLOCK ||
					errno == EAGAIN) {
					// no more clients ready to accept
					// you will continue reading from the socket when you are notified by epoll that there is more data to be read.
					// THIS IS CHRIS SPITBALLIN
					// struct epoll_event event;
					// event.data.ptr = request;
					// event.events = EPOLLIN | EPOLLET;

					// // register the socket with the epoll instance for writing. ----------------------------------------------------------
					// if (epoll_ctl(efd, EPOLL_CTL_ADD, request->soc_ser, &event) < 0) {
					// 	perror("error adding event\n");
					// 	exit(1);
					// }

					return;
					// continue; // instead of break?
				} else {
					// If errno is anything else, this is an error. It actually be best to have your proxy exit at this point.
					perror("read request fail");
					exit(EXIT_FAILURE);
				}
			} else {
				// read ----------------------------------------------------------
				// request->bytes_read_cli += nread;
				// p += nread;
			}
		}
	}


	else if (request->state == SEND_REQUEST) {
		if(verbose)
			printf("--Send Request--\n");
		// loop to write the request to the server socket until one of the following happens:
		int written = 0;
		while(1){
			char* p = request->rec_buf;
			if(verbose)
				printf("p: %s\n",p);
			p += request->bytes_written_ser;
			if(verbose)
				printf("writing %s\n",p);
			if(verbose)
				printf("bytes to write %d\n",request->bytes_to_write_ser);
			if(verbose)
				printf("writing %d total bytes \n",request->bytes_read_cli);
			// request->bytes_to_write_ser;
			// would I use request->bytes_read_cli or request->bytes_to_write_ser?
			sleep(1);
			// written = write(request->soc_ser, p, strlen(request->rec_buf));
			written = write(request->soc_ser, p, request->bytes_to_write_ser);
			request->bytes_written_ser += written;
			request->bytes_to_write_ser -= written;
			p += written;
			if(verbose)
				printf("wrote %d bytes\n",written);
			// if () != strlen(request->rec_buf)) { //clientsfd should be serversfd
				// 	fprintf(stderr, "partial/failed write\n");
				// 	exit(EXIT_FAILURE);
				// } // I don't need this because we're fine with partial writes, right?
			// you have written the entire HTTP request to the server socket. If this is the case:
			if(request->bytes_written_ser == request->bytes_read_cli){
				// register the socket with the epoll instance for reading.
				// change state to READ_RESPONSE.
				bzero(request->rec_buf,MAX_OBJECT_SIZE);
				request->state = READ_RESPONSE;
				// request->soc_ser = serversfd;

				struct epoll_event event;
				event.data.ptr = request;
				event.events = EPOLLIN | EPOLLET;

				// register the socket with the epoll instance for writing. ----------------------------------------------------------
				if (epoll_ctl(efd, EPOLL_CTL_MOD, request->soc_ser, &event) < 0) {
					perror("error adding event\n");
					exit(1);
				}
				if(verbose)
					printf("--Send Request finished--\n");
				return;

				
			} else if(written < 0){
			// write() (or send()) returns a value less than 0.
				if (errno == EWOULDBLOCK ||
					errno == EAGAIN) {
					// no buffer space available for writing to the socket; 
					// you will continue writing to the socket when you are notified by epoll that there is more buffer space available for writing.
					return;
					// continue; // instead of break?
				} else {
					// If errno is anything else, this is an error. You can print out the error, 
					// cancel your client request, and deregister your socket at this point.
					perror("send request fail");
					// close(request->soc_ser);
					exit(EXIT_FAILURE);
				}
				
			} else {
				// request->bytes_written_ser += written;
				// request->bytes_to_write_ser -= written;
				// p += written;
				// if(verbose)
				// 	printf("wrote %d bytes\n",written);
			}

		}
	} else if (request->state == READ_RESPONSE) {
		if(verbose)
			printf("--READ_RESPONSE--\n");
		int nread = 1;
		while(1){
		// loop to read from the server socket until one of the following happens:
			char* p = request->rec_buf;
			p += request->bytes_read_ser;

			nread = recvfrom(request->soc_ser, p, MAX_OBJECT_SIZE, 0,
								(struct sockaddr *)&remote_addr, &remote_addr_len);
			if(nread == 0){
			// you have read the entire HTTP response from the server. Since this is HTTP/1.0, this 
			// is when the call to read() (or recv()) returns 0, indicating that the server has closed 
			// the connection. If this is the case:
				request->state = SEND_RESPONSE;
				// request->soc_ser = serversfd;

				struct epoll_event event;
				event.data.ptr = request;
				event.events = EPOLLOUT | EPOLLET;


				// register the socket with the epoll instance for writing. ----------------------------------------------------------
				if (epoll_ctl(efd, EPOLL_CTL_MOD, request->soc_cli, &event) < 0) {
					fprintf(stderr, "error adding event\n");
					exit(1);
				}
				// register the client socket with the epoll instance for writing.
				// change state to SEND_RESPONSE.
				if(verbose)
					printf("--Read Response finished--\n");
				return;
			} else if (nread < 0){
				if (errno == EWOULDBLOCK ||
					errno == EAGAIN) {
					// no more data ready to be read; you will continue reading from the socket 
					// when you are notified by epoll that there is more data to be read.
					return;
					// continue; // instead of break?
				} else {
					// If errno is anything else, this is an error. You can print out the error, cancel your client request, and deregister your socket at this point.
					perror("read request fail");
					exit(EXIT_FAILURE);
				}
			} else {
				// printf("reading...\n");
				request->bytes_read_ser += nread;
				p += nread;
				if(verbose)
					printf("current response:\n%s\n", request->rec_buf);
			} 

		}
	} else if (request->state == SEND_RESPONSE) {
		if(verbose)
			printf("SEND_RESPONSE\n");
		int written = 0;
		while(1){
			char* p = request->rec_buf;
			p += request->bytes_written_cli;

			written = write(request->soc_cli, p, request->bytes_read_ser);
			request->bytes_written_cli += written;
			p += written;
			// written = write(request->soc_cli, request->rec_buf, strlen(request->rec_buf));
			// you have written the entire HTTP request to the server socket. If this is the case:
			if(request->bytes_written_cli == request->bytes_read_ser){
			// you have written the entire HTTP response to the client socket. If this is the case:
				// close your client socket. You are done!
				close(request->soc_cli);
				if(verbose)
					printf("Send Response finished\n");
				return;

				
			} else if(written < 0){
			// write() (or send()) returns a value less than 0.
				if (errno == EWOULDBLOCK ||
					errno == EAGAIN) {
					// there is no buffer space available for writing to the socket; 
					// you will continue writing to the socket when you are notified by epoll that there is more buffer space available for writing.
					return;
					// continue; // instead of break?
				} else {
					// If errno is anything else, this is an error. You can print out the error, 
					// cancel your client request, and deregister your socket at this point.
					perror("send response fail");
					// close(request->soc_ser);
					exit(EXIT_FAILURE);
				}
				
			} else {
				// request->bytes_written_cli += written;
				// p += written;
			}
		}
		
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

		NULL};

	for (i = 0; reqs[i] != NULL; i++) {
		printf("Testing %s\n", reqs[i]);
		if (parse_request(reqs[i], method, hostname, port, path, headers)) {
			printf("METHOD: %s\n", method);
			printf("HOSTNAME: %s\n", hostname);
			printf("PORT: %s\n", port);
			printf("HEADERS: %s\n", headers);
		}
		else
		{
			printf("REQUEST INCOMPLETE\n");
		}
	}
}

void print_bytes(unsigned char *bytes, int byteslen) {
	int i, j, byteslen_adjusted;

	if (byteslen % 8) {
		byteslen_adjusted = ((byteslen / 8) + 1) * 8;
	}
	else
	{
		byteslen_adjusted = byteslen;
	}
	for (i = 0; i < byteslen_adjusted + 1; i++) {
		if (!(i % 8)) {
			if (i > 0) {
				for (j = i - 8; j < i; j++)
				{
					if (j >= byteslen_adjusted)
					{
						printf("  ");
					}
					else if (j >= byteslen)
					{
						printf("  ");
					}
					else if (bytes[j] >= '!' && bytes[j] <= '~')
					{
						printf(" %c", bytes[j]);
					}
					else
					{
						printf(" .");
					}
				}
			}
			if (i < byteslen_adjusted) {
				printf("\n%02X: ", i);
			}
		}
		else if (!(i % 4)) {
			printf(" ");
		}
		if (i >= byteslen_adjusted) {
			continue;
		}
		else if (i >= byteslen) {
			printf("   ");
		}
		else
		{
			printf("%02X ", bytes[i]);
		}
	}
	printf("\n");
}
