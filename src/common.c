#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "common.h"

void _fail(const char* msg, int line, const char* func){
    fprintf(stderr, "Error at line %d (function %s):\n",line,func);
    perror(msg);
    exit(EXIT_FAILURE);
}

void exit_thread(void* ret){
    pthread_t* thread = malloc(sizeof(pthread_t));
    if (thread == NULL) fail("malloc");
    *thread = pthread_self();
    list_push(term_queue, thread); 
    pthread_exit(ret);
}

