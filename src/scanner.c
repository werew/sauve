#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "common.h"
#include "types.h"

/*
    When adding a new folder:
    PT_CHK(pthread_mutex_lock(&folders_queue.mutex))

    if (list_push(folders_queue.list, path) == -1) fail("list_push");
    pthread_cond_signal(&folders_queue.pt_cond);
    
    PT_CHK(pthread_mutex_unlock(&folders_queue.mutex))

    When finished adding:
    
    PT_CHK(pthread_mutex_lock(&folders_queue.mutex))
    folders_queue.active_threads--;

    char* folder;
    while((folder = list_pop(&folders_queue.list)) == NULL && 
           folders_queue.active_threads > 0){
        pthread_cond_wait(&folders_queue.pt_cond, &folders_queue.mutex);
    }

    if (folder == NULL) {
        pthread_cond_broadcast(&folders_queue.pt_cond);
    } else {
        folders_queue.active_threads++;
    }

    PT_CHK(pthread_mutex_unlock(&folders_queue.mutex))

    return folder;


*/

char* pop_folder(){
    
    PT_CHK(pthread_mutex_lock(&folders_queue.mutex))

    // This scanner is looking for a folder, so it's not active
    folders_queue.active_threads--;

    // Get a folder or wait to get one
    char* folder;
    while((folder = list_pop(folders_queue.list)) == NULL && 
           folders_queue.active_threads > 0){
        pthread_cond_wait(&folders_queue.pt_cond, &folders_queue.mutex);
    }

    if (folder == NULL) {
        // Didn't get a folder: there are no active scanners
        // Notify all the scanners.
        pthread_cond_broadcast(&folders_queue.pt_cond);
    } else {
        // Got a folder: this scanner is now active
        folders_queue.active_threads++;
    }

    PT_CHK(pthread_mutex_unlock(&folders_queue.mutex))

    return folder;
}

void* scanner(void* arg){
    // Get a folder
    char* folder;
    while ((folder = pop_folder()) != NULL){
        printf("%s\n",folder);
    }
    signal_term();    
    return (void*) 0; 
}

void launch_scanners(unsigned int n_scanners, const char* source){
    int i; 
    for (i=0; i<n_scanners; i++){
        int e;
        pthread_t thread; 
        if ((e = pthread_create(&thread, NULL, scanner, (void*) source)) != 0){
            errno = e;
            fail("pthread_create");
        }
    }  
}

