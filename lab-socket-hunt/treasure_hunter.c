// Replace PUT_USERID_HERE with your actual BYU CS user id, which you can find
// by running `id -u` on a CS lab machine.
#define USERID 1823700709
#define BUF_SIZE 8
#define BYTE_SIZE 2
#define ID_SIZE 8
#define SEED_SIZE 4

#include <stdio.h>
#include <string.h>

int verbose = 0;

void print_bytes(unsigned char *bytes, int byteslen);

int main(int argc, char *argv[]) {
	int server = atoi(argv[1]);
	int port = atoi(argv[2]);
	char * port_c = argv[2]; 
	int level = atoi(argv[3]);
	int seed = atoi(argv[4]);

	unsigned char buf[BUF_SIZE];
	bzero(buf,BUF_SIZE);

	memcpy(&buf[1], level, BYTE_SIZE);
	memcpy(&buf[2], USERID, ID_SIZE);
	memcpy(&buf[6], seed, SEED_SIZE);
	print("hey\n");

	print_bytes(buf,BUF_SIZE);

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
