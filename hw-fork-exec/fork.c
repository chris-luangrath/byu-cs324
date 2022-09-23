#include<stdio.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<unistd.h>
#define MSGSIZE 32

int main(int argc, char *argv[]) {
	int pid, p[2];
	char inbuf[MSGSIZE];
	FILE * fp;
	fp = fopen("fork-output.txt","r");
	// fprintf(fp,"BEFORE FORK\n");
	// fflush(fp);

	pipe(p);
	if ((pid = fork()) < 0) {
		fprintf(stderr, "Could not fork()");
		exit(1);
	}

	/* BEGIN SECTION A */

	fprintf(fp,"SECTION A\n");
	fflush(fp);
	printf("Section A;  pid %d\n", getpid());
	// sleep(30);

	/* END SECTION A */
	if (pid == 0) {
		/* BEGIN SECTION B */
		close(p[0]);
		fprintf(fp,"SECTION B\n"); 
		write(p[1],"hello from Section B\n",MSGSIZE);
		fflush(fp);
		printf("Section B\n");
		// sleep(30);
		// sleep(30);
		// printf("Section B done sleeping\n");
		
		char *newenviron[] = { NULL };
		printf("Program \"%s\" has pid %d. Sleeping.\n", argv[0], getpid());
	        // sleep(30);

	        if (argc <= 1) {
	                printf("No program to exec.  Exiting...\n");
		        exit(0);
		}

        	printf("Running exec of \"%s\"\n", argv[1]);
		
		int new;
		new = fileno(fp);
		dup2(new,1);
        	execve(argv[1], &argv[1], newenviron);
	        printf("End of program \"%s\".\n", argv[0]);

		exit(0);

		/* END SECTION B */
	} else {
		/* BEGIN SECTION C */
		close(p[1]);
		int readChar;
		//readChar = 
		// read(p[0].inbuf,MSGSIZE);
		// add null character?
		fprintf(fp,"SECTION C\n");
		fflush(fp);
		printf("Section C\n");
		// sleep(30);
		readChar = read(p[0],inbuf,MSGSIZE);
		inbuf[readChar] = '\0';
		printf("%s\n",inbuf);
		// wait(NULL); // added for 11
		// sleep(30);
		// printf("Section C done sleeping\n");
		exit(0);

		/* END SECTION C */
	}
	/* BEGIN SECTION D */

	fprintf(fp,"SECTION D\n");
	fflush(fp);
	printf("Section D\n");
	// sleep(30);

	/* END SECTION D */
}

