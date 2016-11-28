#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include "types.h"
#include "common.h"
#include "scanner.h"
#include "analyzer.h"

void usage(char* name, int exit_value){
    printf("usage: %s [-h] [-n] [-s n] [-a n] [-f n] "
           "source [previous] destination\n",name);
    exit(exit_value);
}

void init_queues
(unsigned int n_scanners, unsigned int n_analyzers,
 unsigned int max_buff_entries){

    // Init term_queue
    pthread_mutex_init(&term_queue.mutex, NULL);
    pthread_cond_init(&term_queue.push_cond, NULL);
    term_queue.list = create_list(); 
    if (term_queue.list == NULL) fail("init term_queue");

    // Init folders_queue
    folders_queue.active_scanners = n_scanners;
    pthread_mutex_init(&folders_queue.mutex, NULL);
    pthread_cond_init(&folders_queue.pt_cond, NULL);
    folders_queue.list = create_list(); 
    if (folders_queue.list == NULL) fail("init folders_queue");

    size_t s = strlen(SENV.source);
    char* first_folder = malloc(s+1);
    if (first_folder == NULL) fail("malloc");
    strcpy(first_folder,SENV.source);

    if (list_push(folders_queue.list, first_folder) == -1)
        fail("push folders_queue");

    // Init files_queue
    pthread_mutex_init(&files_queue.mutex, NULL);
    pthread_cond_init(&files_queue.read , NULL);
    pthread_cond_init(&files_queue.write, NULL);
    files_queue.receiving = 1;

    files_queue.buf = create_ring_buf(max_buff_entries);
    if (files_queue.buf == NULL) fail("init files_queue");

}


int main(int argc, const char* argv[]){
    // Default values
    unsigned int n_scanners  = 1, 
                 n_analyzers = 1, 
                 max_buff_entries = 8;

    // Get options
    int opt;
    while ((opt = getopt (argc, argv, "hns:a:f:")) != -1){
        switch (opt){
            case 'h': usage(argv[0], 0);
                break;
            case 'n': SENV.debug_opt = 1;
                break;
            case 's': n_scanners = atoi(optarg); // TODO atoi
                break ;
            case 'a': n_analyzers = atoi(optarg); // TODO atoi
                break ;
            case 'f': max_buff_entries = atoi(optarg); // TODO atoi
                break ;
            default: usage(argv[0], 1);
        }
    }

    // Get arguments and init environnement 
    switch (argc - optind){
        case 2: 
            SENV.source = argv[optind];
            SENV.previous = NULL;
            SENV.destination = argv[optind+1];
            break ;
        case 3 :        
            SENV.source = argv[optind];
            SENV.previous = argv[optind+1];
            SENV.destination = argv[optind+2];
            break ;
        default : usage(argv[0], 1);
    }


    init_queues(n_scanners, n_analyzers, max_buff_entries);

    launch_scanners(n_scanners);
    launch_analyzers(n_analyzers);

    // Join threads
    unsigned int n_threads = n_scanners + n_analyzers;
    unsigned int n_joined  = 0;
    while (n_joined < n_threads){
        PT_CHK(pthread_mutex_lock(&term_queue.mutex));

        pthread_t* thread;
        while ((thread = list_pop(term_queue.list)) == NULL){
            pthread_cond_wait(&term_queue.push_cond, &term_queue.mutex);
        }

        PT_CHK(pthread_mutex_unlock(&term_queue.mutex));
        PT_CHK(pthread_join(*thread, NULL));
        free(thread);
        n_joined++;
    }

    free(term_queue.list);
    free(folders_queue.list);
    free(files_queue.buf);
   
    return 0;
}
