#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "scanner.h"
#include "common.h"
#include "types.h"

char* pop_folder(){
    
    PT_CHK(pthread_mutex_lock(&folders_queue.mutex))

    // This scanner is looking for a folder, so it's not active
    folders_queue.active_scanners--;

    // Get a folder or wait to get one
    char* folder;
    while((folder = list_pop(folders_queue.list)) == NULL && 
           folders_queue.active_scanners > 0){
        pthread_cond_wait(&folders_queue.pt_cond, &folders_queue.mutex);
    }

    if (folder == NULL) {
        // Didn't get a folder: there are no active scanners
        // Notify all the scanners
        pthread_cond_broadcast(&folders_queue.pt_cond);
        PT_CHK(pthread_mutex_unlock(&folders_queue.mutex))

        // Notify all the analyzers
        PT_CHK(pthread_mutex_lock(&files_queue.mutex));
        files_queue.receiving = 0;
        pthread_cond_broadcast(&files_queue.read);
        PT_CHK(pthread_mutex_unlock(&files_queue.mutex));
    } else {
        // Got a folder: this scanner is now active
        folders_queue.active_scanners++;
        PT_CHK(pthread_mutex_unlock(&folders_queue.mutex))
    }


    return folder;
}





void push_folder(char* folder){
    PT_CHK(pthread_mutex_lock(&folders_queue.mutex))

    if (list_push(folders_queue.list, folder) == -1) 
        fail("list_push");

    pthread_cond_signal(&folders_queue.pt_cond);
    
    PT_CHK(pthread_mutex_unlock(&folders_queue.mutex))
}


void push_file(char* file){
    PT_CHK(pthread_mutex_lock(&files_queue.mutex));

    while (ring_buf_push(files_queue.buf, file) == -1) {
        pthread_cond_wait(&files_queue.write, &files_queue.mutex);
    }

    pthread_cond_signal(&files_queue.read);
    
    PT_CHK(pthread_mutex_unlock(&files_queue.mutex));
}


void handle_file
(const char* basedir, const char* filename){

    if (strcmp(filename,".")  == 0 ||
        strcmp(filename,"..") == 0 ) return;

    // Create path
    size_t len_bd = strlen(basedir);
    char* path = malloc(len_bd+strlen(filename)+2);
    if (path == NULL) fail("malloc");

   
    // TODO use snprintf 
    strcpy(path,basedir);
    path[len_bd] = '/';
    strcpy(&path[len_bd+1],filename);

    // Handle the file 
    struct stat buf;
    if (lstat(path, &buf) == -1) fail("lstat");
    switch (buf.st_mode & S_IFMT) {
        case S_IFREG: 
                push_file(path);
            break;
        case S_IFDIR: push_folder(path); 
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
    while (1) {
        // TODO change name to PT_CHK
        PT_CHK(readdir_r(d, &entry, &result));
        
        if (result == NULL) break;   

        handle_file(folder, entry.d_name);

    } 
     
    if (closedir(d) == -1) fail("closedir");
}

void copy_dir(char* path){
    char* folder = change_base(path, SENV.source, SENV.destination); 
    if (folder == NULL) fail("change_base"); 
  
    if (SENV.debug_opt == 1){
        printf("Creating directory %s\n",folder);
        free(folder);
        return;
    }

    struct stat buf;
    if (lstat(path, &buf) == -1) fail("lstat");

    mode_t perms = buf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    if (mkdir(folder,perms) == -1) fail("mkdir");

    free(folder);
}

void* scanner(void* arg){
    // Get a folder
    char* folder;
    while ((folder = pop_folder()) != NULL){
        copy_dir(folder);
        explore_folder(folder);
        free(folder);
    }
    signal_term();    
    return (void*) 0; 
}

void launch_scanners(unsigned int n_scanners){
    int i; 
    for (i=0; i<n_scanners; i++){
        int e;
        pthread_t thread; 
        if ((e = pthread_create(&thread, NULL, scanner, (void*) i)) != 0){
            errno = e;
            fail("pthread_create");
        }
    }  
}

