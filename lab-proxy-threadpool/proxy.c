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
	// printf("%s",&request[strlen(request)-4]);
	// unsigned char string[12];
	// memcpy(&string,&request[-4])
	// if (strcmp(request[-4],"\r\n\r\n") == 0){
	if (strcmp(&request[strlen(request)-4],"\r\n\r\n") == 0){
		// printf("nice");
		return 1;
	} else {
		// printf("not nice");
		return 0;
	}
	// return 0;
}

int parse_request(char *request, char *method,
char *hostname, char *port, char *path, char *headers) {
	// int i = 0;
	// int argindex = 0;
	// char spacedelim[] = " ";
	// char rndelim[] = "\r\n";
	// char* token;
	// char* test = request;
	// char test[MAX_OBJECT_SIZE];
	char* ret;
	

	// Method
	int i = 0;
	while(*request != ' '){
		i++;
		request++;
	}
	request -= i;
	memcpy(method,request,i);
	printf("method=%s\n",method);


	// Host
	// char h[] = "Host: ";
	char* h = "Host: ";
	// char test[] = "Host: awefjawpeoifapwehf";
	
	printf("try\n");
	ret = strstr(request,h);
	// i = 0;
	// i++;
	while(*request != ' '){
		request++;
	}
	request++;
	i = 0;
	while(*request != ' ' || *request != ':'){
		request++;
		i++;
	}
	printf("try\n");
	printf("ret=%s\n",ret);

	// if(token == NULL){
	// 	printf("null");
	// 	fflush(stdout);
	// }

	printf("try\n");

	// printf("%s\n",token);

	// while(request[i] != "\r\n"){
	// 	i++;
	// }
	// memcpy(&method,&request);	
	// memcpy(&hostname,&request);
	// memcpy(&port,&request);
	// memcpy(&path,&request);
	// memcpy(&headers,&request);


	return 0;
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
