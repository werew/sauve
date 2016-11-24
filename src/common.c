#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "types.h"

#define _COMMON_C_
#include "common.h"

void _fail(const char* msg, int line, const char* func){
    fprintf(stderr, "Error at line %d (function %s):\n",line,func);
    perror(msg);
    exit(EXIT_FAILURE);
}

void signal_term(){
    pthread_t* thread = malloc(sizeof(pthread_t));
    if (thread == NULL) fail("malloc");
    *thread = pthread_self();

    PT_CHK(pthread_mutex_lock(&term_queue.mutex));

    if (list_push(term_queue.list, thread) == -1) fail("list_push");
    pthread_cond_signal(&term_queue.push_cond);

    PT_CHK(pthread_mutex_unlock(&term_queue.mutex));
}

