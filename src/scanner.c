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

/**
 * Retrives a folder from the folders queue. If no folder is available
 * waits until a new folder is pushed into the queue. If there are not
 * folders available and neither active scanners, notifies all the 
 * inactive threads, "closes" the files queue and returns NULL.
 *
 * @return A zero-terminated string containing the path to the folder
 *         to be copyed or NULL if there are not folders to handle.
 */
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

        if (folders_queue.active_scanners != -1){

            folders_queue.active_scanners = -1;

            // Notify all the scanners
            pthread_cond_broadcast(&folders_queue.pt_cond);
            PT_CHK(pthread_mutex_unlock(&folders_queue.mutex))

            // Notify all the analyzers
            PT_CHK(pthread_mutex_lock(&files_queue.mutex));
            files_queue.receiving = 0;
            pthread_cond_broadcast(&files_queue.read);
            PT_CHK(pthread_mutex_unlock(&files_queue.mutex));
        } else {
            // The other threads have already been notified 
            PT_CHK(pthread_mutex_unlock(&folders_queue.mutex))
        }

    } else {

        // Got a folder: this scanner is now active
        folders_queue.active_scanners++;
        PT_CHK(pthread_mutex_unlock(&folders_queue.mutex))

    }

    return folder;
}



/**
 * Adds a new folder to the folders queue.
 * @param folder A zero-terminated string containing the path of the
 *        folder to be added.
 */
void push_folder(char* folder){
    PT_CHK(pthread_mutex_lock(&folders_queue.mutex))

    // Push folder
    if (list_push(folders_queue.list, folder) == -1) 
        fail("list_push");

    // Tells waiting scanners
    pthread_cond_signal(&folders_queue.pt_cond);
    
    PT_CHK(pthread_mutex_unlock(&folders_queue.mutex))
}



/**
 * Adds a new file to the files queue.
 * @param file A zero-terminated string containing the path of the
 *        file to be added.
 */
void push_file(char* file){
    PT_CHK(pthread_mutex_lock(&files_queue.mutex));

    // Push file
    while (ring_buf_push(files_queue.buf, file) == -1) {
        pthread_cond_wait(&files_queue.write, &files_queue.mutex);
    }

    // Tells waiting analyzers
    pthread_cond_signal(&files_queue.read);
    
    PT_CHK(pthread_mutex_unlock(&files_queue.mutex));
}



/**
 * Handles a file: regular files are pushed into the files queue
 * while directories are pushed into the folders queue. Other
 * types of files are ignored.
 * @param basedir A null-terminated string containing the path
 *        to the basedir of the file
 * @param filename A null-terminated string containing the name
 *        of the file
 */
void handle_file
(const char* basedir, const char* filename){

    // Ignore those folders in order to prevent
    // infinite loops
    if (strcmp(filename,".")  == 0 ||
        strcmp(filename,"..") == 0 ) return;

    // Create path
    size_t len  = strlen(basedir) + strlen(filename) + 2;
    char* path = malloc(len);
    if (path == NULL) fail("malloc");

    snprintf(path,len, "%s/%s", basedir, filename);

    // Handle the file 
    struct stat buf;
    if (lstat(path, &buf) == -1) fail("lstat");
    switch (buf.st_mode & S_IFMT) {
        case S_IFREG: push_file(path);
            break;
        case S_IFDIR: push_folder(path); 
            break;
        default:  // Ignore this file
            free(path);
    }
}



/**
 * Explore a folder by handling each file contained in it
 * @param folder A null-terminated string containing the 
 *        path to the folder
 */
void explore_folder(const char* folder){
    DIR* d = opendir(folder);
    if (d == NULL) fail("Cannot open source folder");
  
    struct dirent entry;
    struct dirent* result;
    while (1) {

        PT_CHK(readdir_r(d, &entry, &result));
        if (result == NULL) break;   

        handle_file(folder, entry.d_name);

    } 
     
    if (closedir(d) == -1) fail("closedir");
}



/**
 * Creates an empty copy of a directory having the
 * same permissions.
 * @param scr Path to the folder to copy
 * @param dest Destination of the copy
 */
void copy_dir(const char* src, const char* dest){

    struct stat buf;
    if (lstat(src, &buf) == -1) fail("lstat");

    mode_t perms = buf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    if (mkdir(dest, perms) == -1) fail("mkdir");
}



/**
 * Entry point of a scanner
 */
void* scanner(void* arg){
    // Get a folder
    char *folder, *copy;
    while ((folder = pop_folder()) != NULL){

        copy = change_base(folder, SENV.source, SENV.destination); 
        if (copy == NULL) fail("change_base"); 
      
        if (SENV.debug_opt == 1){
            printf("Creating directory %s\n",copy);
            free(copy); free(folder);
            continue; 
        }

        copy_dir(folder, copy);
        explore_folder(folder);
        free(folder); free(copy);
    }

    signal_term();    
    return (void*) 0; 
}



/** 
 * Launch a certain naumber of threads scanners. 
 * @param n_scanners The number of scanners to be launched
 */
void launch_scanners(unsigned int n_scanners){
    int i; 
    for (i=0; i<n_scanners; i++){
        int e;
        pthread_t thread; 
        if ((e = pthread_create(&thread, NULL, scanner, NULL)) != 0){
            errno = e;
            fail("pthread_create");
        }
    }  
}

