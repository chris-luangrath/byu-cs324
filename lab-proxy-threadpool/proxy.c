#include "sbuf.h"

#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define REC_SIZE 102400
#define REQUEST_SIZE 1024
// #define REQUEST_SIZE 102400
#define BUF_SIZE 102400
#define NTHREADS  8
#define SBUFSIZE  5

static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:97.0) Gecko/20100101 Firefox/97.0";


int all_headers_received(char *);
int parse_request(char *, char *, char *, char *, char *, char *);
void test_parser();
int open_sfd(char*, char*);
void handle_client(int);
void print_bytes(unsigned char *, int);
void *handle_clients(void *vargp);

int verbose = 0;
sbuf_t sbuf;

int main(int argc, char* argv[])
{
	// test_parser();

	// printf("%s\n", user_agent_hdr);
	int sfd = 0;
	int clientsfd = 0;
	sfd = open_sfd(NULL, argv[1]);
	// printf("sfd=%d\n",sfd);
	pthread_t tid;
	sbuf_init(&sbuf, SBUFSIZE);

	for (int i = 0; i < NTHREADS; i++)  /* Create worker threads */ //line:conc:pre:begincreate
		pthread_create(&tid, NULL, handle_clients, NULL);               //line:conc:pre:endcreate

	while(1){
		// accept(sfd,&remote_addr,&remote_addr_len);
		if ((clientsfd = accept(sfd, NULL, NULL)) < 0) {
			perror("Could not accept");	
			exit(EXIT_FAILURE);
		} else {
			sbuf_insert(&sbuf, clientsfd); /* Insert clientsfd in buffer */
			// pthread_create();
			// pthread_create(&tid, NULL, handle_clients, NULL); 
			// pthread_create(&tid, NULL, handle_clients, (void*) clientsfd); 
		}
		// handle_client(clientsfd);
	}

	return 0;
}

void *handle_clients(void *vargp) 
{  
	pthread_detach(pthread_self()); 
	// int* clientsfd = (int*) vargp;
	while(1){
		int clientsfd = sbuf_remove(&sbuf);
		handle_client(clientsfd);
		// close(clientsfd);
	}

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

// int open_sfd() {
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

void handle_client(int acceptsfd){
	int nread = 0;
	unsigned char rec_buf[REC_SIZE];
	bzero(rec_buf,REC_SIZE);
	socklen_t remote_addr_len;
	struct sockaddr_storage remote_addr; // should I be using sockaddr_in?
	remote_addr_len = sizeof(struct sockaddr_storage);

	char request[REQUEST_SIZE];
	// char request[REQUEST_SIZE], (*p)[REQUEST_SIZE] = &request;
	char *p = &request;
	// char (*p)[REQUEST_SIZE] = &request;
	bzero(request,REQUEST_SIZE);

	char method[16], hostname[64], port[8], path[64], headers[1024];
	memset(method,0,16);
	memset(hostname,0,64);
	memset(port,0,8);
	memset(path,0,64);
	memset(headers,0,1024);


	// int i = 0;
	int headers_recieved = 0;
	while(!headers_recieved){
		// printf("receiving...\n");
		// memset(rec_buf,0,REC_SIZE);
		nread = recvfrom(acceptsfd, rec_buf, REC_SIZE, 0,
							(struct sockaddr *) &remote_addr, &remote_addr_len);
		if (nread == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}
		memcpy(p,rec_buf,nread);
		p += nread;
		if(all_headers_received(request)){
			headers_recieved = 1;
			// printf("done receiving\n");
		} else {
			// printf("%s\n",request);
		}
	}

	// printf("%s\n",request);

	if (parse_request(request, method, hostname, port, path, headers)) {
		printf("METHOD: %s\n", method);
		printf("HOSTNAME: %s\n", hostname);
		printf("PORT: %s\n", port);
		printf("PATH: %s\n", path); // I ADDED THIS ONE. IT WASNT HERE BEFORE
		printf("HEADERS: %s\n", headers);
		// printf("%s\n", user_agent_hdr);
	} else {
		printf("REQUEST INCOMPLETE\n");
		exit(1);
	}
	
	
	
	char* connection = "close";
	char* proxyconnection = "close";

	char newrequest[REQUEST_SIZE];
	p = &newrequest;
	bzero(newrequest,REQUEST_SIZE);
	char buf[BUF_SIZE];
	bzero(buf,BUF_SIZE);
	sprintf(buf,"%s %s HTTP/1.0\r\n",method,path);
	// printf("method=%s\n",method);
	// printf("path=%s\n",path);
	// printf("buf=%s\n",buf);
	
	memcpy(p,&buf,strlen(buf));
	p += strlen(buf);
	
	bzero(buf,BUF_SIZE);
	sprintf(buf,"Host: %s:%s\r\n",hostname,port);
	memcpy(p,&buf,strlen(buf));
	p += strlen(buf);

	bzero(buf,BUF_SIZE);
	sprintf(buf,"%s\r\n",user_agent_hdr);
	memcpy(p,&buf,strlen(buf));
	p += strlen(buf);

	bzero(buf,BUF_SIZE);
	sprintf(buf,"Connection: %s\r\nProxy-Connection: %s\r\n\r\n",connection,proxyconnection);
	memcpy(p,&buf,strlen(buf));
	p += strlen(buf);

	
	// printf("HEY IT'S HERE ----------------------\n");
	// if(verbose)
	// printf("%s\n",newrequest);
	printf("new request:\n%s\n",newrequest);

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

		printf("success?\n");
		close(serversfd);
	}
	// printf("2--------------------------------------------\n");

	if (rp == NULL) {   /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);
	// printf("3--------------------------------------------\n");
	if (write(serversfd, newrequest, strlen(newrequest)) != strlen(newrequest)) { //clientsfd should be serversfd
		fprintf(stderr, "partial/failed write\n");
		exit(EXIT_FAILURE);
	}
	

	// sleep(5);
	// printf("4--------------------------------------------\n");

	char response[MAX_OBJECT_SIZE];
	p = &response;

	int total = 0;
	nread = 1;
	printf("start readin:\n");
	while((nread = read(serversfd,rec_buf,REC_SIZE)) != 0){
		printf("nread=%d\n",nread);
		// nread = read(serversfd,rec_buf,REC_SIZE);
		if (nread == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}
		memcpy(p,&rec_buf,nread);
		// p += strlen(rec_buf);
		p += nread;
		total += nread;
		printf("total=%d\n",total);
		printf("strlen=%ld\n",strlen(response));
	}
	// if(verbose)
	printf("resp:\n%s\n",response);
	// printf("%s\n",response);

	// printf("5--------------------------------------------\n");
	close(serversfd);

	if (send(acceptsfd,response,total,0) < 0)
				fprintf(stderr, "Error sending response\n");

	close(acceptsfd); // should I do this here or in handle-thread
	
	
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
		memset(method,0,16);
		memset(hostname,0,64);
		memset(port,0,8);
		memset(path,0,64);
		memset(headers,0,1024);
		printf("Testing %s\n", reqs[i]);
		if (parse_request(reqs[i], method, hostname, port, path, headers)) {
			printf("METHOD: %s\n", method);
			printf("HOSTNAME: %s\n", hostname);
			printf("PORT: %s\n", port);
			printf("PATH: %s\n", path); // I ADDED THIS ONE. IT WASNT HERE BEFORE
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
