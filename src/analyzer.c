#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
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


void copy_file(const char* file){

    // Build the path to the copy
    char* copy = change_base(file, SENV.source, SENV.destination); 
    if (copy == NULL) fail("change_base"); 

    // Get some infos about the file to copy
    struct stat buf;
    if (lstat(file, &buf) == -1) fail("lstat");
    mode_t perms = buf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

    // Open the source and the destination
    // (...setting the good permissions)
    int fd_src, fd_dst;
    if ((fd_src = open(file, O_RDONLY)) == -1 || 
        (fd_dst = open(copy, O_WRONLY|O_CREAT, perms)) == -1) 
        fail("open");

    // Main copy loop
    ssize_t n;
    char copy_buf[1024]; 
    while ((n = read(fd_src,copy_buf,1024)) != 0){
        if (write(fd_dst, copy_buf, n) < n) fail("write");
    }

    if (close(fd_src) == -1 || 
        close(fd_dst) == -1 ) fail("close");

    // Set the good access and modification time    
    struct utimbuf times = { buf.st_atime, buf.st_mtime };
    if (utime(copy,&times) == -1) fail("utime");
    
    free(copy);
}

void* analyzer(void* arg){
    // Get a file
    char* file;
    while ((file = pop_file()) != NULL){
        printf("\t\t--file--> %s\n",file);
        copy_file(file);
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
