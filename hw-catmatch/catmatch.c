#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

// 1. 
//	01.  1, 2, 3
// 	02.  1, 2
//	03.  3, 2
//	04.  2
//	05.  <sys/types.h>, <sys/stat.h>, <fcntl.h>
//	06.  7, 2
//	07.  SO_ACCEPTCONN
//	08.  3
//	09.  Null-terminated
//	10.  An integer larger than zero


// 2. "I completed the TMUX exercise from Part 2"
int main(int argc, char *argv[]) {
	fprintf(stderr, "%d\n\n", getpid());
	char text[512];
	FILE *file = fopen(argv[1],"r");
	int x = 0;
	char* env;
	if(env = getenv( "CATMATCH_PATTERN")) {
		if(argc == 2) {
			while(fgets(text, sizeof(text), file)) {
				if(strstr(text,env)){
					x = 1;
				}
				printf("%d: %s", x, text);
				x = 0;
			}
		}
	}
	//printf("%s", text);

}
