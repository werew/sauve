#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "common.h"
#include "types.h"

void* scanner(void* arg){
    signal_term();    
    printf("Hello %s\n",(char*)arg);
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

