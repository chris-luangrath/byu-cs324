#include <stdio.h>
#include <string.h>


/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:97.0) Gecko/20100101 Firefox/97.0";

int all_headers_received(char *);
int parse_request(char *, char *, char *, char *, char *, char *);
void test_parser();
void print_bytes(unsigned char *, int);


int main()
{
	test_parser();
	// char test[MAX_OBJECT_SIZE] = "yet another test\r\n\r\n";
	// char* p;
	// p = &test;
	// // *p = 
	// // char * 
	// if (all_headers_received(p) == 1){
	// 	printf("yes\n");
	// } else {
	// 	printf("no\n");
	// }

	printf("%s\n", user_agent_hdr);
	return 0;
}

int all_headers_received(char *request) {
	if (strcmp(&request[strlen(request)-4],"\r\n\r\n") == 0){
		printf("nice\n");
		return 1;
	} else {
		printf("not nice\n");
		return 0;
	}
}

int parse_request(char *request, char *method,
char *hostname, char *port, char *path, char *headers) {
	// if(!all_headers_received(request)){
	// 	return 0;
	// }
	char* ret;
	char* defaultport = "0080";
	char* h = "Host: ";
	int newport = 1;
	
	// memcpy(method,request,i);
	// memset(path,0,64);
	// memset(headers,0,1024);

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
		// printf("space=%d\n",space);
		// printf("space2=%d\n",space2);
		ret++;
		i++;
		// sleep(1);
	}
	if(*ret == ':'){
		// printf("we hit it!\n");
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
		// memcpy(port,"0080",4);
		strcpy(port,defaultport);
		// *port = "0080";
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
		// ret += strlen(port) + 1;
	}
	// printf("to path=%s\n",ret);
	i = 0;
	while(*ret != ' '){
		// printf("pat=%c\n",*ret);
		i++;
		ret++;
	}
	ret -= i;
	// i -= 1;
	// printf("path=%s\n",path);
	memcpy(path,ret,i);
	if(path == NULL){
		return 0;
	}
	printf("path=%s\n",path);
	
	//// Headers
	// printf("headers\n");
	ret = strstr(request,"\r\n");
	ret += 2;
	memcpy(headers,ret,strlen(ret));
	if(headers == NULL){
		return 0;
	}

	// printf("headers=%s\n",headers);
	// sleep(5);

	// if(method == NULL || hostname == NULL || port == NULL || path == NULL || headers == NULL){
	// 	return 0;
	// }

	return 1;
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
			sleep(5);
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
