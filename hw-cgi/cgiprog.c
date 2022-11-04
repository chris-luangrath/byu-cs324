// No concurrency
// 1.
// USER         PID    PPID NLWP     LWP S CMD
// cl442    2779747 2779273    1 2779747 S echoserveri

// 2.
// 1 process running, 1 thread is running, only the first client process connected

// 3.
// When the first client process was interrupted, the last message sent by the next connected client
// was sent

// process
// 4.
// USER         PID    PPID NLWP     LWP S CMD
// cl442    2783099 2779273    1 2783099 S echoserverp
// cl442    2783120 2783099    1 2783120 S echoserverp
// cl442    2783163 2783099    1 2783163 S echoserverp
// cl442    2783186 2783099    1 2783186 S echoserverp

// 5.
// There are 4 threads and four processes, as there are four different pids and 4 different lwp

// simple-thread
// 6.
// USER         PID    PPID NLWP     LWP S CMD
// cl442    2787663 2779273    4 2787663 S echoservert
// cl442    2787663 2779273    4 2787722 S echoservert
// cl442    2787663 2779273    4 2787733 S echoservert
// cl442    2787663 2779273    4 2787746 S echoservert

// 7.
// There is 1 process and 4 threads, as there is only 1 PID and 4 different LWPs

// threadpool
// 8.
// USER         PID    PPID NLWP     LWP S CMD
// cl442    2790118 2779273    9 2790118 S echoservert_pre
// cl442    2790118 2779273    9 2790119 S echoservert_pre
// cl442    2790118 2779273    9 2790120 S echoservert_pre
// cl442    2790118 2779273    9 2790121 S echoservert_pre
// cl442    2790118 2779273    9 2790122 S echoservert_pre
// cl442    2790118 2779273    9 2790123 S echoservert_pre
// cl442    2790118 2779273    9 2790124 S echoservert_pre
// cl442    2790118 2779273    9 2790125 S echoservert_pre
// cl442    2790118 2779273    9 2790126 S echoservert_pre

// 9.
// There is one process and 9 threads, each thread shares the process context with the other threads

// 10.
// There are 8 producer threads running ////////////////////////////////CHECK

// 11.
// There are 0 consumer threads running

// 12.
// The producer threads are waiting for an empty slot

// 13.
// The consumer threads are waiting for an item

// 14.
// The accepting

// 15.
// The producers inserting an item into the buffer

// 16.
// 2 consumers change state

// 17.
// The producers inserting an item into the buffer

// 18.
// The producer is now waiting for either more data or for an empty slot or for the consumer
// to take something ////////////////////////////////////////////////////////////////////////////CHECK

// 19.
// most to least expensive: thread-based, threadpool-based, process-based

// 20.
// which one has a limitation of number of clients?

// 21.
// Process-based doesn't use inter-process communication because it's one thread per process

// 22.
// Which one seems least complex to implement
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINE 1024

int main(int argc, char *argv[]){
    char *buf;
    char query[MAXLINE], content[MAXLINE];
    // int n1=0, n2=0;

    if ((buf = getenv("QUERY_STRING")) != NULL) {
        strcpy(query, buf);
    }
    // Response body
    sprintf(content, "The query string is: %s\n", query);

    // Header
    printf("Content-type: text/plain\r\n");
    printf("Content-length: %d\r\n\r\n", (int)strlen(content));
    // printf("\r\n");

    printf("%s",content);


    // sprintf(content, "Content-type: text/plain\r\n");
    // sprintf(content, "Content-length: %d\r\n", (int)strlen(content));
    // sprintf(content,"\r\n");


}