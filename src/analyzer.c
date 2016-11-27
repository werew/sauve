#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "analyzer.h"
#include "common.h"
#include "types.h"

// Returns NULL if there are no more active_scanners 
char* pop_file(){
    
    PT_CHK(pthread_mutex_lock(&files_queue.mutex));
    // Get a file or wait to get one
    char* file;
    while((file = ring_buf_pop(files_queue.buf)) == NULL && 
           files_queue.receiving == 1 ){
        pthread_cond_wait(&files_queue.read, &files_queue.mutex);
    }

    pthread_cond_signal(&files_queue.write);

    PT_CHK(pthread_mutex_unlock(&files_queue.mutex));

    return file;
}


void* analyzer(void* arg){
    // Get a file
    char* file;
    while ((file = pop_file()) != NULL){
        printf("\t\t--file--> %s\n",file);
        free(file);
    }
    signal_term();    
    return (void*) 0; 
}



void launch_analyzers(unsigned int n_analyzers){
    int i; 
    for (i=0; i<n_analyzers; i++){
        int e;
        pthread_t thread; 
        if ((e = pthread_create(&thread, NULL, analyzer,(void*) i)) != 0){
            errno = e;
            fail("pthread_create");
        }
    }  
}
