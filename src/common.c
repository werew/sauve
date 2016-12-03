#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "types.h"

#define _COMMON_C_
#include "common.h"

/**
 * Terminates the program printing an error message
 * @param msg Error message
 * @param line Number of the line
 * @param func Name of the function
 *
 * @note Do not use this function direclty, rather use
 *       the wrapping macro "fail(msg)
 */
void _fail(const char* msg, int line, const char* func){
    fprintf(stderr, "Error at line %d (function %s):\n",line,func);
    perror(msg);
    exit(EXIT_FAILURE);
}
 


/**
 * Declare the current thread as being "joinable"
 */
void signal_term(){
    pthread_t* thread = malloc(sizeof(pthread_t));
    if (thread == NULL) fail("malloc");
    *thread = pthread_self();

    PT_CHK(pthread_mutex_lock(&term_queue.mutex));

    if (list_push(term_queue.list, thread) == -1) fail("list_push");
    pthread_cond_signal(&term_queue.push_cond);

    PT_CHK(pthread_mutex_unlock(&term_queue.mutex));
}



/**
 * Change the base of a path with a new one
 * @param path Original path
 * @param old_basedir Old base of the path
 * @param new_basedir Base of the new path
 * @return A pointer to the new path or NULL in case of error
 * note: old_basedir must match exaclty the initial content
 *       of path,  otherwise the result is unspecified 
 */
char* change_base
(const char* path, const char* old_basedir, const char* new_basedir){
    size_t path_len        = strlen(path);
    size_t old_basedir_len = strlen(old_basedir);
    size_t new_basedir_len = strlen(new_basedir);
    size_t new_path_len    = path_len - old_basedir_len +
                             new_basedir_len + 1;

    char* new_path = malloc(new_path_len+1);
    if (new_path == NULL) return NULL;

    snprintf(new_path, new_path_len+1,"%s/%s",
             new_basedir,&path[old_basedir_len]);

    return new_path;
}
