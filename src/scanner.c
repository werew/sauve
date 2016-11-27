#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
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





void push_folder(char* folder){
    PT_CHK(pthread_mutex_lock(&folders_queue.mutex))

    if (list_push(folders_queue.list, folder) == -1) 
        fail("list_push");

    pthread_cond_signal(&folders_queue.pt_cond);
    
    PT_CHK(pthread_mutex_unlock(&folders_queue.mutex))
}




void handle_file
(const char* basedir, const char* filename){
    // Create path
    size_t len_bd = strlen(basedir);
    char* path = malloc(len_bd+strlen(filename)+1);
    if (path == NULL) fail("malloc");
    
    strcpy(path,basedir);
    path[len_bd] = '/';
    strcpy(&path[len_bd+1],filename);

    // Handle the file 
    struct stat buf;
    if (lstat(path, &buf) == -1) fail("lstat");
    switch (buf.st_mode & S_IFMT) {
        case S_IFREG: printf("%s is a regular file\n",path);
            break;
        case S_IFDIR: 
                if (strcmp(filename,".")  == 0 ||
                    strcmp(filename,"..") == 0 ) break;
                push_folder(path);
            break;
        case S_IFLNK: printf("%s is a link\n",path);
            break;
        default:  // Do not treat this file
            free(path);
    }
    
}

void explore_folder(const char* folder){
    DIR* d = opendir(folder);
    if (d == NULL) fail("Cannot open source folder");
    
    struct dirent entry;
    struct dirent* result;
    do {
        // TODO change name to PT_CHK
        PT_CHK(readdir_r(d, &entry, &result));
        
        handle_file(folder, entry.d_name);

    } while (result != NULL);
     
    if (closedir(d) == -1) fail("closedir");
}

void* scanner(void* arg){
    // Get a folder
    char* folder;
    while ((folder = pop_folder()) != NULL){
        printf("%s\n",folder);
        explore_folder(folder);
        free(folder);
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

