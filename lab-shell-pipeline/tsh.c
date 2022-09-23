/* 
 * tsh - A tiny shell program with job control
 * 
 * Chris Luangrath, cl442
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXCMDS 20

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
char sbuf[MAXLINE];         /* for composing sprintf messages */

/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
int parseargs(char **argv, int *cmds, int *stdin_redir, int *stdout_redir);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (e.g., "quit")
 * then execute it immediately.
 *
 * Otherwise, build a pipeline of commands
 * run the job in the context of the child.  Have the parent wait for child process to complete and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{   
    char *argv[MAXARGS];
    int cmds[MAXCMDS];
    int stdin_redir[MAXCMDS];
    int stdout_redir[MAXCMDS];

    // **char argc[MAXARGS];
    int num_commands = parseline(cmdline, argv);
    int num_args = parseargs(argv,cmds,stdin_redir,stdout_redir);

    builtin_cmd(argv);
    
    // /bin/cat < test1.txt | /bin/cat......
    // argv is populated with the strings in the cmndline
    // in parseargs, take in argv and other arrays, 
    // turns commands in argv null to seperate differnt commands <

    // char **argv1 = malloc(sizeof(char *)*MAXARGS);
    // int result1 = parseline(cmdline, argv1);
    // free (argv1);
    // printf("You entered: %s\n", cmdline);

    /*
        - eval takes in command line, does stuff with it
        - parseline fills array with words from command line
        - parseargs returns the number of commands in 
            - 
        - should builtin_cmd 
        cmds has start of each command
        argv has text and redirictions are null
        argv[cmds[0]] is pathname, &argv[cmds[0]] is argv arguments, so on
        stdin/out have different indexes if there's a redirection

    */

    // for(int i = 0; i < sizeof(num_args);i++){
    builtin_cmd(&argv[cmds[0]]);
        // check later
    // }
    // int p[2];
    // pipe(p);
    // FILE * fp;
    int pid1, pid2, p[2];
    char *newenviron[] = { NULL };

    // printf("num commands:%d\n",num_commands);
    printf("hey\n");
    printf("num args:%d\n",num_args);
    // only 1 thing
    if(num_args == 0) {
        if ((pid1 = fork()) < 0) {
            fprintf(stderr, "Could not fork()");
            exit(1);
        }
        // child
        if(pid1==0){
            // Check the command for any input or output redirection, and perform that redirection.
            FILE * fp;
            if(stdin_redir[0] > 0){
                // redirect stdin to stdin_redir[i]

                fp = fopen(argv[stdin_redir[0]],"r");
                dup2(fileno(fp),STDIN_FILENO); // STDIN_FILENO is 0
                // dup2(stdin_redir[0],1); // TODO: i ------------------------------------------
            }
            if (stdout_redir[0] > 0){
                // redirect stdout to stddout_redir[i]
                
                fp = fopen(argv[stdout_redir[0]],"w");
                dup2(fileno(fp),STDOUT_FILENO); // STDOUT_FILENO is 1 (?)
            }
            execve(argv[cmds[0]],&argv[cmds[0]],newenviron); // TODO: i ------------------------------------------

            // Close any open file descriptors that will not be used by the child process. 
            // This includes file descriptors that were created as part of input/output redirection.
            // Run the executable in the context of the child process using execve()

        } else {
            // parent

            // Put the child process in its own process group,
            setpgid(pid1,pid1); 
            // wait for the child process to complete.
            int *status;
            waitpid(pid1, status,WUNTRACED); // right one??????????????????????????????
        }
            

        return;
    }   
    // multiple thingshey
    // printf("multiple things");
    // Create a pipe.
    pipe(p);

    // saw this online, does this work? --------------------------------------------
    // (pid1 = fork()) && (pid2 = fork());
    // if ((pid1 = fork() < 0)) {
	// 	fprintf(stderr, "Could not fork()");
	// 	exit(1);
	// }
    
    pid1 = fork();

    if(pid1 == 0){
        pid2 = fork();
    }
    // child1
    if(pid1==0){
        // Check the command for any input or output redirection, and perform that redirection.
        // printf("child 1");
        FILE * fp;
        if(stdin_redir[0] > 0){
            // redirect stdin to stdin_redir[i]

            fp = fopen(argv[stdin_redir[0]],"r");
            dup2(fileno(fp),STDIN_FILENO); // STDIN_FILENO is 0
            // dup2(stdin_redir[0],1); // TODO: i 
            // Duplicate the appropriate pipe file descriptors to enable the standard output 
            // of one process to be piped to the standard input of the other process.
            
        }
        dup2(fileno(p[1]),STDOUT_FILENO);
        close(p[0]);

        // printf("child 1");


        // if (stdout_redir[0] > 0){
        //     // redirect stdout to stddout_redir[i]

        //     fp = fopen(argv[stdout_redir[0]],"w");
        //     dup2(fileno(fp),STDOUT_FILENO); // STDOUT_FILENO is 1 (?)
        //     // Duplicate the appropriate pipe file descriptors to enable the standard output 
        //     // of one process to be piped to the standard input of the other process.
        //     dup2(fileno(p[0]),STDIN_FILENO);
        // }

        // Duplicate the appropriate pipe file descriptors to enable the standard output 
        // of one process to be piped to the standard input of the other process.
        // dup2(fileno(p[1]),STDOUT_FILENO);

        // Close any open file descriptors that will not be used by the child process. 
        // This includes file descriptors that were created as part of input/output redirection. ???? TODODODO
        
        // Run the executable in the context of the child process using execve()
        execve(argv[cmds[0]],&argv[cmds[0]],newenviron); // TODO: i 

    } else if(pid2 == 0){
        // child2
        // printf("child 2");
        // Check the command for any input or output redirection, and perform that redirection.
        FILE * fp;
        // if(stdin_redir[1] > 0){
        //     // redirect stdin to stdin_redir[i]

        //     fp = fopen(argv[stdin_redir[1]],"r");
        //     dup2(fileno(fp),STDIN_FILENO); // STDIN_FILENO is 0
        //     // dup2(stdin_redir[0],1); // TODO: i 
        // }
        if (stdout_redir[1] > 0){
            // redirect stdout to stddout_redir[i]

            fp = fopen(argv[stdout_redir[1]],"w");
            dup2(fileno(fp),STDOUT_FILENO); // STDOUT_FILENO is 1 (?)
        }
        dup2(fileno(p[0]),STDIN_FILENO);

        close(p[1]);
        // Duplicate the appropriate pipe file descriptors to enable the standard output 
        // of one process to be piped to the standard input of the other process.

        // Close any open file descriptors that will not be used by the child process. 
        // This includes file descriptors that were created as part of input/output redirection. ???? TODODODO
        

        // Run the executable in the context of the child process using execve()
        execve(argv[cmds[1]],&argv[cmds[1]],newenviron); // TODO: i 
    } else {
        // parent
        // printf("parent");

        // Put the child process in its own process group,
        setpgid(pid1,pid1);
        setpgid(pid2,pid1); 

        close(p[0]);
        close(p[1]);

        // wait for the child process to complete.
        int *status;
        waitpid(pid1, status,WUNTRACED);
        waitpid(pid2, status,WUNTRACED);
    }
    

    return;
}

/* 
 * parseargs - Parse the arguments to identify pipelined commands
 * 
 * Walk through each of the arguments to find each pipelined command.  If the
 * argument was | (pipe), then the next argument starts the new command on the
 * pipeline.  If the argument was < or >, then the next argument is the file
 * from/to which stdin or stdout should be redirected, respectively.  After it
 * runs, the arrays for cmds, stdin_redir, and stdout_redir all have the same
 * number of items---which is the number of commands in the pipeline.  The cmds
 * array is populated with the indexes of argv corresponding to the start of
 * each command sequence in the pipeline.  For each slot in cmds, there is a
 * corresponding slot in stdin_redir and stdout_redir.  If the slot has a -1,
 * then there is no redirection; if it is >= 0, then the value corresponds to
 * the index in argv that holds the filename associated with the redirection.
 * 
 */
int parseargs(char **argv, int *cmds, int *stdin_redir, int *stdout_redir) 
{
    int argindex = 0;    /* the index of the current argument in the current cmd */
    int cmdindex = 0;    /* the index of the current cmd */

    if (!argv[argindex]) {
        return 0;
    }

    cmds[cmdindex] = argindex;
    stdin_redir[cmdindex] = -1;
    stdout_redir[cmdindex] = -1;
    argindex++;
    while (argv[argindex]) {
        if (strcmp(argv[argindex], "<") == 0) {
            argv[argindex] = NULL;
            argindex++;
            if (!argv[argindex]) { /* if we have reached the end, then break */
                break;
	        }
            stdin_redir[cmdindex] = argindex;
        } else if (strcmp(argv[argindex], ">") == 0) {
            argv[argindex] = NULL;
            argindex++;
            if (!argv[argindex]) { /* if we have reached the end, then break */
                break;
            }
            stdout_redir[cmdindex] = argindex;
        } else if (strcmp(argv[argindex], "|") == 0) {
            argv[argindex] = NULL;
            argindex++;
            if (!argv[argindex]) { /* if we have reached the end, then break */
                break;
            }
            cmdindex++;
            cmds[cmdindex] = argindex;
            stdin_redir[cmdindex] = -1;
            stdout_redir[cmdindex] = -1;
        }
        argindex++;
    }

    return cmdindex + 1;
}

/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	argv[--argc] = NULL;
    }
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
int builtin_cmd(char **argv) 
{

    if (strcmp(argv[0],"quit") == 0){
        // printf("quitting now\n");
        exit(0);
    }
    else{
            // printf("hey2\n");
        //     printf("%ld\n",sizeof(argv));
        // for (int i = 0; i < sizeof(argv); i++){
        //     printf("it is: %s\n",argv[i]);
        //     // printf("hey3\n");
        // }
        // printf("%s\n",argv[0]);

        return 0;
    }     /* not a builtin command */
}

/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

