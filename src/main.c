#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "types.h"
#include "common.h"
#include "scanner.h"

void usage(char* name, int exit_value){
    printf("usage: %s [-h] [-n] [-s n] [-a n] [-f n] "
           "source [previous] destination\n",name);
    exit(exit_value);
}

void init_env
(unsigned int n_scanners, unsigned int n_analyzers, 
 unsigned int max_buff_entries, unsigned int debug){

    pthread_mutex_init(&term_queue.mutex, NULL);
    pthread_cond_init(&term_queue.push_cond, NULL);
    term_queue.list = create_list(); 
    if (term_queue.list == NULL) fail("init term_queue");

    folders_queue = create_list(); 
    if (folders_queue == NULL) fail("init folders_queue");

    files_queue = create_ring_buf(max_buff_entries);
    
    debug_opt = debug; 
}


int main(int argc, char* argv[]){
    // Default values
    unsigned int n_scanners  = 1, 
                 n_analyzers = 1, 
                 max_buff_entries = 8, 
                 debug = 0;

    char *source, *destination, *previous;


    int opt;
    while ((opt = getopt (argc, argv, "hns:a:f:")) != -1){
        switch (opt){
            case 'h': usage(argv[0], 0);
                break;
            case 'n': debug = 1;
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

    switch (argc - optind){
        case 2: 
            source = argv[optind];
            previous = NULL;
            destination = argv[optind+1];
            break ;
        case 3 :        
            source = argv[optind];
            previous = argv[optind+1];
            destination = argv[optind+1];
            break ;
        default : usage(argv[0], 1);
    }

    init_env(n_scanners, n_analyzers, max_buff_entries, debug);
    
    launch_scanners(n_scanners, source);

    // Join threads
    unsigned int n_threads = n_scanners ; // XXX + n_analyzers;
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
   
    return 0;
}
