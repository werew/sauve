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

/**
 * Retrives a file from the files queue. If no file is available
 * waits until a new file is pushed into the queue. 
 *
 * @return A zero-terminated string containing the path to the file
 *         or NULL if there are not files to handle.
 */
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



/**
 * Checks if a previous copy of the file is identical to the
 * original file. In this case creates an hard link to the original
 * file into the given destination.
 * @param src Path to the original file
 * @param dest Path to the potential link
 * @param buf_src struct stat of the source file
 * @return 1 if an hard link has been created, 0 otherwise
 */
int try_link(const char* src, const char* dest, struct stat* buf_src){
    
    // Build the path to the previous copy
    char* prev = change_base(src, SENV.source, SENV.previous); 
    if (prev == NULL) fail("change_base"); 

    // Get some infos about the previous copy
    struct stat buf_prv;
    if (lstat(prev, &buf_prv) == -1) fail("lstat");

    // Compare source file with previous copy
    if (buf_src->st_mode  != buf_prv.st_mode  ||
        buf_src->st_size  != buf_prv.st_size  ||
        buf_src->st_atime != buf_prv.st_atime ||
        buf_src->st_mtime != buf_prv.st_mtime ){
        free(prev);
        return 0;
    }
   
    // If debug is set 
    if (SENV.debug_opt == 1){
        printf("---> Linking %s to %s\n",prev, dest);
        free(prev);
        return 1;
    }

    if (link(prev, dest) == -1) fail("link");

    free(prev);
    
    return 1;
}



/**
 * Copies a file togheter with its permissions and 
 * last access and modification.
 * @param scr File to copy
 * @param dest Path to the copy
 */
void copy_file(const char* src, const char* dest){

    // Get some infos about the file to copy
    struct stat buf;
    if (lstat(src, &buf) == -1) fail("lstat");
    mode_t perms = buf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

    // If previous is set, try to just link the file
    if (SENV.previous  != NULL && 
        try_link(src,dest,&buf) == 1) return;

    // If debug is set 
    if (SENV.debug_opt == 1){
        printf("---> Copying %s to %s\n",src, dest);
        return;
    }

    // Open the source and the destination
    // (...setting the good permissions)
    int fd_src, fd_dst;
    if ((fd_src = open(src , O_RDONLY)) == -1 || 
        (fd_dst = open(dest, O_WRONLY|O_CREAT, perms)) == -1) 
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
    if (utime(dest,&times) == -1) fail("utime");
}



/**
 * Entry point of an analyzer
 */
void* analyzer(void* arg){
    // Get a file
    char* src;
    while ((src = pop_file()) != NULL){

        // Build the path to the copy
        char* dest = change_base(src, SENV.source, SENV.destination); 
        if (dest == NULL) fail("change_base"); 

        copy_file(src, dest);

        free(src);
        free(dest);
    }
    signal_term();    
    return (void*) 0; 
}


/** 
 * Launch a certain naumber of threads analyzer. 
 * @param n_analyzers The number of analyzers to be launched
 */
void launch_analyzers(unsigned int n_analyzers){
    int i; 
    for (i=0; i<n_analyzers; i++){
        int e;
        pthread_t thread; 
        if ((e = pthread_create(&thread, NULL, analyzer, NULL)) != 0){
            errno = e;
            fail("pthread_create");
        }
    }  
}
